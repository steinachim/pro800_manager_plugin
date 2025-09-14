/*
  ==============================================================================

    SettingsTab.cpp
    Created: 9 Sep 2025 11:12:00am
    Author:  Achim Stein

  ==============================================================================
*/

#include "SettingsTab.h"

#include "../midi/MidiHandler.h"
#include "../midi/SettingsMessage.h"

SettingsTab::SettingsTab(MidiHandler *midiHandler) : Component(), MidiComponent(midiHandler, true)
{
    button_RefreshSettings.onClick = [midiHandler] {
        midiHandler->sendMidiMessage(SettingsMessage::request());
    };

    spinBox_Brightness.setRange(0.0, 16.0, 1.0);
    spinBox_Brightness.setSliderStyle(juce::Slider::SliderStyle::IncDecButtons);
    spinBox_Brightness.onValueChange = ([this] { updateSettings(Pro800Settings::SETTINGS_BRIGHTNESS, (int)spinBox_Brightness.getValue()); });

    addAndMakeVisible(spinBox_Brightness);
    addAndMakeVisible(button_RefreshSettings);
}

void SettingsTab::handlePro800SettingsUpdate()
{
  std::shared_ptr<SettingsMessage> settingsMessage = getCurrentSettings();
  if(!settingsMessage)
  {
    return;
  }

  spinBox_Brightness.setValue( settingsMessage->getValue(Pro800Settings::SETTINGS_BRIGHTNESS), juce::NotificationType::dontSendNotification );

}

void SettingsTab::resized()
{
    button_RefreshSettings.setBounds(10, 10, getWidth() - 20, 30);
    spinBox_Brightness.setBounds(10, 50, 120, 30);
}
