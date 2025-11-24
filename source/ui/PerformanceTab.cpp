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

void PerformanceTab::loadFromProgram(const std::shared_ptr<ProgramMessage> &programMessage)
{
    setComponentValue(&this->slider_VibratoAmount, Pro800CCMessages::VIBRATO_AMOUNT, programMessage->getValue(Pro800ProgramField::PROGRAM_FIELD_LFO_VIBRATO_AMOUNT), 65535);
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
    this->combo_LFOtarget.addItem("Osc A & B", Pro800LFOTarget::LFO_TARGET_OSC_AB+1);
    this->combo_LFOtarget.addItem("Osc A", Pro800LFOTarget::LFO_TARGET_OSC_A+1);
    this->combo_LFOtarget.addItem("Osc B", Pro800LFOTarget::LFO_TARGET_OSC_B+1);
    this->combo_LFOtarget.addItem("VCA", Pro800LFOTarget::LFO_TARGET_VCA+1);
    this->combo_LFOspeed.addItem("Fast", Pro800LFOSpeed::LFO_SPEED_FAST+1);
    this->combo_LFOspeed.addItem("Slow", Pro800LFOSpeed::LFO_SPEED_SLOW+1);

    this->group_LFO.setTextLabelPosition(juce::Justification::left);
    this->group_LFO.addComponents( { 
        &label_LFOtarget, &combo_LFOtarget,
        &label_LFOspeed,  &combo_LFOspeed
    });

    this->setupMidiCCComponent(Pro800CCMessages::LFO_TARGET, &combo_LFOtarget);
    this->setupMidiCCComponent(Pro800CCMessages::LFO_SPEED, &combo_LFOspeed);

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

    this->setupMidiCCComponent(Pro800CCMessages::VIBRATO_SPEED, &slider_VibratoSpeed);
    this->setupMidiCCComponent(Pro800CCMessages::VIBRATO_AMOUNT, &slider_VibratoAmount);

    this->addAndMakeVisible(group_Vibrato);
}

void PerformanceTab::setupGroupModulation()
{
    // 3 - Modulation
    this->combo_ModulationWheelAmount.addItem("Full", Pro800ModWheelAmount::MOD_WHEEL_AMOUNT_FULL+1);
    this->combo_ModulationWheelAmount.addItem("High", Pro800ModWheelAmount::MOD_WHEEL_AMOUNT_HIGH+1);
    this->combo_ModulationWheelAmount.addItem("Low", Pro800ModWheelAmount::MOD_WHEEL_AMOUNT_LOW+1);
    this->combo_ModulationWheelAmount.addItem("Min", Pro800ModWheelAmount::MOD_WHEEL_AMOUNT_MIN+1);
    this->combo_ModulationWheelTarget.addItem("LFO", Pro800ModWheelTarget::MOD_WHEEL_TARGET_LFO+1);
    this->combo_ModulationWheelTarget.addItem("Vibrato", Pro800ModWheelTarget::MOD_WHEEL_TARGET_VIBRATO+1);
    this->slider_ModulationDelay.setRange(0.0, 127.0, 1.0);

    this->group_Modulation.setTextLabelPosition(juce::Justification::left);
    this->group_Modulation.addComponents( {
        &label_ModulationWheelAmount, &combo_ModulationWheelAmount,
        &label_ModulationWheelTarget, &combo_ModulationWheelTarget,
        &label_ModulationDelay,       &slider_ModulationDelay
    });

    this->setupMidiCCComponent(Pro800CCMessages::MOD_WHEEL_AMOUNT, &combo_ModulationWheelAmount);
    this->setupMidiCCComponent(Pro800CCMessages::MOD_WHEEL_TARGET, &combo_ModulationWheelTarget);
    this->setupMidiCCComponent(Pro800CCMessages::MODULATION_DELAY, &slider_ModulationDelay);

    this->addAndMakeVisible(group_Modulation);
}

