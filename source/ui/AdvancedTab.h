/*
  ==============================================================================

    AdvancedTab.h
    Created: 9 Sep 2025 11:12:08am
    Author:  Achim Stein

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiComponent.h"

class MidiHandler;

class AdvancedTab : public juce::Component, public MidiComponent
{
    public:
        AdvancedTab(MidiHandler *parent);
        virtual ~AdvancedTab() override;

        void resized() override;
        void handleMidiLog(const juce::MidiMessage &message, const juce::String &logPrefix) override;

    private:
        void addLogMessage(const juce::String &newMessage);

        juce::TextEditor textEdit_inputMidiMessage;
        juce::TextEditor textEdit_midiMessageLog;

        juce::TextButton button_sendMessage;
        juce::TextButton button_debug;

        juce::ToggleButton checkBox_enableLogging { "Enable Logging" };

        juce::Slider slider_debugInput { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdvancedTab)
};
