/*
  ==============================================================================

    SettingsTab.h
    Created: 9 Sep 2025 11:12:08am
    Author:  Achim Stein

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiComponent.h"

class SettingsTab : public juce::Component, public MidiComponent
{
    public:
        SettingsTab(MidiHandler *midiHandler);
        virtual void handlePro800SettingsUpdate() override;

    
        void resized() override;

private:
    juce::TextButton button_RefreshSettings { "Refresh Settings" };
    juce::Slider spinBox_Brightness;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsTab)
};