void PerformanceTab::setupGroupEnvelopes()
{
    // 4 - Envelopes
    this->combo_EnvSpeedVCA.addItem("Fast", Pro800EnvelopeSpeed::ENV_SPEED_FAST+1);
    this->combo_EnvSpeedVCA.addItem("Slow", Pro800EnvelopeSpeed::ENV_SPEED_SLOW+1);
    this->combo_EnvShapeVCA.addItem("Exponential", Pro800EnvelopeShape::ENV_SHAPE_EXPONENTIAL+1);
    this->combo_EnvShapeVCA.addItem("Linear", Pro800EnvelopeShape::ENV_SHAPE_LINEAR+1);

    this->combo_EnvSpeedVCF.addItem("Fast", Pro800EnvelopeSpeed::ENV_SPEED_FAST+1);
    this->combo_EnvSpeedVCF.addItem("Slow", Pro800EnvelopeSpeed::ENV_SPEED_SLOW+1);
    this->combo_EnvShapeVCF.addItem("Exponential", Pro800EnvelopeShape::ENV_SHAPE_EXPONENTIAL+1);
    this->combo_EnvShapeVCF.addItem("Linear", Pro800EnvelopeShape::ENV_SHAPE_LINEAR+1);

    this->group_Envelopes.setTextLabelPosition(juce::Justification::left);
    this->group_Envelopes.addComponents( {
        &label_EnvSpeedVCA, &combo_EnvSpeedVCA,
        &label_EnvShapeVCA, &combo_EnvShapeVCA,
        &label_EnvSpeedVCF, &combo_EnvSpeedVCF,
        &label_EnvShapeVCF, &combo_EnvShapeVCF,        
    });

    this->setupMidiCCComponent(Pro800CCMessages::VCA_ENV_SPEED, &combo_EnvSpeedVCA);
    this->setupMidiCCComponent(Pro800CCMessages::VCA_ENV_SHAPE, &combo_EnvShapeVCA);
  
    this->setupMidiCCComponent(Pro800CCMessages::VCF_ENV_SPEED, &combo_EnvSpeedVCF);
    this->setupMidiCCComponent(Pro800CCMessages::VCF_ENV_SHAPE, &combo_EnvShapeVCF);
    
    this->addAndMakeVisible(group_Envelopes);
}

void PerformanceTab::setupGroupPitchBend()
{
    // 5 - Pitch Bend
    this->combo_PitchBendTarget.addItem("Off", Pro800PitchBendTarget::PITCH_BEND_TARGET_OFF+1);
    this->combo_PitchBendTarget.addItem("VCF", Pro800PitchBendTarget::PITCH_BEND_TARGET_VCF+1);
    this->combo_PitchBendTarget.addItem("VCO", Pro800PitchBendTarget::PITCH_BEND_TARGET_VCO+1);
    this->combo_PitchBendTarget.addItem("Volume", Pro800PitchBendTarget::PITCH_BEND_TARGET_VOLUME+1);
    this->slider_PitchBendRange.setRange(0.0, 31.0, 1.0);

    this->group_PitchBend.setTextLabelPosition(juce::Justification::left);
    this->group_PitchBend.addComponents( {
        &label_PitchBendTarget, &combo_PitchBendTarget,
        &label_PitchBendRange,  &slider_PitchBendRange
    });

    this->setupMidiCCComponent(Pro800CCMessages::PITCH_BEND_TARGET, &combo_PitchBendTarget);
    this->setupMidiCCComponent(Pro800CCMessages::PITCH_BEND_RANGE, &slider_PitchBendRange);

    this->addAndMakeVisible(group_PitchBend);
}

