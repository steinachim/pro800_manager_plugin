/**
 * Pro800 Manager Plugin
 * Copyright (C) 2025 Achim Stein
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

#pragma once

#include "../midi/MidiHandler.h"
#include "../tailoring/Pro800ChannelResolver.h"
#include "../tailoring/Pro800PanelState.h"
#include "../tailoring/Pro800SettingsConstants.h"

#include <juce_events/juce_events.h>

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>

class ProgramMessage;
class SettingsMessage;

/**
 * What the plugin knows about the connected synth, and the sequences that keep it that way: connecting, finding
 * the MIDI channel, following the preset the synth is on, selecting a preset, reverting to the stored one.
 *
 * Everything runs on the message thread, on top of MidiHandler's request/reply exchange. Owned by the processor,
 * so it outlives the editor; the UI observes it through Listener.
 *
 * The three things the Pro-800 lets a host see are kept apart on purpose:
 *  - the stored preset records (what a Load or Revert puts in the voice engine),
 *  - the selection pointer in the settings block (which preset the synth says it is on),
 *  - and the sound it is actually playing, which is *not* readable. Provenance says what the plugin's controls
 *    currently represent and whether the sound has been edited since.
 */
class SynthSession : private juce::Timer, private MidiHandler::Listener
{
public:
    enum class ConnectionState { DISCONNECTED,
        PROBING,
        CONNECTED };

    struct Channels
    {
        // sending: the synth's MIDI RX Channel, unless the user chose one
        bool automatic = true; // derived from the synth (default), or chosen by the user
        int manualChannel = 1; // 1-16, used when !automatic

        // receiving: the synth's MIDI TX Channel (the one it sends its knob movements on), unless the user chose one
        bool receiveAutomatic = true;
        int manualReceiveChannel = 0; // 1-16, or 0 = every channel; used when !receiveAutomatic

        Pro800MidiChannel rx; // MIDI RX Channel as the synth reports it (meaningful when connected)
        Pro800MidiChannel tx; // MIDI TX Channel
        bool readFromSynth = false; // false until the settings block has been read

        /** The channel (1-16) channel-voice messages go out on; nullopt if the synth would not hear them. */
        std::optional<int> sendChannel() const;

        /** The channel (1-16) incoming channel-voice messages are accepted on; 0 = every channel. */
        int receiveChannel() const;

        /** e.g. "Auto (3, from synth)", "Auto (3, DIP switches)", "Auto (OFF!)", "5 (synth reports 3)". */
        juce::String describe() const;

        /** e.g. "Auto (4, from synth)", "Auto (any - synth is on THRU)", "All". */
        juce::String describeReceive() const;
    };

    enum class PointerFreshness { UNKNOWN,
        CONFIRMED,
        PENDING,
        UNCONFIRMED };

    /** The preset the synth's settings block points at. */
    struct Pointer
    {
        std::optional<int> program; // 0-399
        std::string name; // of the stored record, empty if unknown
        PointerFreshness freshness = PointerFreshness::UNKNOWN;

        /** e.g. "B05" */
        juce::String label() const;
    };

    /** What the CC-bound controls (Front Panel / Performance tabs) currently represent. */
    struct Provenance
    {
        enum class Basis { UNKNOWN,
            STORED,
            PANEL };

        Basis basis = Basis::UNKNOWN;
        std::optional<int> program; // STORED: the record the controls were set from
        bool editedInApp = false; // a control was moved here since
        bool editedOnSynth = false; // the synth reported a knob movement since

        bool isEdited() const { return editedInApp || editedOnSynth; }

        /** e.g. "B05 as stored", "B05 + edits (in app)", "aligned with the panel", "unknown" */
        juce::String describe() const;
    };

    /** How a settings change the user made is getting on. */
    struct SettingsWriteStatus
    {
        enum class State { IDLE,
            SAVING,
            FAILED };

        State state = State::IDLE;
        juce::String message; // "Saving MIDI Input Channel...", or what did not land
        std::vector<Pro800Settings> changedOnSynth; // fields the synth changed by itself (front panel), for a few seconds
    };

