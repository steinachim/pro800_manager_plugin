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
#include "../Pro800Constants.h"

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

void FrontPanelTab::loadFromProgram(const std::shared_ptr<ProgramMessage> &programMessage)
{
    std::cerr << "FrontPanelTab::loadFromProgram: Loading program " << programMessage->toString() << std::endl;
    setComponentValue(&this->slider_OscAFrequency, Pro800CCMessages::OSC_A_FREQ, programMessage->getValue(Pro800ProgramField::PROGRAM_FIELD_OSC_A_FREQ), 65535);

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

void FrontPanelTab::setComponentValue(juce::Component *component, int midiCC, int value, int maxValue)
{
    if ( midiCC == Pro800CCMessages::FILTER_KEYBOARD_TRACKING )
    {
        this->radio_FilterTrackingFull.setToggleState(value == Pro800FilterKeyboardTracking::TRACKING_FULL, juce::NotificationType::dontSendNotification);
        this->radio_FilterTrackingHalf.setToggleState(value == Pro800FilterKeyboardTracking::TRACKING_HALF, juce::NotificationType::dontSendNotification);
        this->radio_FilterTrackingOff.setToggleState (value == Pro800FilterKeyboardTracking::TRACKING_OFF,  juce::NotificationType::dontSendNotification);
    }
    else
    {
        MidiComponent::setComponentValue(component, midiCC, value, maxValue);
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

    setupMidiCCComponent(Pro800CCMessages::OSC_A_FREQ, &slider_OscAFrequency);
    setupMidiCCComponent(Pro800CCMessages::OSC_A_LEVEL, &slider_OscALevel);
    setupMidiCCComponent(Pro800CCMessages::OSC_A_PULSE_WIDTH, &slider_OscAPulseWidth);
    setupMidiCCComponent(Pro800CCMessages::OSC_A_SYNC, &checkBox_OscASync);
    setupMidiCCComponent(Pro800CCMessages::OSC_A_SHAPE_SAW, &checkBox_OscAShapeSaw);
    setupMidiCCComponent(Pro800CCMessages::OSC_A_SHAPE_TRI, &checkBox_OscAShapeTri);
    setupMidiCCComponent(Pro800CCMessages::OSC_A_SHAPE_RECT, &checkBox_OscAShapeRect);

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

    setupMidiCCComponent(Pro800CCMessages::OSC_B_FREQ, &slider_OscBFrequency);
    setupMidiCCComponent(Pro800CCMessages::OSC_B_FINE, &slider_OscBFine);
    setupMidiCCComponent(Pro800CCMessages::OSC_B_PULSE_WIDTH, &slider_OscBPulseWidth);
    setupMidiCCComponent(Pro800CCMessages::OSC_B_LEVEL, &slider_OscBLevel);
    
    setupMidiCCComponent(Pro800CCMessages::OSC_B_SHAPE_SAW, &checkBox_OscBShapeSaw);
    setupMidiCCComponent(Pro800CCMessages::OSC_B_SHAPE_TRI, &checkBox_OscBShapeTri);
    setupMidiCCComponent(Pro800CCMessages::OSC_B_SHAPE_RECT, &checkBox_OscBShapeRect);
    
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

    setupMidiCCComponent(Pro800CCMessages::POLY_MOD_SOURCE_FILTER_ENV, &slider_PolyModSourceFilterEnv);
    setupMidiCCComponent(Pro800CCMessages::POLY_MOD_SOURCE_OSC_B, &slider_PolyModSourceOscB);
    setupMidiCCComponent(Pro800CCMessages::POLY_MOD_DEST_FREQ_A, &checkBox_PolyModDestFreqA);
    setupMidiCCComponent(Pro800CCMessages::POLY_MOD_DEST_FILTER, &checkBox_PolyModDestFilter);
    setupMidiCCComponent(Pro800CCMessages::POLY_MOD_UNISON_TRACK, &checkBox_PolyModUnisonTrack);
    
    group_PolyMod.addComponent(&group_PolyModSourceAmount, 1, 2);
    group_PolyMod.addComponents({&group_PolyModDestination, &group_PolyModUnisonTrack});
    addAndMakeVisible(this->group_PolyMod);
}

void FrontPanelTab::setupGroupNoise()
{
    group_Noise.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_NoiseLevel, group_NoiseLevel);

    setupMidiCCComponent(Pro800CCMessages::NOISE_LEVEL, &slider_NoiseLevel);
    group_Noise.addComponent(&group_NoiseLevel);
    addAndMakeVisible(this->group_Noise);
}

void FrontPanelTab::setupGroupLFO()
{
    group_LFO.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_LFOFrequency, group_LFOFrequency);
    setupRotarySlider(slider_LFOInitialAmount, group_LFOInitialAmount);

    checkBox_LFODestFreqAB.setButtonText("Freq A-B");
    checkBox_LFODestPulseWidthAB.setButtonText("PW A-B");
    checkBox_LFODestFilter.setButtonText("Filter");
    group_LFODestination.addComponents({&checkBox_LFODestFreqAB, &checkBox_LFODestPulseWidthAB, &checkBox_LFODestFilter});

    comboBox_LFOShape.addItem("Sine", Pro800LFOShape::LFO_SHAPE_SINE+1);
    comboBox_LFOShape.addItem("Triangle", Pro800LFOShape::LFO_SHAPE_TRIANGLE+1);   
    comboBox_LFOShape.addItem("Saw", Pro800LFOShape::LFO_SHAPE_SAW+1);
    comboBox_LFOShape.addItem("Pulse", Pro800LFOShape::LFO_SHAPE_PULSE+1);
    comboBox_LFOShape.addItem("Random", Pro800LFOShape::LFO_SHAPE_RANDOM+1);
    comboBox_LFOShape.addItem("Noise", Pro800LFOShape::LFO_SHAPE_NOISE+1);
    group_LFOShape.addComponent(&comboBox_LFOShape);

    setupMidiCCComponent(Pro800CCMessages::LFO_MOD_FREQ, &slider_LFOFrequency);
    setupMidiCCComponent(Pro800CCMessages::LFO_MOD_INITIAL_AMOUNT, &slider_LFOInitialAmount);
    setupMidiCCComponent(Pro800CCMessages::LFO_MOD_DEST_FREQ_AB, &checkBox_LFODestFreqAB);
    setupMidiCCComponent(Pro800CCMessages::LFO_MOD_DEST_PW_AB, &checkBox_LFODestPulseWidthAB);
    setupMidiCCComponent(Pro800CCMessages::LFO_MOD_DEST_FILTER, &checkBox_LFODestFilter);
    setupMidiCCComponent(Pro800CCMessages::LFO_MOD_SHAPE, &comboBox_LFOShape);

    group_LFO.addComponents({&group_LFOFrequency, &group_LFOShape, &group_LFOInitialAmount, &group_LFODestination});
    addAndMakeVisible(this->group_LFO);
}

