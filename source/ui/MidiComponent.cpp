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
#include "../midi/Pro800FactoryResetMessage.h"
#include "../midi/ProgramMessage.h"
#include "../midi/SettingsMessage.h"
#include "../midi/VersionMessage.h"

#include "../tailoring/Pro800CCConstants.h"
#include "../tailoring/Pro800CCUtils.h"

MidiComponent::MidiComponent (MidiHandler* handler, bool registerMidiCC, const juce::Array<MessageType> messageTypes)
{
    this->registeredMessageTypes = messageTypes;
    this->midiHandler = handler;

    for (auto type : this->registeredMessageTypes)
    {
        this->midiHandler->registerMessageComponent (type, this);
    }

    if (registerMidiCC)
    {
        this->midiHandler->registerMidiCCComponent (this);
    }
}

MidiComponent::~MidiComponent()
{
    for (auto type : this->registeredMessageTypes)
    {
        this->midiHandler->unregisterMessageComponent (type, this);
    }

    this->midiHandler->unregisterMidiCCComponent (this);
}

void MidiComponent::requestFactoryReset()
{
    this->midiHandler->sendMidiMessage (Pro800FactoryResetMessage::request());
}

void MidiComponent::sendPrograms (const std::vector<std::shared_ptr<ProgramMessage>>& programs)
{
    std::vector<juce::MidiMessage> messages;
    messages.reserve (programs.size());

    for (const auto& program : programs)
    {
        messages.push_back (program->toMidiMessage());
    }

    this->midiHandler->sendMidiMessagesInBackground (std::move (messages), PROGRAM_SEND_INTERVAL_MS, "Sending program");
}

MidiHandler& MidiComponent::getMidiHandler() const
{
    return *this->midiHandler;
}

Pro800ProgramField MidiComponent::getProgramField (const juce::Component& component)
{
    const int stored = component.getProperties().getWithDefault (PROGRAM_FIELD_PROPERTY, static_cast<int> (Pro800ProgramField::NONE));
    return static_cast<Pro800ProgramField> (stored);
}

Pro800CCMessages MidiComponent::getMidiCC (const juce::Component& component)
{
    const int stored = component.getProperties().getWithDefault (MIDI_CC_PROPERTY, static_cast<int> (Pro800CCMessages::NONE));
    return static_cast<Pro800CCMessages> (stored);
}

void MidiComponent::addEnumItems (juce::ComboBox& comboBox, const std::vector<EnumItem>& items)
{
    for (const auto& item : items)
    {
        comboBox.addItem (item.name, item.value + COMBO_BOX_ID_OFFSET);
    }
}

void MidiComponent::sendMidiMessage (const juce::MidiMessage& message)
{
    this->midiHandler->sendMidiMessage (message);
}

void MidiComponent::requestProgramDump()
{
    this->midiHandler->requestProgramDump();
}

void MidiComponent::loadProgram (const ProgramMessage& program)
{
    this->midiHandler->loadProgram (program);
}

void MidiComponent::handlePro800Message (MessageType type, std::shared_ptr<Pro800MidiMessage>& message)
{
    switch (type)
    {
        case MessageType::PRO800_SETTINGS:
            this->currentSettings = std::dynamic_pointer_cast<SettingsMessage> (message);
            handlePro800SettingsUpdate();
            break;

        case MessageType::PRO800_VERSION:
            this->currentVersion = std::dynamic_pointer_cast<VersionMessage> (message);
            handlePro800VersionUpdate();
            break;

        case MessageType::PRO800_STATUS:
            // do nothing
            break;

        case MessageType::PRO800_PROGRAM:
        {
            std::shared_ptr<ProgramMessage> programMessage = std::dynamic_pointer_cast<ProgramMessage> (message);
            handlePro800ProgramDump (programMessage);
            break;
        }

        case MessageType::MIDI_LOG:
        case MessageType::PRO800_UNKNOWN:
        default:
            // should never be reached
            juce::Logger::writeToLog ("[WARNING] handlePro800Message(): Unsupported / unknown message type " + juce::String (static_cast<int> (type)));
    }
}

