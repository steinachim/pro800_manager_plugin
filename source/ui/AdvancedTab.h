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

    private:
        void addMidiMessage(const juce::String &newMessage);

        juce::TextEditor textEdit_inputMidiMessage;
        juce::TextEditor textEdit_midiMessageLog;

        juce::TextButton button_sendMessage;
        juce::TextButton button_debug;

        uint8_t currentTestNum = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdvancedTab)
};
