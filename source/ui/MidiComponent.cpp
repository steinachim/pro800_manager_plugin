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

#include "MidiComponent.h"

#include "../midi/MidiHandler.h"
#include "../midi/SettingsMessage.h"
#include "../midi/VersionMessage.h"
#include "../midi/ProgramMessage.h"
#include "../midi/Pro800FactoryResetMessage.h"

#include "../tailoring/Pro800CCConstants.h"
#include "../tailoring/Pro800CCUtils.h"


MidiComponent::MidiComponent(MidiHandler *handler, bool registerMidiCC, const juce::Array<MessageType> messageTypes)
{
    this->midiDumpRequestThread = std::make_unique<MidiDumpRequestThread>(handler);

    this->registeredMessageTypes = messageTypes;
    this->midiHandler = handler;

    for ( auto type : this->registeredMessageTypes )
    {
        this->midiHandler->registerMessageComponent(type, this);
    }

    if ( registerMidiCC )
    {
        this->midiHandler->registerMidiCCComponent(this);
    }
}

MidiComponent::~MidiComponent()
{
    for ( auto type : this->registeredMessageTypes )
    {
        this->midiHandler->unregisterMessageComponent(type, this);
    }

    this->midiHandler->unregisterMidiCCComponent(this);
}

void MidiComponent::requestFactoryReset()
{
    this->midiHandler->sendMidiMessage(Pro800FactoryResetMessage::request());
}

void MidiComponent::sendProgram(std::shared_ptr<ProgramMessage> &message)
{
    juce::MidiMessage midiMessage(message->getRawData()->data(), (int)message->getRawDataSize());
    this->midiHandler->sendMidiMessage(midiMessage);
}

void MidiComponent::requestProgramDump()
{
    this->midiDumpRequestThread->startThread();
}

void MidiComponent::loadProgram(uint16_t programNumber)
{
    uint8_t program = programNumber % 100; // range 0-99
    uint8_t bank = (uint8_t)(programNumber / 100);
    
    this->midiHandler->sendMidiCCMessage(CC_BANK_SELECT, bank);
    this->midiHandler->sendProgramChange(program);
}

void MidiComponent::handlePro800Message(MessageType type, std::shared_ptr<Pro800MidiMessage> &message)
{
    switch(type)
    {
        case PRO800_SETTINGS_MESSAGE:
            this->currentSettings = std::dynamic_pointer_cast<SettingsMessage>(message);
            handlePro800SettingsUpdate();
            break;

        case PRO800_VERSION_MESSAGE:
            this->currentVersion = std::dynamic_pointer_cast<VersionMessage>(message);
            handlePro800VersionUpdate();
            break;

        
        case PRO800_STATUS_MESSAGE:
            // do nothing
            break;

        case PRO800_PROGRAM_MESSAGE:
        {
            std::shared_ptr<ProgramMessage> programMessage = std::dynamic_pointer_cast<ProgramMessage>(message);
            handlePro800ProgramDump(programMessage);
            break;
        }

        case MIDI_CC_MESSAGE:
        case MIDI_LOG_MESSAGE:
        case PRO800_UNKNOWN_MESSAGE:
        default:
            // should never be reached
            juce::Logger::writeToLog("[WARNING] handlePro800Message(): Unsupported / unknown message type " + juce::String((int)type));
    }
}

void MidiComponent::handleMidiCCMessage (uint8_t midiCC, uint8_t value)
{
    Pro800CCMessages ccNumber = (Pro800CCMessages)midiCC;
    int propertyValue = value;
    for (auto* component : this->registeredCCComponents.getReference (ccNumber))
    {
        int programFieldNumber = component->getProperties().getWithDefault (PROGRAM_FIELD_PROPERTY, PROGRAM_FIELD_NONE);

        if ( programFieldNumber != PROGRAM_FIELD_NONE )
        {
            propertyValue = Pro800CCUtils::programEnumValueFromCC(value, PRO800_PROGRAM_FIELDS.at((Pro800ProgramField)programFieldNumber).numValues);
        }
        
        setComponentValue(component, propertyValue, 127);
    }
}

void MidiComponent::handlePro800SettingsUpdate()
{
    // do nothing by default
}

void MidiComponent::handlePro800VersionUpdate()
{
    // do nothing by default
}

void MidiComponent::handlePro800ProgramDump(std::shared_ptr<ProgramMessage> &/*programMessage&*/)
{
    // do nothing by default
}


void MidiComponent::handleMidiLog(const juce::MidiMessage &/*message*/, const juce::String &/*logPrefix*/)
{
    // do nothing by default
}

