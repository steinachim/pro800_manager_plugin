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

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>


#include "../PluginProcessor.h"
#include "MidiCallbackMessage.h"

#include "Pro800MidiMessage.h"
#include "Pro800MessageFactory.h"
#include "SettingsMessage.h"
#include "VersionMessage.h"

#include "../ui/MidiComponent.h"



MidiHandler::MidiHandler(Pro800ManagerAudioProcessor* parent) : juce::MidiInputCallback()
{
    this->processor = parent;
}

MidiHandler::~MidiHandler()
{
    
}

void MidiHandler::setMidiChannel (uint8_t channel)
{
    this->midiChannel = channel;
}

void MidiHandler::connectMidiDevices(const juce::String& inputDeviceName, const juce::String& outputDeviceName)
{
    this->midiInput = juce::MidiInput::openDevice(inputDeviceName, this);
    this->midiOutput = juce::MidiOutput::openDevice(outputDeviceName);

    if ( this->midiInput )
    {
        this->midiInput->start();
        sendMidiMessage(VersionMessage::request());
    }
}

void MidiHandler::handleIncomingMidiMessage (juce::MidiInput */*source*/, const juce::MidiMessage& message) 
{
    (new MidiCallbackMessage(this, message))->post();
}

void MidiHandler::handleMidiMessage (const juce::MidiMessage& message, bool sent)
{    
    if ( !message.isSysEx() && message.getChannel() != 0 && message.getChannel() != this->midiChannel )
    {
        // not our channel
        return;
    }

    // don't log note messages... 
    for(auto *component : this->midiComponents.getReference(MessageType::MIDI_LOG_MESSAGE ))
    {
        juce::String logPrefix = (sent ? "Sent message:" : "Received message:");
        component->handleMidiLog(message, logPrefix);
    }

    if ( sent )
    {
        // we sent it ourselves. Don't do anything.
        return;
    }


    if (message.isController())
    {
        uint8_t midiCC = (uint8_t) message.getControllerNumber();
        uint8_t value = (uint8_t) message.getControllerValue();

        for(auto *component : this->midiCCComponents )
        {
            component->handleMidiCCMessage(midiCC, value);
        }
    }
    else if ( message.isSysEx())
    {
        std::shared_ptr<Pro800MidiMessage> pro800Message = Pro800MessageFactory::createMidiMessage(message);
        if ( !pro800Message )
        {
            std::cerr << "[WARNING] Received invalid pro800Message" << std::endl;
            return;
        }

        MessageType type = pro800Message->getMessageType();

        for(auto *component : this->midiComponents.getReference(type) )
        {
                component->handlePro800Message(type, pro800Message);
        }
    }
}

void MidiHandler::registerMidiCCComponent(MidiComponent *component)
{
    this->midiCCComponents.add(component);
}

void MidiHandler::unregisterMidiCCComponent(MidiComponent *component)
{
    this->midiCCComponents.removeAllInstancesOf(component);
}

void MidiHandler::registerMessageComponent(MessageType type, MidiComponent *component)
{
    this->midiComponents.getReference(type).add(component);
}

void MidiHandler::unregisterMessageComponent(MessageType type, MidiComponent *component)
{
    this->midiComponents.getReference(type).removeAllInstancesOf(component);
}

void MidiHandler::sendMidiCCMessage (uint8_t midiCC, uint8_t value)
{
    juce::MidiMessage message = juce::MidiMessage::controllerEvent (midiChannel, (int) midiCC, (int) value);  
    sendMidiMessage(message);
}

void MidiHandler::sendProgramChange (uint8_t program)
{
    juce::MidiMessage message = juce::MidiMessage::programChange (midiChannel, (int) program);  
    sendMidiMessage(message);
}

void MidiHandler::sendMidiMessage (const juce::MidiMessage& message)
{
    if ( !this->midiOutput )
    {
        std::cerr << "[ERROR] Cannot send MIDI message: MIDI output device is not open!" << std::endl;
        return;
    }

    (new MidiCallbackMessage(this, message, true))->post();

    this->midiOutput->sendMessageNow(message);
}
