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

#include "MidiHandler.h"

#include "Pro800MessageFactory.h"
#include "Pro800MidiMessage.h"
#include "ProgramMessage.h"

#include "../ui/MidiComponent.h"

//==============================================================================
MidiHandler::MidiHandler()
    : sysExExchange ([this] (const juce::MidiMessage& message, bool isPolling) { sendMidiMessage (message, isPolling); })
{
}

MidiHandler::~MidiHandler()
{
    // 1. close the devices: no more incoming callbacks after this
    {
        const juce::ScopedLock lock (this->deviceLock);
        this->midiInput.reset();
        this->midiOutput.reset();
    }

    // 2. whatever is still queued will never be delivered
    cancelPendingUpdate();
}

void MidiHandler::addListener (Listener* listener)
{
    this->listeners.add (listener);
}

void MidiHandler::removeListener (Listener* listener)
{
    this->listeners.remove (listener);
}

void MidiHandler::setMidiChannel (uint8_t channel)
{
    this->midiChannel = channel;
}

uint8_t MidiHandler::getMidiChannel() const
{
    return this->midiChannel;
}

void MidiHandler::setInboundChannel (uint8_t channel)
{
    this->inboundChannel = channel;
}

void MidiHandler::connectMidiDevices (const juce::String& inputDeviceIdentifier, const juce::String& outputDeviceIdentifier)
{
    // whatever was waiting for a reply from the old device will not get one
    cancelExchanges();

    {
        const juce::ScopedLock lock (this->deviceLock);
        this->midiInput = juce::MidiInput::openDevice (inputDeviceIdentifier, this);
        this->midiOutput = juce::MidiOutput::openDevice (outputDeviceIdentifier);

        if (this->midiInput)
        {
            this->midiInput->start();
        }
    }

    this->portOpenedAt = juce::Time::getMillisecondCounterHiRes();
    this->recentChannelVoice.clear();
}

bool MidiHandler::hasOpenDevices() const
{
    const juce::ScopedLock lock (this->deviceLock);
    return this->testOutput != nullptr || (this->midiInput != nullptr && this->midiOutput != nullptr);
}

void MidiHandler::setTestTransport (std::function<void (const juce::MidiMessage&)> output)
{
    cancelExchanges();

    {
        const juce::ScopedLock lock (this->deviceLock);
        this->testOutput = std::move (output);
    }

    this->portOpenedAt = juce::Time::getMillisecondCounterHiRes() - PORT_OPEN_BURST_MS; // no burst window in tests
    this->recentChannelVoice.clear();
}

//==============================================================================
void MidiHandler::handleIncomingMidiMessage (juce::MidiInput* /*source*/, const juce::MidiMessage& message)
{
    // MIDI driver thread: just hand it over to the message thread
    queueEvent (MidiEvent { message, false });
}

void MidiHandler::queueEvent (MidiEvent event)
{
    {
        const juce::ScopedLock lock (this->pendingEventsLock);
        this->pendingEvents.push_back (std::move (event));
    }

    triggerAsyncUpdate();
}

void MidiHandler::handleAsyncUpdate()
{
    // take the whole batch so that the lock is not held while the components run
    // (they may send messages themselves, which queues again)
    std::vector<MidiEvent> batch;
    {
        const juce::ScopedLock lock (this->pendingEventsLock);
        batch.swap (this->pendingEvents);
    }

    for (const auto& event : batch)
    {
        handleEvent (event);
    }
}

void MidiHandler::handleEvent (const MidiEvent& event)
{
    const juce::MidiMessage& message = event.message;

    // getChannel() is 0 for SysEx and other channel-less messages: those always pass
    if (!event.sent && message.getChannel() != 0)
    {
        if (this->inboundChannel != 0 && message.getChannel() != this->inboundChannel)
        {
            return; // not our channel
        }

        // the synth's own MIDI Thru sends everything we send straight back; and right after the port opens it
        // delivers the knob movements that were buffered while nothing listened (docs, "Transport behaviour")
        if (consumeEcho (message) || juce::Time::getMillisecondCounterHiRes() - this->portOpenedAt < PORT_OPEN_BURST_MS)
        {
            return;
        }
    }

    // a received SysEx may be the reply an exchange is waiting for; it is still passed on to the components below
    bool consumedByExchange = false;
    bool isPolling = event.isPolling;
    if (!event.sent && message.isSysEx())
    {
        isPolling = this->sysExExchange.isCurrentPolling();
        consumedByExchange = this->sysExExchange.offerInbound (message);
        isPolling = consumedByExchange && isPolling;
    }

    // note: iterate over copies of the component lists so that a component may (un)register from within its handler
    const juce::String logPrefix = (event.sent ? "Sent message:" : "Received message:");
    for (auto* component : componentsFor (MessageType::MIDI_LOG))
    {
        component->handleMidiLog (message, logPrefix, isPolling);
    }

    if (event.sent)
    {
        // we sent it ourselves. Don't do anything.
        return;
    }

    if (message.getChannel() != 0)
    {
        this->listeners.call ([&] (Listener& l) { l.channelVoiceReceived (message); });
    }

    if (message.isController())
    {
        // any controller number is a legal Pro800CCMessages value; components only react to the ones they registered for
        const auto midiCC = static_cast<Pro800CCMessages> (message.getControllerNumber());
        const auto value = (uint8_t) message.getControllerValue();

        const juce::Array<MidiComponent*> ccComponents (this->midiCCComponents);
        for (auto* component : ccComponents)
        {
            component->handleMidiCCMessage (midiCC, value);
        }
    }
    else if (message.isSysEx())
    {
        std::shared_ptr<Pro800MidiMessage> pro800Message = Pro800MessageFactory::createMidiMessage (message);
        if (!pro800Message)
        {
            if (!consumedByExchange) // an empty slot's bare F0 F7 is a regular answer to a read, not an invalid message
            {
                juce::Logger::writeToLog ("[WARNING] Received invalid pro800Message");
            }
            return;
        }

        MessageType type = pro800Message->getMessageType();

        for (auto* component : componentsFor (type))
        {
            component->handlePro800Message (type, pro800Message);
        }
    }
}

