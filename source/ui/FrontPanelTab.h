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

class GroupOscillatorA : public EqualSpacingGroupComponent, public MidiComponent
{
public:
    GroupOscillatorA(MidiHandler *midiHandler);
    virtual ~GroupOscillatorA() override;
    
private:
    EqualSpacingGroupComponent group_Frequency { "Frequency", 30 };
    EqualSpacingGroupComponent group_PulseWidth { "Pulse Width", 30};
    EqualSpacingGroupComponent group_Level { "Level", 30 };
    juce::Slider slider_Frequency;
    juce::Slider slider_PulseWidth;
    juce::Slider slider_Level;
    
    EqualSpacingGroupComponent group_Sync { "Sync", 30 };
    juce::ToggleButton checkBox_Sync;
    
    EqualSpacingGroupComponent group_Shape { "Shape", 30, juce::FlexBox::Direction::column};
    juce::ToggleButton checkBox_ShapeSaw;
    juce::ToggleButton checkBox_ShapeTri;
    juce::ToggleButton checkBox_ShapeRect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroupOscillatorA)

};

class GroupOscillatorB : public EqualSpacingGroupComponent, public MidiComponent
{
public:
    GroupOscillatorB(MidiHandler *midiHandler);
    virtual ~GroupOscillatorB() override;

private:
    EqualSpacingGroupComponent group_Frequency { "Frequency", 30 };
    EqualSpacingGroupComponent group_Fine { "Fine", 30};
    EqualSpacingGroupComponent group_PulseWidth { "Pulse Width", 30};
    EqualSpacingGroupComponent group_Level { "Level", 30 };
    juce::Slider slider_Frequency;
    juce::Slider slider_Fine;
    juce::Slider slider_PulseWidth;
    juce::Slider slider_Level;
    
    EqualSpacingGroupComponent group_Shape { "Shape", 30, juce::FlexBox::Direction::column};
    juce::ToggleButton checkBox_ShapeSaw;
    juce::ToggleButton checkBox_ShapeTri;
    juce::ToggleButton checkBox_ShapeRect;
        
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroupOscillatorB)
};

class GroupPolyMod : public EqualSpacingGroupComponent, public MidiComponent
{
public:
    GroupPolyMod(MidiHandler *midiHandler);
    virtual ~GroupPolyMod() override;
    
private:
    EqualSpacingGroupComponent group_SourceAmount { "Source Amount", 30 };
    EqualSpacingGroupComponent group_Destination { "Destination", 30, juce::FlexBox::Direction::column };
    EqualSpacingGroupComponent group_UnisonTrack { "Unison Track", 30, juce::FlexBox::Direction::column };
    
    EqualSpacingGroupComponent group_SourceFilterEnv { "Filter Env", 0 };
    EqualSpacingGroupComponent group_SourceOscB { "OSC B", 0 };
    juce::Slider slider_SourceFilterEnv;
    juce::Slider slider_SourceOscB;
    
    juce::ToggleButton checkBox_DestFreqA;
    juce::ToggleButton checkBox_DestFilter;
    
    juce::ToggleButton checkBox_UnisonTrack;
};

class GroupNoise : public EqualSpacingGroupComponent, public MidiComponent
{
public:
    GroupNoise(MidiHandler *midiHandler);
    virtual ~GroupNoise() override;

private:
    EqualSpacingGroupComponent group_NoiseLevel { "Noise Level", 30 };
    juce::Slider slider_NoiseLevel;
};

class GroupLfoMod : public EqualSpacingGroupComponent, public MidiComponent
{
public:
    GroupLfoMod(MidiHandler *midiHandler);
    virtual ~GroupLfoMod() override;
    
private:
    EqualSpacingGroupComponent group_Frequency { "Frequency", 30 };
    EqualSpacingGroupComponent group_Shape { "Shape", 30 };
    EqualSpacingGroupComponent group_InitialAmount { "Initial Amount", 30 };
    EqualSpacingGroupComponent group_Destination { "Destination", 30, juce::FlexBox::Direction::column };
    
