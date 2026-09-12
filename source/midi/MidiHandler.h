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

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include "../tailoring/Pro800Constants.h"

#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

class MidiComponent;

/**
 * Owns the MIDI devices and dispatches everything that is sent or received to the registered
 * MidiComponents.
 *
 * Threading:
 *  - Incoming MIDI arrives on the MIDI driver thread. It is queued and delivered to the components
 *    on the message thread via the AsyncUpdater, so the components never see another thread.
 *  - sendMidiMessage() may be called from the message thread or from the background sender thread.
 *  - Everything else (device connection, component registration) is message-thread only.
 */
class MidiHandler : public juce::MidiInputCallback, private juce::AsyncUpdater
{
public:
    static constexpr int PROGRAM_DUMP_REQUEST_INTERVAL_MS = 10;

    /** Receives the progress of background sending. All callbacks arrive on the message thread. */
    struct Listener
    {
        virtual ~Listener() = default;

        /** Called after each message of a background sequence has been sent. */
        virtual void backgroundSendingProgress(const juce::String& description, int numSent, int numTotal) = 0;

        /** Called when a background sequence has ended, either completely or because it was cancelled. */
        virtual void backgroundSendingFinished(bool cancelled) = 0;
    };

    MidiHandler();
    ~MidiHandler() override;

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

    void setMidiChannel(uint8_t channel);

    /** Closes the current devices and opens the given ones (empty identifier = none). Cancels background sending. */
    void connectMidiDevices(const juce::String& inputDeviceIdentifier, const juce::String& outputDeviceIdentifier);

    void registerMidiCCComponent(MidiComponent *component);
    void unregisterMidiCCComponent(MidiComponent *component);

    void registerMessageComponent(MessageType type, MidiComponent *component);
    void unregisterMessageComponent(MessageType type, MidiComponent *component);

    void sendMidiCCMessage(uint8_t midiCC, uint8_t value);
    void sendProgramChange (uint8_t program);

    /** Sends immediately. Thread-safe. */
    void sendMidiMessage(const juce::MidiMessage& message);

    /**
     * Sends the messages one by one from a background thread, pausing intervalMs between them,
     * so that the message thread stays responsive. A sequence that is still running is cancelled first.
     * Progress is reported to the listeners; progressDescription is passed along for display
     * (e.g. "Sending program" -> "Sending program 12/400").
     */
    void sendMidiMessagesInBackground(std::vector<juce::MidiMessage> messages, int intervalMs, const juce::String& progressDescription);

    /** Requests a dump of all programs from the device (in the background, see above). */
    void requestProgramDump();

    /** Stops a running background sequence; returns once the sender thread has stopped. */
    void cancelBackgroundSending();

private:
    // MidiInputCallback (MIDI driver thread)
    void handleIncomingMidiMessage (juce::MidiInput *source, const juce::MidiMessage& message) override;

    // AsyncUpdater (message thread)
    void handleAsyncUpdate() override;

    // events that other threads hand over to the message thread
    struct MidiEvent     { juce::MidiMessage message; bool sent; };
    struct ProgressEvent { juce::String description; int numSent; int numTotal; };
    struct FinishedEvent { bool cancelled; };
    using QueuedEvent = std::variant<MidiEvent, ProgressEvent, FinishedEvent>;

    void queueEvent(QueuedEvent event); // any thread

    void handleEvent(const MidiEvent& event);
    void handleEvent(const ProgressEvent& event);
    void handleEvent(const FinishedEvent& event);

    class BackgroundSender : public juce::Thread
    {
    public:
        explicit BackgroundSender(MidiHandler& handler);

        /** Cancels a running sequence, then starts sending the given one. Message thread only. */
        void send(std::vector<juce::MidiMessage> newMessages, int newIntervalMs, const juce::String& newDescription);

        void run() override;

    private:
        MidiHandler& owner;

        // only touched while the thread is not running
        std::vector<juce::MidiMessage> messages;
        int intervalMs = 0;
        juce::String description;
    };

    std::vector<QueuedEvent> pendingEvents;
    juce::CriticalSection pendingEventsLock;

    juce::ListenerList<Listener> listeners;

    // guards midiInput/midiOutput against sendMidiMessage() running on the sender thread
    juce::CriticalSection deviceLock;
    std::unique_ptr<juce::MidiInput> midiInput;
    std::unique_ptr<juce::MidiOutput> midiOutput;

    juce::Array<MidiComponent *> midiCCComponents;
    juce::HashMap<MessageType, juce::Array<MidiComponent *>> midiComponents;

    uint8_t midiChannel = 1;

    // declared last: stopped in the destructor before anything else is torn down
    BackgroundSender backgroundSender { *this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiHandler)
};
