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

#include "Pro800MidiMessage.h"
#include "Pro800MessageFactory.h"
#include "ProgramMessage.h"
#include "VersionMessage.h"

#include "../ui/MidiComponent.h"

namespace
{
    // generous: the sender thread checks for exit after every message, so it stops within one interval
    constexpr int SENDER_THREAD_STOP_TIMEOUT_MS = 2000;
}

//==============================================================================
MidiHandler::BackgroundSender::BackgroundSender(MidiHandler& handler)
    : juce::Thread("Pro800 MIDI sender"), owner(handler)
{
}

void MidiHandler::BackgroundSender::send(std::vector<juce::MidiMessage> newMessages, int newIntervalMs, const juce::String& newDescription)
{
    stopThread(SENDER_THREAD_STOP_TIMEOUT_MS); // no-op if idle, otherwise cancels the running sequence

    this->messages = std::move(newMessages);
    this->intervalMs = newIntervalMs;
    this->description = newDescription;

    if ( !this->messages.empty() )
    {
        startThread();
    }
}

void MidiHandler::BackgroundSender::run()
{
    const int numTotal = (int) this->messages.size();
    int numSent = 0;

    for ( ; numSent < numTotal && !threadShouldExit(); numSent++ )
    {
        this->owner.sendMidiMessage(this->messages[(size_t) numSent]);
        this->owner.queueEvent(ProgressEvent { this->description, numSent + 1, numTotal });

        if ( numSent + 1 < numTotal )
        {
            wait(this->intervalMs); // returns early when stopThread() notifies us
        }
    }

    this->owner.queueEvent(FinishedEvent { numSent < numTotal });
}

//==============================================================================
MidiHandler::MidiHandler()
{
}

MidiHandler::~MidiHandler()
{
    // 1. nobody may call sendMidiMessage() from another thread anymore
    cancelBackgroundSending();

    // 2. close the devices: no more incoming callbacks after this
    {
        const juce::ScopedLock lock(this->deviceLock);
        this->midiInput.reset();
        this->midiOutput.reset();
    }

    // 3. whatever is still queued will never be delivered
    cancelPendingUpdate();
}

void MidiHandler::addListener(Listener* listener)
{
    this->listeners.add(listener);
}

void MidiHandler::removeListener(Listener* listener)
{
    this->listeners.remove(listener);
}

void MidiHandler::setMidiChannel (uint8_t channel)
{
    this->midiChannel = channel;
}

void MidiHandler::connectMidiDevices(const juce::String& inputDeviceIdentifier, const juce::String& outputDeviceIdentifier)
{
    // the sender thread must not be in the middle of using the old output device
    cancelBackgroundSending();

    {
        const juce::ScopedLock lock(this->deviceLock);
        this->midiInput = juce::MidiInput::openDevice(inputDeviceIdentifier, this);
        this->midiOutput = juce::MidiOutput::openDevice(outputDeviceIdentifier);

        if ( this->midiInput )
        {
            this->midiInput->start();
        }
    }

    if ( this->midiInput )
    {
        sendMidiMessage(VersionMessage::request());
    }
}

//==============================================================================
void MidiHandler::handleIncomingMidiMessage (juce::MidiInput */*source*/, const juce::MidiMessage& message)
{
    // MIDI driver thread: just hand it over to the message thread
    queueEvent(MidiEvent { message, false });
}

void MidiHandler::queueEvent(QueuedEvent event)
{
    {
        const juce::ScopedLock lock(this->pendingEventsLock);
        this->pendingEvents.push_back(std::move(event));
    }

    triggerAsyncUpdate();
}

void MidiHandler::handleAsyncUpdate()
{
    // take the whole batch so that the lock is not held while the components run
    // (they may send messages themselves, which queues again)
    std::vector<QueuedEvent> batch;
    {
        const juce::ScopedLock lock(this->pendingEventsLock);
        batch.swap(this->pendingEvents);
    }

    for ( const auto& event : batch )
    {
        std::visit([this] (const auto& e) { handleEvent(e); }, event);
    }
}

void MidiHandler::handleEvent(const ProgressEvent& event)
{
    this->listeners.call([&] (Listener& l) { l.backgroundSendingProgress(event.description, event.numSent, event.numTotal); });
}

void MidiHandler::handleEvent(const FinishedEvent& event)
{
    this->listeners.call([&] (Listener& l) { l.backgroundSendingFinished(event.cancelled); });
}