    juce::Slider slider_Frequency;
    juce::ComboBox comboBox_Shape;
    juce::Slider slider_InitialAmount;
    
    juce::ToggleButton checkBox_DestFreqAB;
    juce::ToggleButton checkBox_DestPulseWidthAB;
    juce::ToggleButton checkBox_DestFilter;
};

class GroupGlide : public EqualSpacingGroupComponent, public MidiComponent
{
public:
    GroupGlide(MidiHandler *midiHandler);
    virtual ~GroupGlide() override;
    
private:
    EqualSpacingGroupComponent group_GlideAmount { "Glide Amount", 30 };
    juce::Slider slider_GlideAmount;
};


class GroupAmplifier : public EqualSpacingGroupComponent, public MidiComponent
{
public:
    GroupAmplifier(MidiHandler *midiHandler);
    virtual ~GroupAmplifier() override;
    
private:
    EqualSpacingGroupComponent group_Attack { "Attack", 30 };
    EqualSpacingGroupComponent group_Decay { "Decay", 30 };
    EqualSpacingGroupComponent group_Sustain { "Sustain", 30 };
    EqualSpacingGroupComponent group_Release { "Release", 30 };

    juce::Slider slider_Attack;
    juce::Slider slider_Decay;
    juce::Slider slider_Sustain;
    juce::Slider slider_Release;

};

class GroupMaster : public EqualSpacingGroupComponent, public MidiComponent
{
public:
    GroupMaster(MidiHandler *midiHandler);
    virtual ~GroupMaster() override;
    
private:
    EqualSpacingGroupComponent group_MasterTune { "Master Tune", 30 };
    EqualSpacingGroupComponent group_MasterVolume { "Master Volume", 30 };

    juce::Slider slider_MasterTune;
    juce::Slider slider_MasterVolume;

};

class GroupFilter : public juce::GroupComponent, public MidiComponent
{
public:
    GroupFilter(MidiHandler *midiHandler);
    virtual ~GroupFilter() override;

    void resized() override;
    
private:
    EqualSpacingGroupComponent group_Cutoff { "Cutoff", 30 };
    EqualSpacingGroupComponent group_Resonance { "Resonance", 30 };
    EqualSpacingGroupComponent group_EnvAmount { "Env Amount", 30 };
    EqualSpacingGroupComponent group_KeyboardTracking { "Keyboard Tracking", 30, juce::FlexBox::Direction::column };
    juce::Slider slider_Cutoff;
    juce::Slider slider_Resonance;
    juce::Slider slider_EnvAmount;
    juce::ToggleButton radio_TrackingFull;
    juce::ToggleButton radio_TrackingHalf;
    juce::ToggleButton radio_TrackingOff;
    
    EqualSpacingGroupComponent group_Attack { "Attack", 30 };
    EqualSpacingGroupComponent group_Decay { "Decay", 30 };
    EqualSpacingGroupComponent group_Sustain { "Sustain", 30 };
    EqualSpacingGroupComponent group_Release { "Release", 30 };
    juce::Slider slider_Attack;
    juce::Slider slider_Decay;
    juce::Slider slider_Sustain;
    juce::Slider slider_Release;
};



class FrontPanelTab : public juce::Component
{
public:
    FrontPanelTab(MidiHandler *midiHandler);
    virtual ~FrontPanelTab() override;
    
    void resized() override;
    
private:
    GroupOscillatorA *group_OscillatorA;
    GroupOscillatorB *group_OscillatorB;
    GroupPolyMod *group_PolyMod;
    GroupNoise *group_Noise;
    GroupLfoMod *group_LfoMod;
    GroupGlide *group_Glide;
    
    GroupFilter *group_Filter;
    GroupAmplifier *group_Amplifier;
    GroupMaster *group_Master;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrontPanelTab)
};