void MidiComponent::setupMidiComponent(juce::Component *component, Pro800CCMessages midiCC, Pro800ProgramField programField, Pro800Settings settingsField)
{
    component->getProperties().set(MIDI_CC_PROPERTY, (int)midiCC);
    component->getProperties().set(PROGRAM_FIELD_PROPERTY, (int)programField);
    component->getProperties().set(SETTINGS_FIELD_PROPERTY, (int)settingsField);

    if ( midiCC == Pro800CCMessages::CC_NONE )
    {
        return;
    }

    uint8_t ccNumber = static_cast<uint8_t>(midiCC);
    this->registeredCCComponents.getReference(midiCC).add(component);
    if (juce::Slider* slider = dynamic_cast<juce::Slider*> (component))
    {
        slider->onValueChange = ([this, slider, ccNumber] {
            float sliderValue = (float) slider->getValue();
            float normalizedValue = (sliderValue - (float) slider->getMinimum()) / ((float) slider->getMaximum() - (float) slider->getMinimum());
            uint8_t midiValue = (uint8_t) (normalizedValue * 127.0f);
            midiHandler->sendMidiCCMessage (ccNumber, midiValue);
        });
    }
    else if (juce::ToggleButton* button = dynamic_cast<juce::ToggleButton*> (component))
    {
        if ( button->getRadioGroupId() )
        {
            // radio button
            button->onClick = [this, button, ccNumber]
            {
                int value = button->getProperties()[RADIO_VALUE_PROPERTY];
                int programFieldNumber = button->getProperties().getWithDefault (PROGRAM_FIELD_PROPERTY, PROGRAM_FIELD_NONE); // ensure property exists

                if ( programFieldNumber != PROGRAM_FIELD_NONE )
                {
                    value = Pro800CCUtils::ccFromProgramEnumValue(value, PRO800_PROGRAM_FIELDS.at((Pro800ProgramField)programFieldNumber).numValues);
                }

                midiHandler->sendMidiCCMessage(ccNumber, (uint8_t)value);
            };
        }
        else
        {
            button->onClick = ([this, button, ccNumber] {
                bool buttonState = button->getToggleState();
                int value = buttonState ? 1 : 0;
                
                int programFieldNumber = button->getProperties().getWithDefault (PROGRAM_FIELD_PROPERTY, PROGRAM_FIELD_NONE); // ensure property exists
                if ( programFieldNumber != PROGRAM_FIELD_NONE )
                {
                    value = Pro800CCUtils::ccFromProgramEnumValue(value, PRO800_PROGRAM_FIELDS.at((Pro800ProgramField)programFieldNumber).numValues);
                }
                midiHandler->sendMidiCCMessage (ccNumber, (uint8_t)value);
            });
        }
    }
    else if (juce::ComboBox* comboBox = dynamic_cast<juce::ComboBox*> (component))
    {
        comboBox->onChange = ([this, comboBox, ccNumber] {
            int value = comboBox->getSelectedId() - 1; // -1 because ComboBox IDs start at 1

            int programFieldNumber = comboBox->getProperties().getWithDefault (PROGRAM_FIELD_PROPERTY, PROGRAM_FIELD_NONE); // ensure property exists
            if ( programFieldNumber != PROGRAM_FIELD_NONE )
            {
                value = Pro800CCUtils::ccFromProgramEnumValue(value, PRO800_PROGRAM_FIELDS.at((Pro800ProgramField)programFieldNumber).numValues);
            }
            midiHandler->sendMidiCCMessage (ccNumber, (uint8_t)value);
        });
    }
    else
    {
        juce::Logger::writeToLog("[WARNING] setupMidiComponent() - Unknown component type");
    }
}

void MidiComponent::removeMidiComponent(juce::Component *component)
{
    for ( auto ccComponents : this->registeredCCComponents )
    {
        ccComponents.removeAllInstancesOf(component);
    }
}

std::shared_ptr<SettingsMessage> &MidiComponent::getCurrentSettings()
{
    return this->currentSettings;
}

void MidiComponent::updateSettings(Pro800Settings setting, int value)
{
    if ( !this->currentSettings )
    {
        juce::Logger::writeToLog("[WARNING] updateSettings: Load settings first!");
        return;
    }

    this->currentSettings->setValue(setting, value);
    midiHandler->sendMidiMessage(*(this->currentSettings->toMidiMessage().get()));
}

std::shared_ptr<VersionMessage> &MidiComponent::getCurrentVersion()
{
    return this->currentVersion;
}

void MidiComponent::setComponentValue (juce::Component* component, int value, int maxValue)
{
    if (juce::Slider* slider = dynamic_cast<juce::Slider*> (component))
    {
        // scale value to slider
        if ( maxValue != -1 )
        {
            value = (int)((double) value / (double)maxValue * (slider->getMaximum() - slider->getMinimum()) + slider->getMinimum());
        }

        slider->setValue (value, juce::dontSendNotification);
    }
    else if (juce::Button* button = dynamic_cast<juce::Button*> (component))
    {
        button->setToggleState (value != 0, juce::dontSendNotification);
    }
    else if (juce::ComboBox* comboBox = dynamic_cast<juce::ComboBox*> (component))
    {
        comboBox->setSelectedId ((int) value + 1, juce::dontSendNotification); // +1 because ComboBox IDs start at 1
    }
}

void MidiComponent::MidiDumpRequestThread::run()
{
    if (!midiHandler)
        return;

    // request all programs
    for (int i = 0; i < ProgramMessage::NUM_PROGRAMS; i++)
    {
        this->midiHandler->sendMidiMessage (ProgramMessage::request (i));
        juce::Thread::sleep (10);
    }
}

void MidiComponent::loadFromProgram(const std::shared_ptr<ProgramMessage> &programMessage)
{
    for (auto entry : this->registeredCCComponents)
    {
        for (auto* component : entry)
        {
            int fieldVar = component->getProperties().getWithDefault (PROGRAM_FIELD_PROPERTY, PROGRAM_FIELD_NONE);
            Pro800ProgramField field = (Pro800ProgramField) fieldVar;

            
            if (field == PROGRAM_FIELD_LFO_DEST)
            {
                int ccNumber = component->getProperties().getWithDefault (MIDI_CC_PROPERTY, CC_NONE);
                int value = programMessage->getLfoDestinationValue ((Pro800CCMessages) ccNumber);
                setComponentValue (component, value);
            }
            else if (field != PROGRAM_FIELD_NONE)
            {
                int value = programMessage->getValue ((Pro800ProgramField) field);
                setComponentValue (component, value, 65535);
            }
        }
    }
}