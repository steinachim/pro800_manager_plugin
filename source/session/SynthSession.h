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
        bool panelIsSound = false; // PANEL: the user vouched that the synth is in manual mode

        bool isEdited() const { return editedInApp || editedOnSynth; }

        /** e.g. "B05 as stored", "B05 + edits (in app)", "unknown" */
        juce::String describe() const;
    };

    struct Listener
    {
        virtual ~Listener() = default;

        /** Something above changed: read the getters. Message thread. */
        virtual void synthSessionChanged() = 0;
    };

    /** How often the settings block is read to follow the synth's own preset changes. */
    static constexpr int POLL_INTERVAL_MS = 1500;
    /** A settings write commits on the synth's own cycle: about one in fourteen only shows ~1.6 s later. */
    static constexpr int WRITE_CONFIRM_WINDOW_MS = 2500;
    static constexpr int WRITE_CONFIRM_STEP_MS = 300;
    /** No SysEx request this soon after a channel-voice message: it would only be lost and retried. */
    static constexpr int CHANNEL_VOICE_QUIET_MS = 1000;

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
     * Changes one setting on the synth: patches the block, writes it back whole (there is no per-field write) and
     * keeps the value against stale read-backs until the synth confirms it or WRITE_CONFIRM_WINDOW_MS pass.
     */
    void writeSetting (Pro800Settings setting, int value);

    /** nullopt = derive the channel from the synth. */
    void setManualChannel (std::optional<int> channel);

    /** nullopt = accept what the synth transmits on; 0 = every channel; 1-16 = that channel. */
    void setManualReceiveChannel (std::optional<int> channel);

    /** Front Panel tab, phase 3: the controls were set from the physical panel. */
    void notePanelMirrored (bool panelIsSound);

    //==============================================================================
    // state

    ConnectionState getConnectionState() const { return this->connectionState; }
    bool isConnected() const { return this->connectionState == ConnectionState::CONNECTED; }
    const juce::String& getFirmwareVersion() const { return this->firmwareVersion; }
    bool isFirmwareSupported() const { return this->firmwareSupported; }
    const Channels& getChannels() const { return this->channels; }
    const Pointer& getPointer() const { return this->pointer; }
    const Provenance& getProvenance() const { return this->provenance; }

    /** What the session is doing right now ("Connecting...", "Selecting B05..."), empty when idle. */
    const juce::String& getActivity() const { return this->activity; }
    bool isBusy() const { return this->activity.isNotEmpty(); }

    /** Connected, idle, and no program transfer running (whose replies would be mistaken for ours). */
    bool canStartAction() const;

    /** The last thing that went wrong, empty once the next action succeeds. */
    const juce::String& getLastError() const { return this->lastError; }

    /** The settings block as last read (nullptr before the first read). */
    std::shared_ptr<SettingsMessage> getSettings() const { return this->settings; }

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
    void applyChannels();
    void setActivity (const juce::String& newActivity);
    void fail (const juce::String& error);
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
    juce::String lastError;

    std::shared_ptr<SettingsMessage> settings;
    std::shared_ptr<ProgramMessage> pointerProgram;
    std::optional<int> pointerProgramReadFor; // the program pointerProgram (or its emptiness) was read for
    int pollFailures = 0;

    struct PendingWrite
    {
        int value;
        double deadline;
    };
    std::map<Pro800Settings, PendingWrite> pendingWrites;

    // declared last: every callback checks the weak reference, so nothing runs into a destroyed session
    JUCE_DECLARE_WEAK_REFERENCEABLE (SynthSession)
};