void FrontPanelTab::setupGroupGlide()
{
    group_Glide.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_GlideAmount, group_GlideAmount);

    setupMidiCCComponent(Pro800CCMessages::GLIDE_TIME, &slider_GlideAmount);
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
    radio_FilterTrackingFull.getProperties().set(RADIO_VALUE_PROPERTY, Pro800FilterKeyboardTracking::TRACKING_FULL);
    radio_FilterTrackingHalf.setButtonText("1/2");
    radio_FilterTrackingHalf.setRadioGroupId(1000);
    radio_FilterTrackingHalf.getProperties().set(RADIO_VALUE_PROPERTY, Pro800FilterKeyboardTracking::TRACKING_HALF);
    radio_FilterTrackingOff.setButtonText("Off");
    radio_FilterTrackingOff.setRadioGroupId(1000);
    radio_FilterTrackingOff.getProperties().set(RADIO_VALUE_PROPERTY, Pro800FilterKeyboardTracking::TRACKING_OFF);
    group_FilterKeyboardTracking.addComponents({&radio_FilterTrackingFull, &radio_FilterTrackingHalf, &radio_FilterTrackingOff});
    
    group_Filter.addComponents({&group_FilterCutoff, &group_FilterResonance, &group_FilterEnvAmount, &group_FilterKeyboardTracking,
                                &group_FilterAttack, &group_FilterDecay, &group_FilterSustain, &group_FilterRelease});

    setupMidiCCComponent(Pro800CCMessages::FILTER_CUTOFF, &slider_FilterCutoff);
    setupMidiCCComponent(Pro800CCMessages::FILTER_RESONANCE, &slider_FilterResonance);
    setupMidiCCComponent(Pro800CCMessages::FILTER_ENV_AMOUNT, &slider_FilterEnvAmount);
    setupMidiCCComponent(Pro800CCMessages::FILTER_ATTACK, &slider_FilterAttack);
    setupMidiCCComponent(Pro800CCMessages::FILTER_DECAY, &slider_FilterDecay);
    setupMidiCCComponent(Pro800CCMessages::FILTER_SUSTAIN, &slider_FilterSustain);
    setupMidiCCComponent(Pro800CCMessages::FILTER_RELEASE, &slider_FilterRelease);

    setupMidiCCComponent(Pro800CCMessages::FILTER_KEYBOARD_TRACKING, &radio_FilterTrackingFull); // use special handling in overridden setComponentValue()
    setupMidiCCComponent(Pro800CCMessages::FILTER_KEYBOARD_TRACKING, &radio_FilterTrackingHalf);
    setupMidiCCComponent(Pro800CCMessages::FILTER_KEYBOARD_TRACKING, &radio_FilterTrackingOff);
    
    addAndMakeVisible(group_Filter);
}

