/** 
 * Pro800 Manager Plugin
 * Copyright (C) 2025 Achim Stein
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

#include "PerformanceTab.h"
#include "../midi/ProgramMessage.h" 
#include "../constants/Pro800CCConstants.h"
#include "../constants/Pro800ProgramConstants.h"

PerformanceTab::PerformanceTab (MidiHandler* midiHandler) : Component(), MidiComponent (midiHandler, true)
{
    setupGroupLFO();
    setupGroupVibrato();
    setupGroupModulation();
    setupGroupEnvelopes();
    setupGroupPitchBend();
    setupGroupOscillators();
    setupGroupVelocity();
    setupGroupAftertouch();
    setupGroupSpread();
    setupGroupGlide();
}

PerformanceTab::~PerformanceTab()
{
}

void PerformanceTab::resized()
{
    const int numVerticalElements = 17; // number of elements *including* groups

    auto area = getLocalBounds().reduced (10);
    int elementHeight = area.getHeight() / numVerticalElements;
    int groupWidth = area.getWidth() / 3;

    auto leftColumn = area.withRight (groupWidth);
    auto middleColumn = area.withLeft (groupWidth).withRight (2 * groupWidth);
    auto rightColumn = area.withLeft (2 * groupWidth);

    this->group_LFO.setBounds (leftColumn.removeFromTop (4*elementHeight));
    this->group_Vibrato.setBounds (middleColumn.removeFromTop (4*elementHeight));
    this->group_Modulation.setBounds (rightColumn.removeFromTop (4*elementHeight));

    this->group_Envelopes.setBounds (leftColumn.removeFromTop (5*elementHeight));
    this->group_PitchBend.setBounds (middleColumn.removeFromTop (5*elementHeight));
    this->group_Oscillators.setBounds (rightColumn.removeFromTop (5*elementHeight));

    this->group_Velocity.setBounds (leftColumn.removeFromTop (4*elementHeight));
    this->group_Aftertouch.setBounds (middleColumn.removeFromTop (4*elementHeight));
    this->group_Spread.setBounds (rightColumn.removeFromTop (4*elementHeight));

    this->group_Glide.setBounds (middleColumn.removeFromTop (4*elementHeight));
}


void PerformanceTab::setupGroupLFO()
{
    // 1 - LFO

    // note: LFO Target is a royal pain since it's not a simple one-to-one mapping and
    //       it is cobbeled together from multiple CCs but only one program bit field (!).
    //       To make life easier for me, this is still using CC identifiers and be translate
    //       the program field backwards when settings values.
    //
    //       see: Pro800ProgramLfoDestinationBitMask.

    this->combo_LFOtarget.addItem("Osc A & B", CC_LFO_TARGET_OSC_AB+1);
    this->combo_LFOtarget.addItem("Osc A", CC_LFO_TARGET_OSC_A+1);
    this->combo_LFOtarget.addItem("Osc B", CC_LFO_TARGET_OSC_B+1);
    this->combo_LFOtarget.addItem("VCA", CC_LFO_TARGET_VCA+1);
    this->combo_LFOspeed.addItem("Fast", PROGRAM_LFO_SPEED_FAST+1);
    this->combo_LFOspeed.addItem("Slow", PROGRAM_LFO_SPEED_SLOW+1);

    this->group_LFO.setTextLabelPosition(juce::Justification::left);
    this->group_LFO.addComponents( { 
        &label_LFOtarget, &combo_LFOtarget,
        &label_LFOspeed,  &combo_LFOspeed
    });

    this->setupMidiComponent(&combo_LFOtarget, CC_LFO_TARGET, PROGRAM_FIELD_LFO_DEST);
    this->setupMidiComponent(&combo_LFOspeed,  CC_LFO_SPEED,  PROGRAM_FIELD_LFO_SPEED);

    this->addAndMakeVisible(group_LFO);
}

void PerformanceTab::setupGroupVibrato()
{
    // 2 - Vibrato
    this->slider_VibratoSpeed.setRange(0.0, 127.0, 1.0);
    this->slider_VibratoAmount.setRange(0.0, 127.0, 1.0);

    this->group_Vibrato.setTextLabelPosition(juce::Justification::left);
    this->group_Vibrato.addComponents( {
        &label_VibratoSpeed,  &slider_VibratoSpeed,
        &label_VibratoAmount, &slider_VibratoAmount
    });

    this->setupMidiComponent(&slider_VibratoSpeed, CC_VIBRATO_SPEED, PROGRAM_FIELD_LFO_VIBRATO_FREQ);
    this->setupMidiComponent(&slider_VibratoAmount, CC_VIBRATO_AMOUNT, PROGRAM_FIELD_LFO_AMOUNT);

    this->addAndMakeVisible(group_Vibrato);
}

void PerformanceTab::setupGroupModulation()
{
    // 3 - Modulation
    this->combo_ModulationWheelAmount.addItem("Full", PROGRAM_MOD_WHEEL_AMOUNT_FULL+1);
    this->combo_ModulationWheelAmount.addItem("High", PROGRAM_MOD_WHEEL_AMOUNT_HIGH+1);
    this->combo_ModulationWheelAmount.addItem("Low", PROGRAM_MOD_WHEEL_AMOUNT_LOW+1);
    this->combo_ModulationWheelAmount.addItem("Min", PROGRAM_MOD_WHEEL_AMOUNT_MIN+1);
    this->combo_ModulationWheelTarget.addItem("LFO", PROGRAM_MOD_WHEEL_TARGET_LFO+1);
    this->combo_ModulationWheelTarget.addItem("Vibrato", PROGRAM_MOD_WHEEL_TARGET_VIBRATO+1);
    this->slider_ModulationDelay.setRange(0.0, 127.0, 1.0);

    this->group_Modulation.setTextLabelPosition(juce::Justification::left);
    this->group_Modulation.addComponents( {
        &label_ModulationWheelAmount, &combo_ModulationWheelAmount,
        &label_ModulationWheelTarget, &combo_ModulationWheelTarget,
        &label_ModulationDelay,       &slider_ModulationDelay
    });

    this->setupMidiComponent(&combo_ModulationWheelAmount, CC_MOD_WHEEL_AMOUNT, PROGRAM_FIELD_MODWHEEL_RANGE);
    this->setupMidiComponent(&combo_ModulationWheelTarget, CC_MOD_WHEEL_TARGET, PROGRAM_FIELD_MODWHEEL_TARGET);
    this->setupMidiComponent(&slider_ModulationDelay,      CC_MODULATION_DELAY, PROGRAM_FIELD_LFO_MODULATION_DELAY);

    this->addAndMakeVisible(group_Modulation);
}

void PerformanceTab::setupGroupEnvelopes()
{
    // 4 - Envelopes
    this->combo_EnvSpeedVCA.addItem("Fast", PROGRAM_ENV_SPEED_FAST+1);
    this->combo_EnvSpeedVCA.addItem("Slow", PROGRAM_ENV_SPEED_SLOW+1);
    this->combo_EnvShapeVCA.addItem("Exponential", PROGRAM_ENV_SHAPE_EXPONENTIAL+1);
    this->combo_EnvShapeVCA.addItem("Linear", PROGRAM_ENV_SHAPE_LINEAR+1);

    this->combo_EnvSpeedVCF.addItem("Fast", PROGRAM_ENV_SPEED_FAST+1);
    this->combo_EnvSpeedVCF.addItem("Slow", PROGRAM_ENV_SPEED_SLOW+1);
    this->combo_EnvShapeVCF.addItem("Exponential", PROGRAM_ENV_SHAPE_EXPONENTIAL+1);
    this->combo_EnvShapeVCF.addItem("Linear", PROGRAM_ENV_SHAPE_LINEAR+1);

    this->group_Envelopes.setTextLabelPosition(juce::Justification::left);
    this->group_Envelopes.addComponents( {
        &label_EnvSpeedVCA, &combo_EnvSpeedVCA,
        &label_EnvShapeVCA, &combo_EnvShapeVCA,
        &label_EnvSpeedVCF, &combo_EnvSpeedVCF,
        &label_EnvShapeVCF, &combo_EnvShapeVCF,        
    });

    this->setupMidiComponent(&combo_EnvSpeedVCA, CC_VCA_ENV_SPEED, PROGRAM_FIELD_AMP_ENV_SPEED);
    this->setupMidiComponent(&combo_EnvShapeVCA, CC_VCA_ENV_SHAPE, PROGRAM_FIELD_AMP_ENV_SHAPE);
  
    this->setupMidiComponent(&combo_EnvSpeedVCF, CC_VCF_ENV_SPEED, PROGRAM_FIELD_FILTER_ENV_SPEED);
    this->setupMidiComponent(&combo_EnvShapeVCF, CC_VCF_ENV_SHAPE, PROGRAM_FIELD_FILTER_ENV_SHAPE);
    
    this->addAndMakeVisible(group_Envelopes);
}

void PerformanceTab::setupGroupPitchBend()
{
    // 5 - Pitch Bend
    this->combo_PitchBendTarget.addItem("Off", PROGRAM_PITCH_BEND_TARGET_OFF+1);
    this->combo_PitchBendTarget.addItem("VCF", PROGRAM_PITCH_BEND_TARGET_VCF+1);
    this->combo_PitchBendTarget.addItem("VCO", PROGRAM_PITCH_BEND_TARGET_VCO+1);
    this->combo_PitchBendTarget.addItem("Volume", PROGRAM_PITCH_BEND_TARGET_VOLUME+1);
    this->slider_PitchBendRange.setRange(0.0, 31.0, 1.0);

    this->group_PitchBend.setTextLabelPosition(juce::Justification::left);
    this->group_PitchBend.addComponents( {
        &label_PitchBendTarget, &combo_PitchBendTarget,
        &label_PitchBendRange,  &slider_PitchBendRange
    });

    this->setupMidiComponent(&combo_PitchBendTarget, CC_PITCH_BEND_TARGET, PROGRAM_FIELD_PITCHBEND_TARGET);
    this->setupMidiComponent(&slider_PitchBendRange,  CC_PITCH_BEND_RANGE,  PROGRAM_FIELD_PITCHBEND_RANGE);

    this->addAndMakeVisible(group_PitchBend);
}

void PerformanceTab::setupGroupOscillators()
{
    // 6 - Oscillators
    this->combo_OscAFreqPotMode.addItem("Fixed", PROGRAM_FREQ_POT_MODE_FIXED+1);
    this->combo_OscAFreqPotMode.addItem("Free", PROGRAM_FREQ_POT_MODE_FREE+1);
    this->combo_OscAFreqPotMode.addItem("Semi-Tone", PROGRAM_FREQ_POT_MODE_SEMI+1);
    this->combo_OscAFreqPotMode.addItem("Octave", PROGRAM_FREQ_POT_MODE_OCT+1);
    this->combo_OscBFreqPotMode.addItem("Fixed", PROGRAM_FREQ_POT_MODE_FIXED+1);
    this->combo_OscBFreqPotMode.addItem("Free", PROGRAM_FREQ_POT_MODE_FREE+1);
    this->combo_OscBFreqPotMode.addItem("Semi-Tone", PROGRAM_FREQ_POT_MODE_SEMI+1);
    this->combo_OscBFreqPotMode.addItem("Octave", PROGRAM_FREQ_POT_MODE_OCT+1);
    this->combo_OscKeyboardTracking.addItem("C1", PROGRAM_KEYBOARD_TRACKING_REF_C1+1); 
    this->combo_OscKeyboardTracking.addItem("C2", PROGRAM_KEYBOARD_TRACKING_REF_C2+1); 
    this->combo_OscKeyboardTracking.addItem("C3", PROGRAM_KEYBOARD_TRACKING_REF_C3+1); 
    this->combo_OscKeyboardTracking.addItem("C4", PROGRAM_KEYBOARD_TRACKING_REF_C4+1); 

    this->group_Oscillators.setTextLabelPosition(juce::Justification::left);
    this->group_Oscillators.addComponents( {
        &label_OscAFreqPotMode,     &combo_OscAFreqPotMode,
        &label_OscBFreqPotMode,     &combo_OscBFreqPotMode,
        &label_OscKeyboardTracking, &combo_OscKeyboardTracking
    });
    
    this->setupMidiComponent(&combo_OscAFreqPotMode,     CC_OSC_A_FREQ_POT_MODE,   PROGRAM_FIELD_OSC_A_FREQ_POT_MODE);
    this->setupMidiComponent(&combo_OscBFreqPotMode,     CC_OSC_B_FREQ_POT_MODE,   PROGRAM_FIELD_OSC_B_FREQ_POT_MODE);
    this->setupMidiComponent(&combo_OscKeyboardTracking, CC_KEYBOARD_TRACKING_REF, PROGRAM_FIELD_KEY_TRACKING_REF_NOTE);

    this->addAndMakeVisible(group_Oscillators);
}

void PerformanceTab::setupGroupVelocity()
{
    // 7 - Velocity
    this->slider_VelocityAmountVCA.setRange(0.0, 127.0, 1.0);
    this->slider_VelocityAmountVCF.setRange(0.0, 127.0, 1.0);
    
    this->group_Velocity.setTextLabelPosition(juce::Justification::left);
    this->group_Velocity.addComponents( {
        &label_VelocityAmountVCA, &slider_VelocityAmountVCA,
        &label_VelocityAmountVCF, &slider_VelocityAmountVCF
    });
    
    this->setupMidiComponent(&slider_VelocityAmountVCA, CC_VCA_VELOCITY_AMOUNT, PROGRAM_FIELD_AMP_VELOCITY);
    this->setupMidiComponent(&slider_VelocityAmountVCF, CC_VCF_VELOCITY_AMOUNT, PROGRAM_FIELD_FILTER_VELOCITY);

    this->addAndMakeVisible(group_Velocity);
}

void PerformanceTab::setupGroupAftertouch()
{
    // 8 - Aftertouch
    this->slider_AfterTouchAmountVCA.setRange(0.0, 127.0, 1.0);
    this->slider_AfterTouchAmountVCF.setRange(0.0, 127.0, 1.0);
    this->slider_AfterTouchAmountLFO.setRange(0.0, 127.0, 1.0);

    this->group_Aftertouch.setTextLabelPosition(juce::Justification::left);
    this->group_Aftertouch.addComponents( {
        &label_AfterTouchAmountVCA, &slider_AfterTouchAmountVCA,
        &label_AfterTouchAmountVCF, &slider_AfterTouchAmountVCF,
        &label_AfterTouchAmountLFO, &slider_AfterTouchAmountLFO
    });

    this->setupMidiComponent(&slider_AfterTouchAmountVCA, CC_VCA_AFTERTOUCH_AMOUNT, PROGRAM_FIELD_AMP_AFTERTOUCH_AMOUNT);
    this->setupMidiComponent(&slider_AfterTouchAmountVCF, CC_VCF_AFTERTOUCH_AMOUNT, PROGRAM_FIELD_FILTER_AFTERTOUCH_AMOUNT);
    this->setupMidiComponent(&slider_AfterTouchAmountLFO, CC_LFO_AFTERTOUCH_AMOUNT, PROGRAM_FIELD_LFO_AFTERTOUCH_AMOUNT);

    this->addAndMakeVisible(group_Aftertouch);
}

void PerformanceTab::setupGroupSpread()
{
    // 9 - Spread
    this->checkBox_SpreadVoiceEnable.setButtonText("Enable");
    this->slider_SpreadUnisonDetune.setRange(0.0, 127.0, 1.0);

    this->group_Spread.setTextLabelPosition(juce::Justification::left);
    this->group_Spread.addComponents( {
        &label_SpreadUnisonDetune, &slider_SpreadUnisonDetune,
        &label_SpreadVoiceEnable,  &checkBox_SpreadVoiceEnable
    });

    this->setupMidiComponent(&checkBox_SpreadVoiceEnable, CC_VOICE_SPREAD_ENABLE, PROGRAM_FIELD_VOICE_SPREAD_ENABLE);
    this->setupMidiComponent(&slider_SpreadUnisonDetune,  CC_UNISON_SPREAD_DETUNE, PROGRAM_FIELD_UNISON_DETUNE);

    this->addAndMakeVisible(group_Spread);
}

void PerformanceTab::setupGroupGlide()
{
    // 0 - Glide
    this->combo_GlideMode.addItem("Speed", PROGRAM_GLIDE_MODE_SPEED+1);
    this->combo_GlideMode.addItem("Time", PROGRAM_GLIDE_MODE_TIME+1);

    this->group_Glide.setTextLabelPosition(juce::Justification::left);
    this->group_Glide.addComponents( {
        &label_GlideMode, &combo_GlideMode
    });

    this->setupMidiComponent(&combo_GlideMode, CC_GLIDE_MODE, PROGRAM_FIELD_GLIDE_MODE);

    addAndMakeVisible(group_Glide);
}