    struct Listener
    {
        virtual ~Listener() = default;

        /** Something above changed: read the getters. Message thread. */
        virtual void synthSessionChanged() = 0;

        /** The settings block's content changed (read from the synth, or a write that did not land was undone). */
        virtual void synthSessionSettingsChanged() {}
    };

    /** How often the settings block is read to follow the synth's own preset changes. */
    static constexpr int POLL_INTERVAL_MS = 1500;
    /** Rapid setting changes (spin-box clicks) are written as one block write after this pause. */
    static constexpr int WRITE_DEBOUNCE_MS = 300;
    /** How long a "changed on the synth" notice stays. */
    static constexpr int CHANGED_ON_SYNTH_NOTICE_MS = 5000;
    /** A settings write commits on the synth's own cycle: about one in fourteen only shows ~1.6 s later. */
    static constexpr int WRITE_CONFIRM_WINDOW_MS = 2500;
    static constexpr int WRITE_CONFIRM_STEP_MS = 300;
    /** No SysEx request this soon after a channel-voice message: it would only be lost and retried. */
    static constexpr int CHANNEL_VOICE_QUIET_MS = 1000;
    /** How many slots in a row may go unanswered before a dump or transfer gives up on the synth. */
    static constexpr int MAX_CONSECUTIVE_FAILURES = 3;
    /** How often a step-by-step activity tells the listeners; every slot would re-lay the UI out hundreds of times a second. */
    static constexpr int PROGRESS_NOTIFY_EVERY = 5;

    explicit SynthSession (MidiHandler& midiHandler);
    ~SynthSession() override;

    void addListener (Listener* listener);
    void removeListener (Listener* listener);

    //==============================================================================
    // actions

    /** Call after MidiHandler::connectMidiDevices(): identifies the synth, reads its settings, derives the channel and the current preset, starts following it. */
    void connect();
    void disconnect();

    /** Moves the synth's selection pointer to the program and makes it recall the preset (0x32 00). Discards unsaved edits on the synth. */
    void selectProgram (int program);

    /** Makes the synth recall the preset it points at (0x32 00), discarding unsaved edits on the synth. */
    void reloadStoredProgram();

    /** Reads the settings block now (one poll). */
    void refresh();

    /**
     * Reads all 400 program slots, one request at a time. Each reply reaches the program list the same way a
     * single read does; an empty slot answers with the bare F0 F7 the protocol uses for it and leaves the slot
     * empty rather than timing out. Gives up if the synth stops answering several slots in a row.
     */
    void readAllPrograms();

    /**
     * Writes the programs to the synth, each one confirmed by its status reply and then by reading the slot
     * back - the only check that catches a write the synth accepts and does not store. Slots that could not be
     * confirmed are named at the end; the rest are still written.
     */
    void writePrograms (std::vector<std::shared_ptr<ProgramMessage>> programs);

    /**
     * Changes one setting on the synth: patches the block, writes it back whole (there is no per-field write) after
     * a short debounce, then reads back until the synth shows the value or WRITE_CONFIRM_WINDOW_MS pass. Meanwhile
     * the value is kept against stale read-backs; a value that never lands is undone in the block and reported.
     */
    void writeSetting (Pro800Settings setting, int value);

    /** nullopt = derive the channel from the synth. */
    void setManualChannel (std::optional<int> channel);

    /** nullopt = accept what the synth transmits on; 0 = every channel; 1-16 = that channel. */
    void setManualReceiveChannel (std::optional<int> channel);

    /**
     * Reads the physical control surface (every 0x70 index and the 26 knobs), shows it in the Front Panel
     * controls and sends it to the synth as CC, so that what the synth plays is what the panel - and the
     * plugin - shows. Without that last step the two disagree: a preset load does not move the knobs, so a
     * knob's position is not what is heard until it is touched.
     *
     * The shape switch is resolved against the LFO shape last seen here (see
     * Pro800PanelConversion::lfoShapeFromSwitch()); the shape is left alone while none is known.
     *
     * Needs a synth that receives CC (MIDI CC Mode RX or Send & Receive) on a known channel; refuses with an
     * explanation otherwise, since the sound would silently stay as it was.
     */
    void alignWithPanel();