bool MidiHandler::consumeEcho (const juce::MidiMessage& message)
{
    const double now = juce::Time::getMillisecondCounterHiRes();
    while (!this->recentChannelVoice.empty() && now - this->recentChannelVoice.front().sentAt > ECHO_WINDOW_MS)
    {
        this->recentChannelVoice.pop_front();
    }

    const std::vector<uint8_t> bytes (message.getRawData(), message.getRawData() + message.getRawDataSize());
    for (auto it = this->recentChannelVoice.begin(); it != this->recentChannelVoice.end(); ++it)
    {
        if (it->bytes == bytes)
        {
            this->recentChannelVoice.erase (it); // the synth echoes each message once
            return true;
        }
    }

    return false;
}

//==============================================================================
void MidiHandler::registerMidiCCComponent (MidiComponent* component)
{
    this->midiCCComponents.addIfNotAlreadyThere (component);
}

void MidiHandler::unregisterMidiCCComponent (MidiComponent* component)
{
    this->midiCCComponents.removeAllInstancesOf (component);
}

void MidiHandler::registerMessageComponent (MessageType type, MidiComponent* component)
{
    this->midiComponents[type].addIfNotAlreadyThere (component);
}

void MidiHandler::unregisterMessageComponent (MessageType type, MidiComponent* component)
{
    const auto entry = this->midiComponents.find (type);
    if (entry != this->midiComponents.end())
    {
        entry->second.removeAllInstancesOf (component);
    }
}

juce::Array<MidiComponent*> MidiHandler::componentsFor (MessageType type) const
{
    const auto entry = this->midiComponents.find (type);
    return entry != this->midiComponents.end() ? entry->second : juce::Array<MidiComponent*>();
}

//==============================================================================
void MidiHandler::sendMidiCCMessage (Pro800CCMessages midiCC, uint8_t value)
{
    sendChannelVoice (juce::MidiMessage::controllerEvent (midiChannel, static_cast<int> (midiCC), (int) value));
}

void MidiHandler::sendProgramChange (uint8_t program)
{
    sendChannelVoice (juce::MidiMessage::programChange (midiChannel, (int) program));
}

void MidiHandler::sendChannelVoice (const juce::MidiMessage& message)
{
    JUCE_ASSERT_MESSAGE_THREAD

    const double now = juce::Time::getMillisecondCounterHiRes();
    this->recentChannelVoice.push_back ({ std::vector<uint8_t> (message.getRawData(), message.getRawData() + message.getRawDataSize()), now });
    this->lastChannelVoiceSentAt = now;

    sendMidiMessage (message);

    this->listeners.call ([&] (Listener& l) { l.channelVoiceSent (message); });
}

void MidiHandler::sendChannelVoiceBurst (const std::vector<juce::MidiMessage>& messages)
{
    for (const auto& message : messages)
    {
        sendChannelVoice (message);
    }
}

bool MidiHandler::channelVoiceSentWithin (int milliseconds) const
{
    return juce::Time::getMillisecondCounterHiRes() - this->lastChannelVoiceSentAt < milliseconds;
}

void MidiHandler::mirrorProgram (const ProgramMessage& program)
{
    if (!program.isValid())
    {
        return; // an empty placeholder: our controls have nothing to show
    }

    const juce::Array<MidiComponent*> ccComponents (this->midiCCComponents);
    for (auto* component : ccComponents)
    {
        component->loadFromProgram (program);
    }
}

void MidiHandler::mirrorPanel (const Pro800PanelValues& values)
{
    const juce::Array<MidiComponent*> ccComponents (this->midiCCComponents);
    for (auto* component : ccComponents)
    {
        component->loadFromPanel (values);
    }
}

void MidiHandler::exchange (SysExExchange::Request request)
{
    this->sysExExchange.enqueue (std::move (request));
}

void MidiHandler::cancelExchanges()
{
    this->sysExExchange.cancelAll();
}

bool MidiHandler::isExchangeBusy() const
{
    return this->sysExExchange.isBusy();
}

void MidiHandler::sendMidiMessage (const juce::MidiMessage& message)
{
    sendMidiMessage (message, false);
}

void MidiHandler::sendMidiMessage (const juce::MidiMessage& message, bool isPolling)
{
    const juce::ScopedLock lock (this->deviceLock);

    if (this->testOutput != nullptr)
    {
        queueEvent (MidiEvent { message, true, isPolling });
        this->testOutput (message);
        return;
    }

    if (!this->midiOutput)
    {
        juce::Logger::writeToLog ("[ERROR] Cannot send MIDI message: MIDI output device is not open!");
        return;
    }

    queueEvent (MidiEvent { message, true, isPolling }); // for the log
    this->midiOutput->sendMessageNow (message);
}
