#include "MidiHandler.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "../PluginProcessor.h"
#include "MidiCallbackMessage.h"

#include "Pro800MidiMessage.h"
#include "Pro800MessageFactory.h"
#include "SettingsMessage.h"

#include "../ui/MidiComponent.h"



MidiHandler::MidiHandler(Pro800ManagerAudioProcessor* parent)
{
    this->processor = parent;
}

MidiHandler::~MidiHandler()
{
}

void MidiHandler::handleMidiMessage (const juce::MidiMessage& message, bool sent)
{    
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
    juce::MidiMessage message = juce::MidiMessage::controllerEvent (1, (int) midiCC, (int) value);  
    sendMidiMessage(message);
}

void MidiHandler::sendMidiMessage (const juce::MidiMessage& message)
{
    (new MidiCallbackMessage(this, message, true))->post();
    this->processor->sendMidiMessage(message);
}
