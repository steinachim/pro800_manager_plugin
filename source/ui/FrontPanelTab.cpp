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

#include "FrontPanelTab.h"

#include "../midi/MidiHandler.h"
#include "../midi/ProgramMessage.h"

#include "../constants/Pro800CCConstants.h"
#include "../constants/Pro800ProgramConstants.h"

FrontPanelTab::FrontPanelTab(MidiHandler *midiHandler) : MidiComponent(midiHandler, true)
{
    setupGroupOscA();
    setupGroupOscB();
    setupGroupPolyMod();
    setupGroupNoise();
    setupGroupLFO();
    setupGroupGlide();
    setupGroupFilter();
    setupGroupAmplifier();
    setupGroupMaster();
}

FrontPanelTab::~FrontPanelTab()
{
}

void FrontPanelTab::resized()
{
    auto area = getLocalBounds().reduced(10);
    int groupHeight = area.getHeight() / 4;
    int groupWidth = area.getWidth()/2;
    
    auto leftArea = area.withTrimmedRight(groupWidth);
    auto rightArea = area.withTrimmedLeft(groupWidth);
    
    // left column
    this->group_OscillatorA.setBounds(leftArea.removeFromTop(groupHeight));
    this->group_OscillatorB.setBounds(leftArea.removeFromTop(groupHeight));
    
    this->group_PolyMod.setBounds(leftArea.withHeight(groupHeight).withRight(4*groupWidth/5));
    this->group_Noise.setBounds(leftArea.withHeight(groupHeight).withLeft(4*groupWidth/5));
    leftArea.removeFromTop(groupHeight);
    
    this->group_LFO.setBounds(leftArea.withHeight(groupHeight).withRight(4*groupWidth/5));
    this->group_Glide.setBounds(leftArea.withHeight(groupHeight).withLeft(4*groupWidth/5));
    leftArea.removeFromTop(groupHeight);
    
    // right column
    this->group_Filter.setBounds(rightArea.removeFromTop(2*groupHeight));
    this->group_Amplifier.setBounds(rightArea.removeFromTop(groupHeight));
    this->group_Master.setBounds(rightArea.removeFromTop(groupHeight));
}

void FrontPanelTab::setComponentValue(juce::Component *component, int value, int maxValue)
{
    int programField = component->getProperties().getWithDefault(PROGRAM_FIELD_PROPERTY, PROGRAM_FIELD_NONE);
    if ( programField == PROGRAM_FIELD_FILTER_KEY_TRACKING )
    {
        this->radio_FilterTrackingFull.setToggleState(value == PROGRAM_FILTER_KEYBOARD_TRACKING_FULL, juce::NotificationType::dontSendNotification);
        this->radio_FilterTrackingHalf.setToggleState(value == PROGRAM_FILTER_KEYBOARD_TRACKING_HALF, juce::NotificationType::dontSendNotification);
        this->radio_FilterTrackingOff.setToggleState (value == PROGRAM_FILTER_KEYBOARD_TRACKING_OFF,  juce::NotificationType::dontSendNotification);
    }
    else
    {
        MidiComponent::setComponentValue(component, value, maxValue);
    }
}