void MidiHandler::handleEvent(const MidiEvent& event)
{
    const juce::MidiMessage& message = event.message;

    // getChannel() is 0 for SysEx and other channel-less messages: those always pass
    if ( message.getChannel() != 0 && message.getChannel() != this->midiChannel )
    {
        // not our channel
        return;
    }

    // note: iterate over copies of the component lists so that a component may (un)register from within its handler
    const juce::String logPrefix = (event.sent ? "Sent message:" : "Received message:");
    for(auto *component : this->midiComponents[MessageType::MIDI_LOG_MESSAGE])
    {
        component->handleMidiLog(message, logPrefix);
    }

    if ( event.sent )
    {
        // we sent it ourselves. Don't do anything.
        return;
    }

    if (message.isController())
    {
        uint8_t midiCC = (uint8_t) message.getControllerNumber();
        uint8_t value = (uint8_t) message.getControllerValue();

        const juce::Array<MidiComponent *> ccComponents(this->midiCCComponents);
        for(auto *component : ccComponents )
        {
            component->handleMidiCCMessage(midiCC, value);
        }
    }
    else if ( message.isSysEx())
    {
        std::shared_ptr<Pro800MidiMessage> pro800Message = Pro800MessageFactory::createMidiMessage(message);
        if ( !pro800Message )
        {
            juce::Logger::writeToLog("[WARNING] Received invalid pro800Message");
            return;
        }

        MessageType type = pro800Message->getMessageType();

        for(auto *component : this->midiComponents[type] )
        {
            component->handlePro800Message(type, pro800Message);
        }
    }
}

//==============================================================================
void MidiHandler::registerMidiCCComponent(MidiComponent *component)
{
    this->midiCCComponents.addIfNotAlreadyThere(component);
}

void MidiHandler::unregisterMidiCCComponent(MidiComponent *component)
{
    this->midiCCComponents.removeAllInstancesOf(component);
}

void MidiHandler::registerMessageComponent(MessageType type, MidiComponent *component)
{
    this->midiComponents.getReference(type).addIfNotAlreadyThere(component);
}

void MidiHandler::unregisterMessageComponent(MessageType type, MidiComponent *component)
{
    if ( this->midiComponents.contains(type) )
    {
        this->midiComponents.getReference(type).removeAllInstancesOf(component);
    }
}

//==============================================================================
void MidiHandler::sendMidiCCMessage (uint8_t midiCC, uint8_t value)
{
    sendMidiMessage(juce::MidiMessage::controllerEvent (midiChannel, (int) midiCC, (int) value));
}

void MidiHandler::sendProgramChange (uint8_t program)
{
    sendMidiMessage(juce::MidiMessage::programChange (midiChannel, (int) program));
}

void MidiHandler::sendMidiMessage (const juce::MidiMessage& message)
{
    const juce::ScopedLock lock(this->deviceLock);

    if ( !this->midiOutput )
    {
        juce::Logger::writeToLog("[ERROR] Cannot send MIDI message: MIDI output device is not open!");
        return;
    }

    queueEvent(MidiEvent { message, true }); // for the log
    this->midiOutput->sendMessageNow(message);
}

void MidiHandler::sendMidiMessagesInBackground(std::vector<juce::MidiMessage> messages, int intervalMs, const juce::String& progressDescription)
{
    {
        // report a missing device once here instead of once per message from the sender thread
        const juce::ScopedLock lock(this->deviceLock);
        if ( !this->midiOutput )
        {
            juce::Logger::writeToLog("[ERROR] Cannot send MIDI messages: MIDI output device is not open!");
            return;
        }
    }

    this->backgroundSender.send(std::move(messages), intervalMs, progressDescription);
}

void MidiHandler::requestProgramDump()
{
    std::vector<juce::MidiMessage> requests;
    requests.reserve(ProgramMessage::NUM_PROGRAMS);

    for ( int program = 0; program < ProgramMessage::NUM_PROGRAMS; program++ )
    {
        requests.push_back(ProgramMessage::request(program));
    }

    sendMidiMessagesInBackground(std::move(requests), PROGRAM_DUMP_REQUEST_INTERVAL_MS, "Requesting program");
}

void MidiHandler::cancelBackgroundSending()
{
    this->backgroundSender.stopThread(SENDER_THREAD_STOP_TIMEOUT_MS);
}
