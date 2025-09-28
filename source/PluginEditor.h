/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "juce_audio_utils/juce_audio_utils.h"

#include "PluginProcessor.h"
#include "ui/MainWidget.h"

class MidiHandler;

//==============================================================================
/**
*/
class Pro800ManagerEditor  : public juce::AudioProcessorEditor, public juce::MidiKeyboardState::Listener
{
public:
    Pro800ManagerEditor (MidiHandler *midiHandler, Pro800ManagerAudioProcessor&);
    ~Pro800ManagerEditor() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Pro800ManagerAudioProcessor& audioProcessor;
    
    juce::MidiKeyboardState keyboardState;

    MainWidget *tabBar;
    juce::MidiKeyboardComponent keyboardPanel { keyboardState, juce::MidiKeyboardComponent::Orientation::horizontalKeyboard };
    juce::TextButton button_ShowHideKeyboard { "Hide Keyboard" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pro800ManagerEditor)
};