void FrontPanelTab::setupGroupOscA()
{
    group_OscillatorA.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_OscAFrequency, group_OscAFrequency);
    setupRotarySlider(slider_OscALevel, group_OscALevel);
    setupRotarySlider(slider_OscAPulseWidth, group_OscAPulseWidth);
   
    checkBox_OscASync.setButtonText("Enabled");
    group_OscASync.addComponent(&checkBox_OscASync);
    
    checkBox_OscAShapeSaw.setButtonText("Saw");
    checkBox_OscAShapeTri.setButtonText("Tri");
    checkBox_OscAShapeRect.setButtonText("Rect");
    group_OscAShape.addComponents({&checkBox_OscAShapeSaw, &checkBox_OscAShapeTri, &checkBox_OscAShapeRect});

    setupMidiComponent(&slider_OscAFrequency,   CC_OSC_A_FREQ,        PROGRAM_FIELD_OSC_A_FREQ);
    setupMidiComponent(&slider_OscALevel,       CC_OSC_A_LEVEL,       PROGRAM_FIELD_OSC_A_LEVEL);
    setupMidiComponent(&slider_OscAPulseWidth,  CC_OSC_A_PULSE_WIDTH, PROGRAM_FIELD_OSC_A_PULSE_WIDTH);
    setupMidiComponent(&checkBox_OscASync,      CC_OSC_A_SYNC,        PROGRAM_FIELD_OSC_A_SYNC);
    setupMidiComponent(&checkBox_OscAShapeSaw,  CC_OSC_A_SHAPE_SAW,   PROGRAM_FIELD_OSC_A_SHAPE_SAW);
    setupMidiComponent(&checkBox_OscAShapeTri,  CC_OSC_A_SHAPE_TRI,   PROGRAM_FIELD_OSC_A_SHAPE_TRI);
    setupMidiComponent(&checkBox_OscAShapeRect, CC_OSC_A_SHAPE_RECT,  PROGRAM_FIELD_OSC_A_SHAPE_RECT);

    group_OscillatorA.addComponents({ &group_OscAFrequency, &group_OscASync, &group_OscAShape, &group_OscAPulseWidth, &group_OscALevel });
    addAndMakeVisible(this->group_OscillatorA);
}

void FrontPanelTab::setupGroupOscB()
{
    group_OscillatorB.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_OscBFrequency, group_OscBFrequency);
    setupRotarySlider(slider_OscBFine, group_OscBFine);
    setupRotarySlider(slider_OscBPulseWidth, group_OscBPulseWidth);
    setupRotarySlider(slider_OscBLevel, group_OscBLevel);
    
    checkBox_OscBShapeSaw.setButtonText("Saw");
    checkBox_OscBShapeTri.setButtonText("Tri");
    checkBox_OscBShapeRect.setButtonText("Rect");
    group_OscBShape.addComponents({&checkBox_OscBShapeSaw, &checkBox_OscBShapeTri, &checkBox_OscBShapeRect});

    setupMidiComponent(&slider_OscBFrequency,  CC_OSC_B_FREQ,        PROGRAM_FIELD_OSC_B_FREQ);
    setupMidiComponent(&slider_OscBFine,       CC_OSC_B_FINE,        PROGRAM_FIELD_OSC_B_FINE_FREQ);
    setupMidiComponent(&slider_OscBPulseWidth, CC_OSC_B_PULSE_WIDTH, PROGRAM_FIELD_OSC_B_PULSE_WIDTH);
    setupMidiComponent(&slider_OscBLevel,      CC_OSC_B_LEVEL,       PROGRAM_FIELD_OSC_B_LEVEL);
    
    setupMidiComponent(&checkBox_OscBShapeSaw,  CC_OSC_B_SHAPE_SAW,  PROGRAM_FIELD_OSC_B_SHAPE_SAW);
    setupMidiComponent(&checkBox_OscBShapeTri,  CC_OSC_B_SHAPE_TRI,  PROGRAM_FIELD_OSC_B_SHAPE_TRI);
    setupMidiComponent(&checkBox_OscBShapeRect, CC_OSC_B_SHAPE_RECT, PROGRAM_FIELD_OSC_B_SHAPE_RECT);
    
    group_OscillatorB.addComponents({ &group_OscBFrequency, &group_OscBFine, &group_OscBShape, &group_OscBPulseWidth, &group_OscBLevel});
    addAndMakeVisible(this->group_OscillatorB);
}

