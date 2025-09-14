/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "ui/MainWidget.h"

class MidiHandler;

//==============================================================================
/**
*/
class Pro800ManagerEditor  : public juce::AudioProcessorEditor
{
public:
    Pro800ManagerEditor (MidiHandler *midiHandler, Pro800ManagerAudioProcessor&);
    ~Pro800ManagerEditor() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Pro800ManagerAudioProcessor& audioProcessor;
    
    MainWidget *tabBar;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pro800ManagerEditor)
};