void PerformanceTab::setupGroupOscillators()
{
    // 6 - Oscillators
    this->combo_OscAFreqPotMode.addItem("Fixed", Pro800FreqPotMode::FREQ_POT_MODE_FIXED+1);
    this->combo_OscAFreqPotMode.addItem("Free", Pro800FreqPotMode::FREQ_POT_MODE_FREE+1);
    this->combo_OscAFreqPotMode.addItem("Semi-Tone", Pro800FreqPotMode::FREQ_POT_MODE_SEMI+1);
    this->combo_OscAFreqPotMode.addItem("Octave", Pro800FreqPotMode::FREQ_POT_MODE_OCT+1);
    this->combo_OscBFreqPotMode.addItem("Fixed", Pro800FreqPotMode::FREQ_POT_MODE_FIXED+1);
    this->combo_OscBFreqPotMode.addItem("Free", Pro800FreqPotMode::FREQ_POT_MODE_FREE+1);
    this->combo_OscBFreqPotMode.addItem("Semi-Tone", Pro800FreqPotMode::FREQ_POT_MODE_SEMI+1);
    this->combo_OscBFreqPotMode.addItem("Octave", Pro800FreqPotMode::FREQ_POT_MODE_OCT+1);
    this->combo_OscKeyboardTracking.addItem("C1", Pro800KeyboardTracking::KEYBOARD_TRACKING_C1+1); 
    this->combo_OscKeyboardTracking.addItem("C2", Pro800KeyboardTracking::KEYBOARD_TRACKING_C2+1); 
    this->combo_OscKeyboardTracking.addItem("C3", Pro800KeyboardTracking::KEYBOARD_TRACKING_C3+1); 
    this->combo_OscKeyboardTracking.addItem("C4", Pro800KeyboardTracking::KEYBOARD_TRACKING_C4+1); 

    this->group_Oscillators.setTextLabelPosition(juce::Justification::left);
    this->group_Oscillators.addComponents( {
        &label_OscAFreqPotMode,     &combo_OscAFreqPotMode,
        &label_OscBFreqPotMode,     &combo_OscBFreqPotMode,
        &label_OscKeyboardTracking, &combo_OscKeyboardTracking
    });
    
    this->setupMidiCCComponent(Pro800CCMessages::OSC_A_FREQ_POT_MODE, &combo_OscAFreqPotMode);
    this->setupMidiCCComponent(Pro800CCMessages::OSC_B_FREQ_POT_MODE, &combo_OscBFreqPotMode);
    this->setupMidiCCComponent(Pro800CCMessages::KEYBOARD_TRACKING, &combo_OscKeyboardTracking);

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
    
    this->setupMidiCCComponent(Pro800CCMessages::VCA_VELOCITY_AMOUNT, &slider_VelocityAmountVCA);
    this->setupMidiCCComponent(Pro800CCMessages::VCF_VELOCITY_AMOUNT, &slider_VelocityAmountVCF);

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

    this->setupMidiCCComponent(Pro800CCMessages::VCA_AFTERTOUCH_AMOUNT, &slider_AfterTouchAmountVCA);
    this->setupMidiCCComponent(Pro800CCMessages::VCF_AFTERTOUCH_AMOUNT, &slider_AfterTouchAmountVCF);
    this->setupMidiCCComponent(Pro800CCMessages::LFO_AFTERTOUCH_AMOUNT, &slider_AfterTouchAmountLFO);

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

    this->setupMidiCCComponent(Pro800CCMessages::VOICE_SPREAD_ENABLE, &checkBox_SpreadVoiceEnable);
    this->setupMidiCCComponent(Pro800CCMessages::UNISON_SPREAD_DETUNE, &slider_SpreadUnisonDetune);

    this->addAndMakeVisible(group_Spread);
}

void PerformanceTab::setupGroupGlide()
{
    // 0 - Glide
    this->combo_GlideMode.addItem("Speed", Pro800GlideMode::GLIDE_MODE_SPEED+1);
    this->combo_GlideMode.addItem("Time", Pro800GlideMode::GLIDE_MODE_TIME+1);

    this->group_Glide.setTextLabelPosition(juce::Justification::left);
    this->group_Glide.addComponents( {
        &label_GlideMode, &combo_GlideMode
    });

    this->setupMidiCCComponent(Pro800CCMessages::GLIDE_MODE, &combo_GlideMode);

    addAndMakeVisible(group_Glide);
}
