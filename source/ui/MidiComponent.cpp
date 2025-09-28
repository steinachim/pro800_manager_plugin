#include "MidiComponent.h"

#include "UiHelpers.h"

#include "../midi/MidiHandler.h"
#include "../midi/SettingsMessage.h"
#include "../midi/VersionMessage.h"
#include "../midi/Pro800FactoryResetMessage.h"


MidiComponent::MidiComponent(MidiHandler *handler, bool registerMidiCC, const juce::Array<MessageType> messageTypes)
{
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

        case PRO800_UNKNOWN_MESSAGE:
        case PRO800_PROGRAM_MESSAGE:
        default:
            std::cerr << "[WARNING] handlePro800Message(): Unsupported / unknown message type" << type << std::endl;
            // do nothing
    }
}

void MidiComponent::handleMidiCCMessage(uint8_t midiCC, uint8_t value)
{
    for( auto *component : this->registeredCCComponents.getReference(midiCC) )
    {
        UiHelpers::setComponentCCValue(component, midiCC, value);
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

void MidiComponent::handleMidiLog(const juce::MidiMessage &/*message*/, const juce::String &/*logPrefix*/)
{
    // do nothing by default
}

void MidiComponent::setupMidiCCComponent(uint8_t midiCC, juce::Component *component)
{
    this->registeredCCComponents.getReference(midiCC).add(component);

    if (juce::Slider* slider = dynamic_cast<juce::Slider*> (component))
    {
        slider->onValueChange = ([this, slider, midiCC] {
            float sliderValue = (float) slider->getValue();
            float normalizedValue = (sliderValue - (float) slider->getMinimum()) / ((float) slider->getMaximum() - (float) slider->getMinimum());
            uint8_t midiValue = (uint8_t) (normalizedValue * 127.0f);
            midiHandler->sendMidiCCMessage (midiCC, midiValue);
        });
    }
    else if (juce::Button* button = dynamic_cast<juce::Button*> (component))
    {
        button->onClick = ([this, button, midiCC] {
            bool buttonState = button->getToggleState();
            uint8_t midiValue = buttonState ? 127 : 0;
            midiHandler->sendMidiCCMessage (midiCC, midiValue);
        });
    }
    else if (juce::ComboBox* comboBox = dynamic_cast<juce::ComboBox*> (component))
    {
        comboBox->onChange = ([this, comboBox, midiCC] {
            int selectedId = comboBox->getSelectedId();
            uint8_t midiValue = (uint8_t) juce::jlimit (0, 127, selectedId - 1); // -1 because ComboBox IDs start at 1
            midiHandler->sendMidiCCMessage (midiCC, midiValue);
        });
    }
    else
    {
      std::cerr << "[WARNING] setupMidiCCComponent() - Unknown component type" << std::endl;
    }
}

void MidiComponent::removeMidiCCComponent(uint8_t midiCC, juce::Component *component)
{
    this->registeredCCComponents.getReference(midiCC).removeAllInstancesOf(component);
}

std::shared_ptr<SettingsMessage> &MidiComponent::getCurrentSettings()
{
    return this->currentSettings;
}

void MidiComponent::updateSettings(Pro800Settings setting, int value)
{
    if ( !this->currentSettings )
    {
        std::cerr << "[WARNING] updateSettings: Load settings first!" << std::endl;
        return;
    }

    this->currentSettings->setValue(setting, value);
    midiHandler->sendMidiMessage(*(this->currentSettings->toMidiMessage().get()));
}

std::shared_ptr<VersionMessage> &MidiComponent::getCurrentVersion()
{
    return this->currentVersion;
}