void FrontPanelTab::setupGroupPolyMod()
{
    group_PolyMod.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_PolyModSourceFilterEnv, group_PolyModSourceFilterEnv);
    setupRotarySlider(slider_PolyModSourceOscB, group_PolyModSourceOscB);
    group_PolyModSourceAmount.addComponents({&group_PolyModSourceFilterEnv, &group_PolyModSourceOscB});
    group_PolyModSourceAmount.setInnerMargin(0);

    checkBox_PolyModDestFreqA.setButtonText("Freq A");
    checkBox_PolyModDestFilter.setButtonText("Filter");
    group_PolyModDestination.addComponents({&checkBox_PolyModDestFreqA, &checkBox_PolyModDestFilter});
    
    checkBox_PolyModUnisonTrack.setButtonText("Enable");
    group_PolyModUnisonTrack.addComponent(&checkBox_PolyModUnisonTrack);

    setupMidiComponent(&slider_PolyModSourceFilterEnv, CC_POLY_MOD_SOURCE_FILTER_ENV, PROGRAM_FIELD_POLYMOD_SOURCE_FILTER_ENV);
    setupMidiComponent(&slider_PolyModSourceOscB,      CC_POLY_MOD_SOURCE_OSC_B,      PROGRAM_FIELD_POLYMOD_SOURCE_OSC_B);
    setupMidiComponent(&checkBox_PolyModDestFreqA,     CC_POLY_MOD_DEST_FREQ_A,       PROGRAM_FIELD_POLYMOD_DEST_FREQ_A);
    setupMidiComponent(&checkBox_PolyModDestFilter,    CC_POLY_MOD_DEST_FILTER,       PROGRAM_FIELD_POLYMOD_DEST_FILTER);
    setupMidiComponent(&checkBox_PolyModUnisonTrack,   CC_POLY_MOD_UNISON_TRACK,      PROGRAM_FIELD_POLYMOD_UNISON_TRACK);
    
    group_PolyMod.addComponent(&group_PolyModSourceAmount, 1, 2);
    group_PolyMod.addComponents({&group_PolyModDestination, &group_PolyModUnisonTrack});
    addAndMakeVisible(this->group_PolyMod);
}

void FrontPanelTab::setupGroupNoise()
{
    group_Noise.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_NoiseLevel, group_NoiseLevel);

    setupMidiComponent(&slider_NoiseLevel, CC_NOISE_LEVEL, PROGRAM_FIELD_NOISE);
    group_Noise.addComponent(&group_NoiseLevel);
    addAndMakeVisible(this->group_Noise);
}

void FrontPanelTab::setupGroupLFO()
{
    group_LFO.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_LFOFrequency, group_LFOFrequency);
    setupRotarySlider(slider_LFOInitialAmount, group_LFOInitialAmount);

    // note: LFO Target is a royal pain since it's not a simple one-to-one mapping and
    //       it is cobbeled together from multiple CCs but only one program bit field (!).
    //       To make life easier for me, this is still using CC identifiers and be translate
    //       the program field backwards when settings values.
    //
    //       see: Pro800ProgramLfoDestinationBitMask.
    checkBox_LFODestFreqAB.setButtonText("Freq A-B");
    checkBox_LFODestPulseWidthAB.setButtonText("PW A-B");
    checkBox_LFODestFilter.setButtonText("Filter");
    group_LFODestination.addComponents({&checkBox_LFODestFreqAB, &checkBox_LFODestPulseWidthAB, &checkBox_LFODestFilter});

    comboBox_LFOShape.addItem("Sine",     PROGRAM_LFO_SHAPE_SINE+1);
    comboBox_LFOShape.addItem("Triangle", PROGRAM_LFO_SHAPE_TRIANGLE+1);   
    comboBox_LFOShape.addItem("Saw",      PROGRAM_LFO_SHAPE_SAW+1);
    comboBox_LFOShape.addItem("Pulse",    PROGRAM_LFO_SHAPE_PULSE+1);
    comboBox_LFOShape.addItem("Random",   PROGRAM_LFO_SHAPE_RANDOM+1);
    comboBox_LFOShape.addItem("Noise",    PROGRAM_LFO_SHAPE_NOISE+1);
    group_LFOShape.addComponent(&comboBox_LFOShape);

    setupMidiComponent(&slider_LFOFrequency,          CC_LFO_MOD_FREQ,           PROGRAM_FIELD_LFO_FREQ);
    setupMidiComponent(&slider_LFOInitialAmount,      CC_LFO_MOD_INITIAL_AMOUNT, PROGRAM_FIELD_LFO_AMOUNT);
    setupMidiComponent(&checkBox_LFODestFreqAB,       CC_LFO_MOD_DEST_FREQ_AB,   PROGRAM_FIELD_LFO_DEST);
    setupMidiComponent(&checkBox_LFODestPulseWidthAB, CC_LFO_MOD_DEST_PW_AB,     PROGRAM_FIELD_LFO_DEST);
    setupMidiComponent(&checkBox_LFODestFilter,       CC_LFO_MOD_DEST_FILTER,    PROGRAM_FIELD_LFO_DEST);
    setupMidiComponent(&comboBox_LFOShape,            CC_LFO_MOD_SHAPE,          PROGRAM_FIELD_LFO_SHAPE);

    group_LFO.addComponents({&group_LFOFrequency, &group_LFOShape, &group_LFOInitialAmount, &group_LFODestination});
    addAndMakeVisible(this->group_LFO);
}

