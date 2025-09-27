/*
  ==============================================================================

    FrontPanel.h
    Created: 9 Sep 2025 11:11:47am
    Author:  Achim Stein

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "EqualSpacingGroupComponent.h"
#include "MidiComponent.h"

class MidiHandler;

class FrontPanelTab : public juce::Component, public MidiComponent
{
public:
    FrontPanelTab(MidiHandler *midiHandler);
    virtual ~FrontPanelTab() override;
    
    void resized() override;
    
private:
    void setupGroupOscA();
    void setupGroupOscB();
    void setupGroupPolyMod();
    void setupGroupNoise();
    void setupGroupLFO();
    void setupGroupGlide();
    void setupGroupFilter();
    void setupGroupAmplifier();
    void setupGroupMaster();

    EqualSpacingGroupComponent group_OscillatorA { "Oscillator A", 255, 1, 5 };
    EqualSpacingGroupComponent group_OscAFrequency { "Frequency", 30 };
    EqualSpacingGroupComponent group_OscASync { "Sync", 30 };
    EqualSpacingGroupComponent group_OscAShape { "Shape", 30, 3};
    EqualSpacingGroupComponent group_OscAPulseWidth { "Pulse Width", 30};
    EqualSpacingGroupComponent group_OscALevel { "Level", 30 };
    juce::Slider slider_OscAFrequency;
    juce::Slider slider_OscAPulseWidth;
    juce::Slider slider_OscALevel;
    juce::ToggleButton checkBox_OscASync;
    juce::ToggleButton checkBox_OscAShapeSaw;
    juce::ToggleButton checkBox_OscAShapeTri;
    juce::ToggleButton checkBox_OscAShapeRect;


    EqualSpacingGroupComponent group_OscillatorB { "Oscillator B", 255, 1, 5 };
    EqualSpacingGroupComponent group_OscBFrequency { "Frequency", 30 };
    EqualSpacingGroupComponent group_OscBFine { "Fine", 30};
    EqualSpacingGroupComponent group_OscBShape { "Shape", 30, 3};
    EqualSpacingGroupComponent group_OscBPulseWidth { "Pulse Width", 30};
    EqualSpacingGroupComponent group_OscBLevel { "Level", 30 };
    juce::Slider slider_OscBFrequency;
    juce::Slider slider_OscBFine;
    juce::Slider slider_OscBPulseWidth;
    juce::Slider slider_OscBLevel;
    juce::ToggleButton checkBox_OscBShapeSaw;
    juce::ToggleButton checkBox_OscBShapeTri;
    juce::ToggleButton checkBox_OscBShapeRect;


    EqualSpacingGroupComponent group_PolyMod {"Poly Mod", 255, 1, 4};
    EqualSpacingGroupComponent group_PolyModSourceAmount { "Source Amount", 30, 1, 2};
    EqualSpacingGroupComponent group_PolyModDestination { "Destination", 30, 2};
    EqualSpacingGroupComponent group_PolyModUnisonTrack { "Unison Track", 30};
    EqualSpacingGroupComponent group_PolyModSourceFilterEnv { "Filter Env", 0 };
    EqualSpacingGroupComponent group_PolyModSourceOscB { "OSC B", 0 };
    juce::Slider slider_PolyModSourceFilterEnv;
    juce::Slider slider_PolyModSourceOscB;
    juce::ToggleButton checkBox_PolyModDestFreqA;
    juce::ToggleButton checkBox_PolyModDestFilter;
    juce::ToggleButton checkBox_PolyModUnisonTrack;


    EqualSpacingGroupComponent group_Noise {"Noise"};
    EqualSpacingGroupComponent group_NoiseLevel { "Noise Level", 30 };
    juce::Slider slider_NoiseLevel;


    EqualSpacingGroupComponent group_LFO {"LFO Mod", 255, 1, 4};
    EqualSpacingGroupComponent group_LFOFrequency { "Frequency", 30 };
    EqualSpacingGroupComponent group_LFOShape { "Shape", 30, 4 };
    EqualSpacingGroupComponent group_LFOInitialAmount { "Initial Amount", 30 };
    EqualSpacingGroupComponent group_LFODestination { "Destination", 30, 3 };
    juce::Slider slider_LFOFrequency;
    juce::ComboBox comboBox_LFOShape;
    juce::Slider slider_LFOInitialAmount;
    juce::ToggleButton checkBox_LFODestFreqAB;
    juce::ToggleButton checkBox_LFODestPulseWidthAB;
    juce::ToggleButton checkBox_LFODestFilter;


    EqualSpacingGroupComponent group_Glide {"Glide"};
    EqualSpacingGroupComponent group_GlideAmount { "Glide Amount", 30 };
    juce::Slider slider_GlideAmount;

    
    EqualSpacingGroupComponent group_Filter {"Filter", 255, 2, 4};
    EqualSpacingGroupComponent group_FilterCutoff { "Cutoff", 30 };
    EqualSpacingGroupComponent group_FilterResonance { "Resonance", 30 };
    EqualSpacingGroupComponent group_FilterEnvAmount { "Env Amount", 30 };
    EqualSpacingGroupComponent group_FilterKeyboardTracking { "Keyboard Tracking", 30, 3 };
    juce::Slider slider_FilterCutoff;
    juce::Slider slider_FilterResonance;
    juce::Slider slider_FilterEnvAmount;
    juce::ToggleButton radio_FilterTrackingFull;
    juce::ToggleButton radio_FilterTrackingHalf;
    juce::ToggleButton radio_FilterTrackingOff;
    EqualSpacingGroupComponent group_FilterAttack { "Attack", 30 };
    EqualSpacingGroupComponent group_FilterDecay { "Decay", 30 };
    EqualSpacingGroupComponent group_FilterSustain { "Sustain", 30 };
    EqualSpacingGroupComponent group_FilterRelease { "Release", 30 };
    juce::Slider slider_FilterAttack;
    juce::Slider slider_FilterDecay;
    juce::Slider slider_FilterSustain;
    juce::Slider slider_FilterRelease;

    
    EqualSpacingGroupComponent group_Amplifier {"Amplifier", 255, 1, 4};
    EqualSpacingGroupComponent group_AmplifierAttack { "Attack", 30 };
    EqualSpacingGroupComponent group_AmplifierDecay { "Decay", 30 };
    EqualSpacingGroupComponent group_AmplifierSustain { "Sustain", 30 };
    EqualSpacingGroupComponent group_AmplifierRelease { "Release", 30 };
    juce::Slider slider_AmplifierAttack;
    juce::Slider slider_AmplifierDecay;
    juce::Slider slider_AmplifierSustain;
    juce::Slider slider_AmplifierRelease;

    
    EqualSpacingGroupComponent group_Master { "Master", 255, 1, 2 };
    EqualSpacingGroupComponent group_MasterTune { "Master Tune", 30 };
    EqualSpacingGroupComponent group_MasterVolume { "Master Volume", 30 };
    juce::Slider slider_MasterTune;
    juce::Slider slider_MasterVolume;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrontPanelTab)
};