void FrontPanelTab::setupGroupAmplifier()
{
    group_Amplifier.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_AmplifierAttack, group_AmplifierAttack);
    setupRotarySlider(slider_AmplifierDecay, group_AmplifierDecay);
    setupRotarySlider(slider_AmplifierSustain, group_AmplifierSustain);
    setupRotarySlider(slider_AmplifierRelease, group_AmplifierRelease);

    setupMidiCCComponent(Pro800CCMessages::AMP_ATTACK, &slider_AmplifierAttack);
    setupMidiCCComponent(Pro800CCMessages::AMP_DECAY, &slider_AmplifierDecay);
    setupMidiCCComponent(Pro800CCMessages::AMP_SUSTAIN, &slider_AmplifierSustain);
    setupMidiCCComponent(Pro800CCMessages::AMP_RELEASE, &slider_AmplifierRelease);

    group_Amplifier.addComponents({&group_AmplifierAttack, &group_AmplifierDecay, &group_AmplifierSustain, &group_AmplifierRelease});
    addAndMakeVisible(this->group_Amplifier);
}

void FrontPanelTab::setupGroupMaster()
{
    group_Master.setTextLabelPosition(juce::Justification::left);
    
    setupRotarySlider(slider_MasterTune, group_MasterTune);
    setupRotarySlider(slider_MasterVolume, group_MasterVolume);

    setupMidiCCComponent(Pro800CCMessages::MASTER_VOLUME, &slider_MasterVolume);
    setupMidiCCComponent(Pro800CCMessages::MASTER_TUNE, &slider_MasterTune);

    group_Master.addComponents({&group_MasterTune, &group_MasterVolume});
    addAndMakeVisible(this->group_Master);
}

void FrontPanelTab::setupRotarySlider(juce::Slider &slider, EqualSpacingGroupComponent &parent)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setRange(0.0, 127.0, 1.0);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 15);
    slider.setPopupDisplayEnabled(false, false, &parent);
    slider.setValue(1.0);
    
    parent.addComponent(&slider);
    parent.setInnerMargin(0);
}