    /** Reads the control surface without touching the controls; nullopt if the synth stopped answering. */
    void readPanelState (std::function<void (std::optional<Pro800PanelState>)> callback);

    /** The last panel reading of this connection, if any. */
    const std::optional<Pro800PanelState>& getLastPanelState() const { return this->lastPanelState; }

    /** Why the synth would not hear a CC right now (so alignWithPanel() and the Front Panel controls do nothing), or empty if it would. */
    juce::String reasonCCWouldNotArrive() const;

    //==============================================================================
    // state

    ConnectionState getConnectionState() const { return this->connectionState; }
    bool isConnected() const { return this->connectionState == ConnectionState::CONNECTED; }
    const juce::String& getFirmwareVersion() const { return this->firmwareVersion; }
    bool isFirmwareSupported() const { return this->firmwareSupported; }
    const Channels& getChannels() const { return this->channels; }
    const Pointer& getPointer() const { return this->pointer; }
    const Provenance& getProvenance() const { return this->provenance; }

    /** What the session is doing right now ("Connecting...", "Reading programs 12/400"), empty when idle. */
    const juce::String& getActivity() const { return this->activity; }
    bool isBusy() const { return this->activity.isNotEmpty(); }

    /** How far a step-by-step activity has got; total is 0 for the ones that have no steps to count. */
    int getActivityDone() const { return this->activityDone; }
    int getActivityTotal() const { return this->activityTotal; }
    bool isActivityCancellable() const { return this->activityTotal > 0; }

    /** Stops a running dump or transfer after the request in flight; the rest is left undone. */
    void cancelActivity();

    /** Connected, idle, and no program transfer running (whose replies would be mistaken for ours). */
    bool canStartAction() const;

    /** The last thing that went wrong, empty once the next action succeeds. */
    const juce::String& getLastError() const { return this->lastError; }

    /** The settings block as last read (nullptr before the first read). */
    std::shared_ptr<SettingsMessage> getSettings() const { return this->settings; }

    const SettingsWriteStatus& getSettingsWriteStatus() const { return this->settingsWriteStatus; }

    /** The stored record of the preset the pointer names (nullptr if unknown or the slot is empty). */
    std::shared_ptr<ProgramMessage> getPointerProgram() const { return this->pointerProgram; }

private:
    using ReplyCallback = std::function<void (const juce::MidiMessage* reply)>;
    using SettingsCallback = std::function<void (std::shared_ptr<SettingsMessage> settings)>;
    using ProgramCallback = std::function<void (std::shared_ptr<ProgramMessage> program, bool slotIsEmpty)>;

    // primitives, each calling back on the message thread (with nullptr / false on failure)
    void sendRequest (const juce::MidiMessage& message, SysExMatchers::Matcher matcher, const juce::String& description, int retries, bool isPolling, ReplyCallback callback);
    void readSettings (bool isPolling, SettingsCallback callback);
    void readProgram (int program, bool isPolling, ProgramCallback callback);
    void readDipSwitches (std::function<void (std::optional<int> dipSum)> callback);
    void readDipSwitch (size_t which, std::shared_ptr<std::map<Pro800PanelIndex, int>> values, std::function<void (std::optional<int> dipSum)> callback);
    void readPanelStep (size_t step, std::shared_ptr<Pro800PanelState> state, std::function<void (std::optional<Pro800PanelState>)> callback);

    using ProgramList = std::vector<std::shared_ptr<ProgramMessage>>;
    void readAllProgramsStep (int program, int consecutiveFailures);
    void writeProgramsStep (std::shared_ptr<ProgramList> programs, size_t index, std::shared_ptr<std::vector<int>> unconfirmed, int consecutiveFailures);
    /** Writes one program and reads it back; confirmed is false if the synth did not store exactly what it was sent. */
    void writeProgramVerified (const ProgramMessage& program, std::function<void (bool answered, bool confirmed)> callback);

