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

#include "EqualSpacingGroupComponent.h"

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

    EqualSpacingGroupComponent group_LFO { "1 - LFO", 255, 3, 2 };
    juce::Label label_LFOtarget { "", "Target" };
    juce::ComboBox combo_LFOtarget;
    juce::Label label_LFOspeed { "", "Speed" };
    juce::ComboBox combo_LFOspeed;

    EqualSpacingGroupComponent group_Vibrato { "2 - Vibrato", 255, 3, 2 };
    juce::Label label_VibratoAmount { "", "Target" };
    juce::Slider slider_VibratoAmount { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Label label_VibratoSpeed { "", "Target" };
    juce::Slider slider_VibratoSpeed { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    EqualSpacingGroupComponent group_Modulation { "3 - Modulation", 255, 3, 2 };
    juce::Label label_ModulationWheelTarget { "", "Wheel Amount" };
    juce::ComboBox combo_ModulationWheelTarget;
    juce::Label label_ModulationWheelAmount { "", "Wheel Amount" };
    juce::ComboBox combo_ModulationWheelAmount;
    juce::Label label_ModulationDelay { "", "Modulation Delay" };
    juce::Slider slider_ModulationDelay { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    EqualSpacingGroupComponent group_Envelopes { "4 - Envelopes", 255, 4, 2 };
    juce::Label label_EnvShapeVCA { "", "VCA Envelope Shape" };
    juce::ComboBox combo_EnvShapeVCA;
    juce::Label label_EnvSpeedVCA { "", "VCA Envelope Speed" };
    juce::ComboBox combo_EnvSpeedVCA;
    juce::Label label_EnvShapeVCF { "", "VCF Envelope Shape" };
    juce::ComboBox combo_EnvShapeVCF;
    juce::Label label_EnvSpeedVCF { "", "VCF Envelope Speed" };
    juce::ComboBox combo_EnvSpeedVCF;

    EqualSpacingGroupComponent group_PitchBend { "5 - Pitch Bend", 255, 4, 2 };
    juce::Label label_PitchBendTarget { "", "Target" };
    juce::ComboBox combo_PitchBendTarget;
    juce::Label label_PitchBendRange { "", "Range" };
    juce::Slider slider_PitchBendRange { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    EqualSpacingGroupComponent group_Oscillators { "6 - Oscillators", 255, 4, 2 };
    juce::Label label_OscAFreqPotMode = { "", "Osc A Freq Pot Mode"};
    juce::ComboBox combo_OscAFreqPotMode;
    juce::Label label_OscBFreqPotMode = { "", "Osc B Freq Pot Mode"};
    juce::ComboBox combo_OscBFreqPotMode;
    juce::Label label_OscKeyboardTracking = { "", "Keyboard Tracking"};
    juce::ComboBox combo_OscKeyboardTracking;

    EqualSpacingGroupComponent group_Velocity { "7 - Velocity", 255, 3, 2 };
    juce::Label label_VelocityAmountVCF = { "", "VCF Velocity Amount"};
    juce::Slider slider_VelocityAmountVCF { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Label label_VelocityAmountVCA = { "", "VCA Velocity Amount"};
    juce::Slider slider_VelocityAmountVCA { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    EqualSpacingGroupComponent group_Aftertouch { "8 - Aftertouch", 255, 3, 2};
    juce::Label label_AfterTouchAmountVCA = { "", "VCA Velocity Amount"};
    juce::Slider slider_AfterTouchAmountVCA { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Label label_AfterTouchAmountVCF = { "", "VCF Velocity Amount"};
    juce::Slider slider_AfterTouchAmountVCF { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Label label_AfterTouchAmountLFO = { "", "LFO Velocity Amount"};
    juce::Slider slider_AfterTouchAmountLFO { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    EqualSpacingGroupComponent group_Spread { "9 - Spread", 255, 3, 2 };
    juce::Label label_SpreadVoiceEnable {"", "Voice Spread"};
    juce::ToggleButton checkBox_SpreadVoiceEnable;
    juce::Label label_SpreadUnisonDetune {"", "Unison Spread Detune"};
    juce::Slider slider_SpreadUnisonDetune { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    EqualSpacingGroupComponent group_Glide { "0 - Glide", 255, 3, 2 };
    juce::Label label_GlideMode { "", "Glide Mode" };
    juce::ComboBox combo_GlideMode;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerformanceTab)
};
