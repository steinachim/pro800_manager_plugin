#include "MidiHandler.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "../PluginProcessor.h"
#include "Pro800MidiMessage.h"
#include "Pro800MessageFactory.h"
#include "SettingsMessage.h"

#include "../ui/UiHelpers.h"
#include "../ui/MidiComponent.h"



MidiHandler::MidiHandler(Pro800ManagerAudioProcessor* parent)
{
    this->processor = parent;
}

MidiHandler::~MidiHandler()
{
}

void MidiHandler::handleMidiMessage (const juce::MidiMessage& message)
{
    for(auto *component : logComponents)
    {
        UiHelpers::setComponentLogValue(component, message, "Received message:");
    }

    if (message.isController())
    {
        uint8_t midiCC = (uint8_t) message.getControllerNumber();
        uint8_t value = (uint8_t) message.getControllerValue();

        auto ccComponents = this->midiCCComponents[midiCC];
        ccComponents.addArray(this->midiCCComponents[255]); // listeners to all ccs        
        for(auto *component : ccComponents)
        {
            UiHelpers::setComponentCCValue(component, midiCC, value);
        }
    }
    else if ( message.isSysEx())
    {
        std::shared_ptr<Pro800MidiMessage> pro800Message = Pro800MessageFactory::createMidiMessage(message);
        if ( auto settingsMessage = std::dynamic_pointer_cast<SettingsMessage>(pro800Message))
        {
            for(auto *component : this->settingsComponents )
            {
                component->handlePro800SettingsMessage(settingsMessage);
            }
        }
    }
}

void MidiHandler::registerMidiCCComponent(uint8_t midiCC, juce::Component *component)
{
    this->midiCCComponents.getReference(midiCC).add(component);
}

void MidiHandler::unregisterMidiCCComponent(uint8_t midiCC, juce::Component *component)
{
    this->midiCCComponents.getReference(midiCC).removeAllInstancesOf(component);
}

void MidiHandler::registerMidiLogComponent(juce::Component *component)
{
    this->logComponents.add(component);
}

void MidiHandler::unregisterMidiLogComponent(juce::Component *component)
{
    this->logComponents.removeAllInstancesOf(component);
}

void MidiHandler::registerSettingsComponent(MidiComponent *component)
{
    this->settingsComponents.add(component);
}

void MidiHandler::unregisterSettingsComponent(MidiComponent *component)
{
    this->settingsComponents.removeAllInstancesOf(component);
}

void MidiHandler::sendMidiCCMessage (uint8_t midiCC, uint8_t value)
{
    juce::MidiMessage message = juce::MidiMessage::controllerEvent (1, (int) midiCC, (int) value);  
    sendMidiMessage(message);
}

void MidiHandler::sendMidiMessage (const juce::MidiMessage& message)
{
    for(auto *component : logComponents)
    {
        UiHelpers::setComponentLogValue(component, message, "Sending message:");
    }
    this->processor->sendMidiMessage(message);
}