    void writeSettings (std::shared_ptr<SettingsMessage> newSettings, std::function<void (bool accepted)> callback);
    void sendReload (std::function<void (bool accepted)> callback);

    /** Re-reads the settings block every WRITE_CONFIRM_STEP_MS until `landed` holds or the window closes. */
    void waitForSettings (std::function<bool (const SettingsMessage&)> landed, double deadline, std::function<void (bool confirmed)> callback);

    // sequences
    void continueConnectWithSettings (std::shared_ptr<SettingsMessage> newSettings);
    void finishConnect();
    void poll (bool isPolling);
    /** Takes the pointer from the settings block; if it moved (and mirrorIfChanged), shows the stored record in the controls. */
    void updatePointer (bool mirrorIfChanged, bool isPolling, std::function<void()> then);
    /** Reads the stored record and shows it in the controls: after a select or a reload it is what the synth plays. */
    void showStoredProgram (int program, std::function<void()> then);

    // state changes
    void applySettings (std::shared_ptr<SettingsMessage> newSettings);
    void applySettings (std::shared_ptr<SettingsMessage> newSettings, std::optional<int> dipSum);
    /** Re-applies every pending write to a freshly read block: our value is the truth until the synth shows it, or the window closes. */
    void protectPendingWrites (SettingsMessage& newSettings);
    /** Sends the block with every pending value and starts reading back. */
    void flushPendingWrites();
    /** Reads the block every WRITE_CONFIRM_STEP_MS while writes are pending, then reports what did not land. */
    void verifyPendingWrites();
    void notifySettingsChanged();
    void applyChannels();
    void setActivity (const juce::String& newActivity, int done = 0, int total = 0);
    /** Moves a step-by-step activity on, telling the listeners only now and then: 400 of these run in a second or two. */
    void setActivityProgress (int done);
    void fail (const juce::String& error);
    /** Like fail(), for something that worked but is worth knowing about; shown in the same place. */
    void warn (const juce::String& note);
    void notify();

    void timerCallback() override;
    void channelVoiceSent (const juce::MidiMessage& message) override;
    void channelVoiceReceived (const juce::MidiMessage& message) override;

    MidiHandler& midiHandler;
    juce::ListenerList<Listener> listeners;

    ConnectionState connectionState = ConnectionState::DISCONNECTED;
    juce::String firmwareVersion;
    bool firmwareSupported = false;
    Channels channels;
    Pointer pointer;
    Provenance provenance;
    juce::String activity;
    juce::String activityDescription; // without the "12/400"
    int activityDone = 0;
    int activityTotal = 0;
    bool cancelRequested = false;
    juce::String lastError;

    std::shared_ptr<SettingsMessage> settings;
    std::shared_ptr<ProgramMessage> pointerProgram;
    std::optional<int> pointerProgramReadFor; // the program pointerProgram (or its emptiness) was read for
    std::optional<Pro800PanelState> lastPanelState;
    /** The LFO shape as last seen here - from a record shown in the controls, or from a CC sent or received. */
    std::optional<int> currentLfoShape;
    void noteLfoShapeFromCC (const juce::MidiMessage& message);
    int pollFailures = 0;

    struct PendingWrite
    {
        int value;
        double deadline;
        bool written = false; // false while still waiting for the debounce
        int readBack = 0; // what the synth showed instead, once the window has closed
    };
    std::map<Pro800Settings, PendingWrite> pendingWrites;
    std::vector<std::pair<Pro800Settings, PendingWrite>> failedWrites; // collected by protectPendingWrites(), reported by verifyPendingWrites()
    SettingsWriteStatus settingsWriteStatus;
    bool verifyingWrites = false;

    struct DebounceTimer : public juce::Timer
    {
        std::function<void()> onFire;
        void timerCallback() override
        {
            stopTimer();
            if (onFire)
            {
                onFire();
            }
        }
    };
    DebounceTimer writeDebounce;

    // declared last: every callback checks the weak reference, so nothing runs into a destroyed session
    JUCE_DECLARE_WEAK_REFERENCEABLE (SynthSession)
};