void MidiComponent::handleMidiCCMessage (Pro800CCMessages midiCC, uint8_t value)
{
    const auto entry = this->registeredCCComponents.find (midiCC);
    if (entry == this->registeredCCComponents.end())
    {
        return;
    }

    for (auto* component : entry->second)
    {
        const Pro800ProgramField programField = getProgramField (*component);

        if (programField == Pro800ProgramField::PITCHBEND_RANGE)
        {
            // pitchbend range is in increments of 4 (0 - 124)
            setComponentValue (component, value, 124);
            continue;
        }

        // every component starts from the raw CC value; only enum fields need it mapped
        int componentValue = value;
        if (programField != Pro800ProgramField::NONE)
        {
            componentValue = Pro800CCUtils::programEnumValueFromCC (value, PRO800_PROGRAM_FIELDS.at (programField).numValues);
        }

        setComponentValue (component, componentValue, 127);
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

void MidiComponent::handlePro800ProgramDump (std::shared_ptr<ProgramMessage>& /*programMessage&*/)
{
    // do nothing by default
}

void MidiComponent::handleMidiLog (const juce::MidiMessage& /*message*/, const juce::String& /*logPrefix*/)
{
    // do nothing by default
}

void MidiComponent::setupMidiComponent (juce::Component* component, Pro800CCMessages midiCC, Pro800ProgramField programField, Pro800Settings settingsField)
{
    // enums are stored as ints in the property set (juce::var has no enum type); see getProgramField()/getMidiCC()
    component->getProperties().set (MIDI_CC_PROPERTY, static_cast<int> (midiCC));
    component->getProperties().set (PROGRAM_FIELD_PROPERTY, static_cast<int> (programField));
    component->getProperties().set (SETTINGS_FIELD_PROPERTY, static_cast<int> (settingsField));

    if (midiCC == Pro800CCMessages::NONE)
    {
        return;
    }

    this->registeredCCComponents[midiCC].add (component);
    if (juce::Slider* slider = dynamic_cast<juce::Slider*> (component))
    {
        slider->onValueChange = ([this, slider, midiCC] {
            double sliderValue = slider->getValue();
            uint8_t midiValue = 0;

            if (getProgramField (*slider) == Pro800ProgramField::PITCHBEND_RANGE)
            {
                // pitchbend range is in increments of 4 (0 - 124)
                midiValue = (uint8_t) (sliderValue * 4);
            }
            else
            {
                double normalizedValue = (sliderValue - slider->getMinimum()) / (slider->getMaximum() - slider->getMinimum());
                midiValue = (uint8_t) (normalizedValue * 127.0f);
            }
            midiHandler->sendMidiCCMessage (midiCC, midiValue);
        });
    }
    else if (juce::ToggleButton* button = dynamic_cast<juce::ToggleButton*> (component))
    {
        if (button->getRadioGroupId())
        {
            // radio button
            button->onClick = [this, button, midiCC] {
                int value = button->getProperties()[RADIO_VALUE_PROPERTY];
                const Pro800ProgramField linkedField = getProgramField (*button);

                if (linkedField != Pro800ProgramField::NONE)
                {
                    value = Pro800CCUtils::ccFromProgramEnumValue (value, PRO800_PROGRAM_FIELDS.at (linkedField).numValues);
                }

                midiHandler->sendMidiCCMessage (midiCC, (uint8_t) value);
            };
        }
        else
        {
            button->onClick = ([this, button, midiCC] {
                bool buttonState = button->getToggleState();
                int value = buttonState ? CC_ON : CC_OFF;

                midiHandler->sendMidiCCMessage (midiCC, (uint8_t) value);
            });
        }
    }
    else if (juce::ComboBox* comboBox = dynamic_cast<juce::ComboBox*> (component))
    {
        comboBox->onChange = ([this, comboBox, midiCC] {
            int value = comboBox->getSelectedId() - COMBO_BOX_ID_OFFSET;

            const Pro800ProgramField linkedField = getProgramField (*comboBox);
            if (linkedField != Pro800ProgramField::NONE)
            {
                value = Pro800CCUtils::ccFromProgramEnumValue (value, PRO800_PROGRAM_FIELDS.at (linkedField).numValues);
            }

            midiHandler->sendMidiCCMessage (midiCC, (uint8_t) value);
        });
    }
    else
    {
        juce::Logger::writeToLog ("[WARNING] setupMidiComponent() - Unknown component type");
    }
}

std::shared_ptr<SettingsMessage>& MidiComponent::getCurrentSettings()
{
    return this->currentSettings;
}

void MidiComponent::updateSettings (Pro800Settings setting, int value)
{
    if (!this->currentSettings)
    {
        juce::Logger::writeToLog ("[WARNING] updateSettings: Load settings first!");
        return;
    }

    this->currentSettings->setValue (setting, value);
    midiHandler->sendMidiMessage (this->currentSettings->toMidiMessage());
}

std::shared_ptr<VersionMessage>& MidiComponent::getCurrentVersion()
{
    return this->currentVersion;
}

void MidiComponent::setComponentValue (juce::Component* component, int value, int maxValue)
{
    if (juce::Slider* slider = dynamic_cast<juce::Slider*> (component))
    {
        // scale value to slider
        if (maxValue != -1)
        {
            value = (int) ((double) value / (double) maxValue * (slider->getMaximum() - slider->getMinimum()) + slider->getMinimum());
        }

        slider->setValue (value, juce::dontSendNotification);
    }
    else if (juce::Button* button = dynamic_cast<juce::Button*> (component))
    {
        button->setToggleState (value != 0, juce::dontSendNotification);
    }
    else if (juce::ComboBox* comboBox = dynamic_cast<juce::ComboBox*> (component))
    {
        comboBox->setSelectedId (value + COMBO_BOX_ID_OFFSET, juce::dontSendNotification);
    }
}

void MidiComponent::loadFromProgram (const ProgramMessage& program)
{
    for (const auto& [midiCC, components] : this->registeredCCComponents)
    {
        for (auto* component : components)
        {
            const Pro800ProgramField field = getProgramField (*component);

            if (field == Pro800ProgramField::LFO_DEST)
            {
                int value = program.getLfoDestinationValue (getMidiCC (*component));
                setComponentValue (component, value);
            }
            else if (field != Pro800ProgramField::NONE)
            {
                int value = program.getValue (field);
                setComponentValue (component, value, 65535);
            }
        }
    }
}