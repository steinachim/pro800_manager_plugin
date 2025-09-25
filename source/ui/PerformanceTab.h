/*
  ==============================================================================

    PerformanceTab.h
    Created: 9 Sep 2025 11:12:08am
    Author:  Achim Stein

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiComponent.h"

#include "TwoColumnGroupComponent.h"

class PerformanceTab : public juce::Component, public MidiComponent
{
public:
    PerformanceTab(MidiHandler *midiHandler);
    ~PerformanceTab() override;

    void resized() override;

private:
    void setupGroupLFO();
    void setupGroupVibrato();
    void setupGroupModulation();
    void setupGroupEnvelopes();
    void setupGroupPitchBend();
    void setupGroupOscillators();
    void setupGroupVelocity();
    void setupGroupAftertouch();
    void setupGroupSpread();
    void setupGroupGlide();

    TwoColumnGroupComponent group_LFO { "1 - LFO", 3 };
    juce::ComboBox combo_LFOtarget;
    juce::ComboBox combo_LFOspeed;

    TwoColumnGroupComponent group_Vibrato { "2 - Vibrato", 3 };
    juce::Slider slider_VibratoAmount { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Slider slider_VibratoSpeed { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    TwoColumnGroupComponent group_Modulation { "3 - Modulation", 3 };
    juce::ComboBox combo_ModulationWheelTarget;
    juce::ComboBox combo_ModulationWheelAmount;
    juce::Slider slider_ModulationDelay { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    TwoColumnGroupComponent group_Envelopes { "4 - Envelopes", 4 };
    juce::ComboBox combo_EnvShapeVCA;
    juce::ComboBox combo_EnvSpeedVCA;
    juce::ComboBox combo_EnvShapeVCF;
    juce::ComboBox combo_EnvSpeedVCF;

    TwoColumnGroupComponent group_PitchBend { "5 - Pitch Bend", 4 };
    juce::ComboBox combo_PitchBendTarget;
    juce::Slider slider_PitchBendRange { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    TwoColumnGroupComponent group_Oscillators { "6 - Oscillators", 4 };
    juce::ComboBox combo_OscAFreqPotMode;
    juce::ComboBox combo_OscBFreqPotMode;
    juce::ComboBox combo_OscKeyboardTracking;

    TwoColumnGroupComponent group_Velocity { "7 - Velocity", 3 };
    juce::Slider slider_VelocityAmountVCF { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Slider slider_VelocityAmountVCA { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    TwoColumnGroupComponent group_Aftertouch { "8 - Aftertouch", 3};
    juce::Slider slider_AfterTouchAmountVCA { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Slider slider_AfterTouchAmountVCF { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Slider slider_AfterTouchAmountLFO { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    TwoColumnGroupComponent group_Spread { "9 - Spread", 3 };
    juce::ToggleButton checkBox_SpreadVoiceEnable;
    juce::Slider slider_SpreadUnisonDetune { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    TwoColumnGroupComponent group_Glide { "0 - Glide", 1 };
    juce::ComboBox combo_GlideMode;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerformanceTab)
};
