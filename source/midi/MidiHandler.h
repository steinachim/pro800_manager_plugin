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

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

#include "../tailoring/Pro800CCConstants.h"
#include "../tailoring/Pro800Constants.h"
#include "SysExExchange.h"

#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <vector>

class MidiComponent;
class ProgramMessage;
struct Pro800PanelValues;

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
    /** Receives the channel-voice traffic. All callbacks arrive on the message thread. */
    struct Listener
    {
        virtual ~Listener() = default;

        /** A channel-voice message (CC, program change, note) went out through sendChannelVoice(). */
        virtual void channelVoiceSent (const juce::MidiMessage& /*message*/) {}

        /** A channel-voice message from the synth passed the channel filter and was not the echo of one we sent. */
        virtual void channelVoiceReceived (const juce::MidiMessage& /*message*/) {}
    };

    /** The synth echoes every channel-voice message back (MIDI Thru); an echo arriving later than this is not recognised as one. */
    static constexpr int ECHO_WINDOW_MS = 1000;

    /** Knob movements made while nothing held the port arrive as a burst right after it is opened; they are stale and dropped. */
    static constexpr int PORT_OPEN_BURST_MS = 500;

    MidiHandler();
    ~MidiHandler() override;

    void addListener (Listener* listener);
    void removeListener (Listener* listener);

    /** The channel (1-16) channel-voice messages go out on. */
    void setMidiChannel (uint8_t channel);
    uint8_t getMidiChannel() const;

    /** The channel incoming channel-voice messages are accepted on; 0 = every channel. */
    void setInboundChannel (uint8_t channel);

    /** Closes the current devices and opens the given ones (empty identifier = none). Cancels background sending and pending exchanges. */
    void connectMidiDevices (const juce::String& inputDeviceIdentifier, const juce::String& outputDeviceIdentifier);
    bool hasOpenDevices() const;

    /**
     * For tests: a stand-in for the MIDI devices. Everything sent goes to `output` instead of a device (nullptr
     * restores the devices); replies are injected with handleIncomingMidiMessage(). Counts as open devices.
     */
    void setTestTransport (std::function<void (const juce::MidiMessage&)> output);

    void registerMidiCCComponent (MidiComponent* component);
    void unregisterMidiCCComponent (MidiComponent* component);

    void registerMessageComponent (MessageType type, MidiComponent* component);
    void unregisterMessageComponent (MessageType type, MidiComponent* component);

    void sendMidiCCMessage (Pro800CCMessages midiCC, uint8_t value);
    void sendProgramChange (uint8_t program);

    /**
     * Sends a channel-voice message now (message thread) and remembers it, so that the synth's echo of it is
     * dropped on the way in and channelVoiceSentWithin() can report it.
     */
    void sendChannelVoice (const juce::MidiMessage& message);

    /**
     * Sends several channel-voice messages at once, all remembered for echo suppression. This is the shape of
     * traffic the synth's own knobs produce, and a few dozen three-byte messages are less than one program dump,
     * so they go out without pacing.
     */
    void sendChannelVoiceBurst (const std::vector<juce::MidiMessage>& messages);

    /** True if a channel-voice message went out within the last milliseconds: the next SysEx request is likely to need its retry. */
    bool channelVoiceSentWithin (int milliseconds) const;

    /**
     * Sets the controls of all CC components from the program (a stored record: the Pro-800 cannot be asked
     * for the sound it is playing). Sends nothing.
     */
    void mirrorProgram (const ProgramMessage& program);

    /** Sets the controls the physical panel determines (see Pro800PanelConversion). Sends nothing. */
    void mirrorPanel (const Pro800PanelValues& values);

    /** Sends a request and calls back with the reply (see SysExExchange). Message thread only. */
    void exchange (SysExExchange::Request request);

    /** Completes every pending exchange with no reply. */
    void cancelExchanges();

    /** True while a request is in flight or queued. */
    bool isExchangeBusy() const;

    /** Sends immediately. */
    void sendMidiMessage (const juce::MidiMessage& message);

    // MidiInputCallback (MIDI driver thread; also the injection point of the test transport)
    void handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message) override;

private:
    // AsyncUpdater (message thread)
    void handleAsyncUpdate() override;

    /** The MIDI driver thread hands received messages over to the message thread as these. */
    struct MidiEvent
    {
        juce::MidiMessage message;
        bool sent;
        bool isPolling = false;
    };

    void queueEvent (MidiEvent event); // any thread

    void handleEvent (const MidiEvent& event);

    /** True if the message is the echo of a channel-voice message we sent recently; forgets that message. */
    bool consumeEcho (const juce::MidiMessage& message);

    void sendMidiMessage (const juce::MidiMessage& message, bool isPolling);

    std::vector<MidiEvent> pendingEvents;
    juce::CriticalSection pendingEventsLock;

    juce::ListenerList<Listener> listeners;

    // guards midiInput/midiOutput against sendMidiMessage() running on the sender thread
    juce::CriticalSection deviceLock;
    std::unique_ptr<juce::MidiInput> midiInput;
    std::unique_ptr<juce::MidiOutput> midiOutput;
    std::function<void (const juce::MidiMessage&)> testOutput;

    juce::Array<MidiComponent*> midiCCComponents;
    std::map<MessageType, juce::Array<MidiComponent*>> midiComponents;

    /** A copy of the components registered for the type (empty if none): safe to iterate while they (un)register. */
    juce::Array<MidiComponent*> componentsFor (MessageType type) const;

    uint8_t midiChannel = 1;
    uint8_t inboundChannel = 0;

    struct SentChannelVoice
    {
        std::vector<uint8_t> bytes;
        double sentAt;
    };
    std::deque<SentChannelVoice> recentChannelVoice; // message thread only
    double lastChannelVoiceSentAt = 0.0;
    double portOpenedAt = 0.0;

    SysExExchange sysExExchange;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiHandler)
};