void FrontPanelTab::setupGroupGlide()
{
    group_Glide.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_GlideAmount, group_GlideAmount);

    setupMidiComponent(&slider_GlideAmount, CC_GLIDE_TIME, PROGRAM_FIELD_GLIDE_AMOUNT);
    group_Glide.addComponent(&group_GlideAmount);

    addAndMakeVisible(group_Glide);
}

void FrontPanelTab::setupGroupFilter()
{    
    group_Filter.setTextLabelPosition(juce::Justification::left);

    setupRotarySlider(slider_FilterCutoff, group_FilterCutoff);
    setupRotarySlider(slider_FilterResonance, group_FilterResonance);
    setupRotarySlider(slider_FilterEnvAmount, group_FilterEnvAmount);

    setupRotarySlider(slider_FilterAttack, group_FilterAttack);
    setupRotarySlider(slider_FilterDecay, group_FilterDecay);
    setupRotarySlider(slider_FilterSustain, group_FilterSustain);
    setupRotarySlider(slider_FilterRelease, group_FilterRelease);
    
    radio_FilterTrackingFull.setButtonText("Full");
    radio_FilterTrackingFull.setRadioGroupId(1000);
    radio_FilterTrackingFull.getProperties().set(RADIO_VALUE_PROPERTY, PROGRAM_FILTER_KEYBOARD_TRACKING_FULL);
    radio_FilterTrackingHalf.setButtonText("1/2");
    radio_FilterTrackingHalf.setRadioGroupId(1000);
    radio_FilterTrackingHalf.getProperties().set(RADIO_VALUE_PROPERTY, PROGRAM_FILTER_KEYBOARD_TRACKING_HALF);
    radio_FilterTrackingOff.setButtonText("Off");
    radio_FilterTrackingOff.setRadioGroupId(1000);
    radio_FilterTrackingOff.getProperties().set(RADIO_VALUE_PROPERTY, PROGRAM_FILTER_KEYBOARD_TRACKING_OFF);
    group_FilterKeyboardTracking.addComponents({&radio_FilterTrackingFull, &radio_FilterTrackingHalf, &radio_FilterTrackingOff});
    
    group_Filter.addComponents({&group_FilterCutoff, &group_FilterResonance, &group_FilterEnvAmount, &group_FilterKeyboardTracking,
                                &group_FilterAttack, &group_FilterDecay, &group_FilterSustain, &group_FilterRelease});

    setupMidiComponent(&slider_FilterCutoff,     CC_FILTER_CUTOFF,     PROGRAM_FIELD_FILTER_CUTOFF);
    setupMidiComponent(&slider_FilterResonance,  CC_FILTER_RESONANCE,  PROGRAM_FIELD_FILTER_RESONANCE);
    setupMidiComponent(&slider_FilterEnvAmount,  CC_FILTER_ENV_AMOUNT, PROGRAM_FIELD_FILTER_ENV_AMOUNT);
    setupMidiComponent(&slider_FilterAttack,     CC_FILTER_ATTACK,     PROGRAM_FIELD_FILTER_ATTACK);
    setupMidiComponent(&slider_FilterDecay,      CC_FILTER_DECAY,      PROGRAM_FIELD_FILTER_DECAY);
    setupMidiComponent(&slider_FilterSustain,    CC_FILTER_SUSTAIN,    PROGRAM_FIELD_FILTER_SUSTAIN);
    setupMidiComponent(&slider_FilterRelease,    CC_FILTER_RELEASE,    PROGRAM_FIELD_FILTER_RELEASE);

    setupMidiComponent(&radio_FilterTrackingFull, CC_FILTER_KEYBOARD_TRACKING, PROGRAM_FIELD_FILTER_KEY_TRACKING); // use special handling in overridden setComponentValue()
    setupMidiComponent(&radio_FilterTrackingHalf, CC_FILTER_KEYBOARD_TRACKING, PROGRAM_FIELD_FILTER_KEY_TRACKING); 
    setupMidiComponent(&radio_FilterTrackingOff,  CC_FILTER_KEYBOARD_TRACKING, PROGRAM_FIELD_FILTER_KEY_TRACKING);
    
    addAndMakeVisible(group_Filter);
}

