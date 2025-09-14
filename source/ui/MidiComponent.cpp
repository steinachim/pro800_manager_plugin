#include "MidiComponent.h"

#include "../midi/MidiHandler.h"
#include "../midi/SettingsMessage.h"

MidiComponent::MidiComponent(MidiHandler *handler, bool registerSettingsMessage)
{
    this->registerSettings = registerSettingsMessage;
    this->midiHandler = handler;

    if ( registerSettings )
    {
        this->midiHandler->registerSettingsComponent(this);
    }
}

MidiComponent::~MidiComponent()
{

    if( registerSettings )
    {
        this->midiHandler->unregisterSettingsComponent(this);
    }
}

void MidiComponent::handlePro800SettingsMessage(std::shared_ptr<SettingsMessage> &settingsMessage)
{
    this->currentSettings = settingsMessage;
    handlePro800SettingsUpdate();
}

void MidiComponent::handlePro800SettingsUpdate()
{
    // do nothing by default
}

void MidiComponent::setupMidiCCComponent(uint8_t midiCC, juce::Component *component)
{
  midiHandler->registerMidiCCComponent(midiCC, component);

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
    midiHandler->unregisterMidiCCComponent(midiCC, component);
}

void MidiComponent::setupMidiLogComponent(juce::Component *component)
{
    midiHandler->registerMidiLogComponent(component);
}

void MidiComponent::removeMidiLogComponent(juce::Component *component)
{
    midiHandler->unregisterMidiLogComponent(component);
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