void FrontPanelTab::setupGroupAmplifier()
{
    group_Amplifier.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_AmplifierAttack, group_AmplifierAttack);
    setupRotarySlider(slider_AmplifierDecay, group_AmplifierDecay);
    setupRotarySlider(slider_AmplifierSustain, group_AmplifierSustain);
    setupRotarySlider(slider_AmplifierRelease, group_AmplifierRelease);

    setupMidiComponent(&slider_AmplifierAttack,  CC_AMP_ATTACK,  PROGRAM_FIELD_AMP_ATTACK);
    setupMidiComponent(&slider_AmplifierDecay,   CC_AMP_DECAY,   PROGRAM_FIELD_AMP_DECAY);
    setupMidiComponent(&slider_AmplifierSustain, CC_AMP_SUSTAIN, PROGRAM_FIELD_AMP_SUSTAIN);
    setupMidiComponent(&slider_AmplifierRelease, CC_AMP_RELEASE, PROGRAM_FIELD_AMP_RELEASE);

    group_Amplifier.addComponents({&group_AmplifierAttack, &group_AmplifierDecay, &group_AmplifierSustain, &group_AmplifierRelease});
    addAndMakeVisible(this->group_Amplifier);
}

void FrontPanelTab::setupGroupMaster()
{
    group_Master.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_MasterTune, group_MasterTune);
    setupRotarySlider(slider_MasterVolume, group_MasterVolume);

    setupMidiComponent(&slider_MasterVolume, CC_MASTER_VOLUME, PROGRAM_FIELD_NONE);
    setupMidiComponent(&slider_MasterTune,   CC_MASTER_TUNE,   PROGRAM_FIELD_NONE);

    group_Master.addComponents({&group_MasterTune, &group_MasterVolume});
    addAndMakeVisible(this->group_Master);
}

void FrontPanelTab::setupRotarySlider(juce::Slider &slider, EqualSpacingGroupComponent &parent)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setRange(0.0, 65535.0, 1.0);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 15);
    slider.setPopupDisplayEnabled(false, false, &parent);
    slider.setValue(0.0, juce::NotificationType::dontSendNotification);
    
    parent.addComponent(&slider);
    parent.setInnerMargin(0);
}
