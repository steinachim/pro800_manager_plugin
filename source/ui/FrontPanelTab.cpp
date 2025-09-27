/*
  ==============================================================================

    FrontPanel.cpp
    Created: 9 Sep 2025 11:11:39am
    Author:  Achim Stein

  ==============================================================================
*/

#include "FrontPanelTab.h"

#include "../midi/MidiHandler.h"
#include "UiHelpers.h"
#include "../Pro800Constants.h"

FrontPanelTab::FrontPanelTab(MidiHandler *midiHandler) : Component()
{
    this->group_OscillatorA = new GroupOscillatorA(midiHandler);
    this->group_OscillatorB = new GroupOscillatorB(midiHandler);    
    this->group_PolyMod = new GroupPolyMod(midiHandler);
    this->group_Noise = new GroupNoise(midiHandler);        
    this->group_LfoMod = new GroupLfoMod(midiHandler);
    this->group_Glide = new GroupGlide(midiHandler);
    this->group_Filter = new GroupFilter(midiHandler);
    this->group_Amplifier = new GroupAmplifier(midiHandler);
    this->group_Master = new GroupMaster(midiHandler);

    addAndMakeVisible(this->group_OscillatorA);
    addAndMakeVisible(this->group_OscillatorB);
    addAndMakeVisible(this->group_PolyMod);
    addAndMakeVisible(this->group_Noise);
    addAndMakeVisible(this->group_LfoMod);
    addAndMakeVisible(this->group_Glide);
    addAndMakeVisible(this->group_Filter);
    addAndMakeVisible(this->group_Amplifier);
    addAndMakeVisible(this->group_Master);

}

FrontPanelTab::~FrontPanelTab()
{
    delete this->group_OscillatorA;
    delete this->group_OscillatorB;
    delete this->group_PolyMod;
    delete this->group_Noise;
    delete this->group_LfoMod;
    delete this->group_Glide;
    delete this->group_Filter;
    delete this->group_Amplifier;
    delete this->group_Master;
}

void FrontPanelTab::resized()
{
    auto area = getLocalBounds().reduced(10);
    int groupHeight = area.getHeight() / 4;
    int groupWidth = area.getWidth()/2;
    
    auto leftArea = area.withTrimmedRight(groupWidth);
    auto rightArea = area.withTrimmedLeft(groupWidth);
    
    // left column
    this->group_OscillatorA->setBounds(leftArea.removeFromTop(groupHeight));
    this->group_OscillatorB->setBounds(leftArea.removeFromTop(groupHeight));
    
    this->group_PolyMod->setBounds(leftArea.withHeight(groupHeight).withRight(4*groupWidth/5));
    this->group_Noise->setBounds(leftArea.withHeight(groupHeight).withLeft(4*groupWidth/5));
    leftArea.removeFromTop(groupHeight);
    
    this->group_LfoMod->setBounds(leftArea.withHeight(groupHeight).withRight(4*groupWidth/5));
    this->group_Glide->setBounds(leftArea.withHeight(groupHeight).withLeft(4*groupWidth/5));
    leftArea.removeFromTop(groupHeight);
    
    // right column
    this->group_Filter->setBounds(rightArea.removeFromTop(2*groupHeight));
    this->group_Amplifier->setBounds(rightArea.removeFromTop(groupHeight));
    this->group_Master->setBounds(rightArea.removeFromTop(groupHeight));
}



GroupOscillatorA::GroupOscillatorA(MidiHandler *midiHandler) : EqualSpacingGroupComponent(), MidiComponent(midiHandler, true)
{
    setText("Oscillator A");
    setTextLabelPosition(juce::Justification::left);
    
    UiHelpers::setupRotarySlider(slider_Frequency, group_Frequency);
    UiHelpers::setupRotarySlider(slider_Level, group_Level);
    UiHelpers::setupRotarySlider(slider_PulseWidth, group_PulseWidth);
   
    checkBox_Sync.setButtonText("Enabled");
    group_Sync.addComponent(&checkBox_Sync);
    
    checkBox_ShapeSaw.setButtonText("Saw");
    checkBox_ShapeTri.setButtonText("Tri");
    checkBox_ShapeRect.setButtonText("Rect");
    group_Shape.addComponents({&checkBox_ShapeSaw, &checkBox_ShapeTri, &checkBox_ShapeRect});

    setupMidiCCComponent(Pro800CCMessages::OSC_A_FREQ, &slider_Frequency);
    setupMidiCCComponent(Pro800CCMessages::OSC_A_LEVEL, &slider_Level);
    setupMidiCCComponent(Pro800CCMessages::OSC_A_PULSE_WIDTH, &slider_PulseWidth);
    setupMidiCCComponent(Pro800CCMessages::OSC_A_SYNC, &checkBox_Sync);
    setupMidiCCComponent(Pro800CCMessages::OSC_A_SHAPE_SAW, &checkBox_ShapeSaw);
    setupMidiCCComponent(Pro800CCMessages::OSC_A_SHAPE_TRI, &checkBox_ShapeTri);
    setupMidiCCComponent(Pro800CCMessages::OSC_A_SHAPE_RECT, &checkBox_ShapeRect);

    addComponents({ &group_Frequency, &group_Sync, &group_Shape, &group_PulseWidth, &group_Level });
}

GroupOscillatorA::~GroupOscillatorA()
{
    removeMidiCCComponent(Pro800CCMessages::OSC_A_FREQ, &slider_Frequency);
    removeMidiCCComponent(Pro800CCMessages::OSC_A_LEVEL, &slider_Level);
    removeMidiCCComponent(Pro800CCMessages::OSC_A_PULSE_WIDTH, &slider_PulseWidth);
    removeMidiCCComponent(Pro800CCMessages::OSC_A_SYNC, &checkBox_Sync);
    removeMidiCCComponent(Pro800CCMessages::OSC_A_SHAPE_SAW, &checkBox_ShapeSaw);
    removeMidiCCComponent(Pro800CCMessages::OSC_A_SHAPE_TRI, &checkBox_ShapeTri);
    removeMidiCCComponent(Pro800CCMessages::OSC_A_SHAPE_RECT, &checkBox_ShapeRect);
}

GroupOscillatorB::GroupOscillatorB(MidiHandler *midiHandler) : EqualSpacingGroupComponent(), MidiComponent(midiHandler, true)
{
    setText("Oscillator B");
    setTextLabelPosition(juce::Justification::left);
    
    UiHelpers::setupRotarySlider(slider_Frequency, group_Frequency);
    UiHelpers::setupRotarySlider(slider_Fine, group_Fine);
    UiHelpers::setupRotarySlider(slider_PulseWidth, group_PulseWidth);
    UiHelpers::setupRotarySlider(slider_Level, group_Level);
    
    checkBox_ShapeSaw.setButtonText("Saw");
    checkBox_ShapeTri.setButtonText("Tri");
    checkBox_ShapeRect.setButtonText("Rect");
    group_Shape.addComponents({&checkBox_ShapeSaw, &checkBox_ShapeTri, &checkBox_ShapeRect});

    setupMidiCCComponent(Pro800CCMessages::OSC_B_FREQ, &slider_Frequency);
    setupMidiCCComponent(Pro800CCMessages::OSC_B_FINE, &slider_Fine);
    setupMidiCCComponent(Pro800CCMessages::OSC_B_PULSE_WIDTH, &slider_PulseWidth);
    setupMidiCCComponent(Pro800CCMessages::OSC_B_LEVEL, &slider_Level);
    
    setupMidiCCComponent(Pro800CCMessages::OSC_B_SHAPE_SAW, &checkBox_ShapeSaw);
    setupMidiCCComponent(Pro800CCMessages::OSC_B_SHAPE_TRI, &checkBox_ShapeTri);
    setupMidiCCComponent(Pro800CCMessages::OSC_B_SHAPE_RECT, &checkBox_ShapeRect);
    
    addComponents({ &group_Frequency, &group_Fine, &group_Shape, &group_PulseWidth, &group_Level});
}

GroupOscillatorB::~GroupOscillatorB()
{
    removeMidiCCComponent(Pro800CCMessages::OSC_B_FREQ, &slider_Frequency);
    removeMidiCCComponent(Pro800CCMessages::OSC_B_FINE, &slider_Fine);
    removeMidiCCComponent(Pro800CCMessages::OSC_B_PULSE_WIDTH, &slider_PulseWidth);
    removeMidiCCComponent(Pro800CCMessages::OSC_B_LEVEL, &slider_Level);
    
    removeMidiCCComponent(Pro800CCMessages::OSC_B_SHAPE_SAW, &checkBox_ShapeSaw);
    removeMidiCCComponent(Pro800CCMessages::OSC_B_SHAPE_TRI, &checkBox_ShapeTri);
    removeMidiCCComponent(Pro800CCMessages::OSC_B_SHAPE_RECT, &checkBox_ShapeRect);
}

GroupPolyMod::GroupPolyMod(MidiHandler *midiHandler) : EqualSpacingGroupComponent(), MidiComponent(midiHandler, true)
{
    setText("Poly Mod");
    setTextLabelPosition(juce::Justification::left);
    
    UiHelpers::setupRotarySlider(slider_SourceFilterEnv, group_SourceFilterEnv);
    UiHelpers::setupRotarySlider(slider_SourceOscB, group_SourceOscB);
    group_SourceAmount.addComponents({&group_SourceFilterEnv, &group_SourceOscB});

    checkBox_DestFreqA.setButtonText("Freq A");
    checkBox_DestFilter.setButtonText("Filter");
    group_Destination.addComponents({&checkBox_DestFreqA, &checkBox_DestFilter});
    
    checkBox_UnisonTrack.setButtonText("Enable");
    group_UnisonTrack.addComponent(&checkBox_UnisonTrack);

    setupMidiCCComponent(Pro800CCMessages::POLY_MOD_SOURCE_FILTER_ENV, &slider_SourceFilterEnv);
    setupMidiCCComponent(Pro800CCMessages::POLY_MOD_SOURCE_OSC_B, &slider_SourceOscB);
    setupMidiCCComponent(Pro800CCMessages::POLY_MOD_DEST_FREQ_A, &checkBox_DestFreqA);
    setupMidiCCComponent(Pro800CCMessages::POLY_MOD_DEST_FILTER, &checkBox_DestFilter);
    setupMidiCCComponent(Pro800CCMessages::POLY_MOD_UNISON_TRACK, &checkBox_UnisonTrack);
    
    addComponent(&group_SourceAmount, 2.0);
    addComponents({&group_Destination, &group_UnisonTrack});
}

GroupPolyMod::~GroupPolyMod()
{
    removeMidiCCComponent(Pro800CCMessages::POLY_MOD_SOURCE_FILTER_ENV, &slider_SourceFilterEnv);
    removeMidiCCComponent(Pro800CCMessages::POLY_MOD_SOURCE_OSC_B, &slider_SourceOscB);
    removeMidiCCComponent(Pro800CCMessages::POLY_MOD_DEST_FREQ_A, &checkBox_DestFreqA);
    removeMidiCCComponent(Pro800CCMessages::POLY_MOD_DEST_FILTER, &checkBox_DestFilter);
    removeMidiCCComponent(Pro800CCMessages::POLY_MOD_UNISON_TRACK, &checkBox_UnisonTrack);
}

GroupNoise::GroupNoise(MidiHandler *midiHandler) : EqualSpacingGroupComponent(), MidiComponent(midiHandler, true)
{
    setText("Noise");
    setTextLabelPosition(juce::Justification::left);
    
    UiHelpers::setupRotarySlider(slider_NoiseLevel, group_NoiseLevel);

    setupMidiCCComponent(Pro800CCMessages::NOISE_LEVEL, &slider_NoiseLevel);
    addComponent(&group_NoiseLevel);
}

GroupNoise::~GroupNoise()
{
    removeMidiCCComponent(Pro800CCMessages::NOISE_LEVEL, &slider_NoiseLevel);
}

GroupLfoMod::GroupLfoMod(MidiHandler *midiHandler) : EqualSpacingGroupComponent(), MidiComponent(midiHandler, true)
{
    setText("LFO Mod");
    setTextLabelPosition(juce::Justification::left);
    
    UiHelpers::setupRotarySlider(slider_Frequency, group_Frequency);
    UiHelpers::setupRotarySlider(slider_InitialAmount, group_InitialAmount);

    checkBox_DestFreqAB.setButtonText("Freq A-B");
    checkBox_DestPulseWidthAB.setButtonText("PW A-B");
    checkBox_DestFilter.setButtonText("Filter");
    group_Destination.addComponents({&checkBox_DestFreqAB, &checkBox_DestPulseWidthAB, &checkBox_DestFilter});

    comboBox_Shape.addItem("Sine", Pro800LFOShape::LFO_SHAPE_SINE+1);
    comboBox_Shape.addItem("Triangle", Pro800LFOShape::LFO_SHAPE_TRIANGLE+1);   
    comboBox_Shape.addItem("Saw", Pro800LFOShape::LFO_SHAPE_SAW+1);
    comboBox_Shape.addItem("Pulse", Pro800LFOShape::LFO_SHAPE_PULSE+1);
    comboBox_Shape.addItem("Random", Pro800LFOShape::LFO_SHAPE_RANDOM+1);
    comboBox_Shape.addItem("Noise", Pro800LFOShape::LFO_SHAPE_NOISE+1);
    group_Shape.addComponent(&comboBox_Shape);

    setupMidiCCComponent(Pro800CCMessages::LFO_MOD_FREQ, &slider_Frequency);
    setupMidiCCComponent(Pro800CCMessages::LFO_MOD_INITIAL_AMOUNT, &slider_InitialAmount);
    setupMidiCCComponent(Pro800CCMessages::LFO_MOD_DEST_FREQ_AB, &checkBox_DestFreqAB);
    setupMidiCCComponent(Pro800CCMessages::LFO_MOD_DEST_PW_AB, &checkBox_DestPulseWidthAB);
    setupMidiCCComponent(Pro800CCMessages::LFO_MOD_DEST_FILTER, &checkBox_DestFilter);
    setupMidiCCComponent(Pro800CCMessages::LFO_MOD_SHAPE, &comboBox_Shape);

    addComponents({&group_Frequency, &group_Shape, &group_InitialAmount, &group_Destination});
}

GroupLfoMod::~GroupLfoMod()
{
    removeMidiCCComponent(Pro800CCMessages::LFO_MOD_FREQ, &slider_Frequency);
    removeMidiCCComponent(Pro800CCMessages::LFO_MOD_INITIAL_AMOUNT, &slider_InitialAmount);
    removeMidiCCComponent(Pro800CCMessages::LFO_MOD_DEST_FREQ_AB, &checkBox_DestFreqAB);
    removeMidiCCComponent(Pro800CCMessages::LFO_MOD_DEST_PW_AB, &checkBox_DestPulseWidthAB);
    removeMidiCCComponent(Pro800CCMessages::LFO_MOD_DEST_FILTER, &checkBox_DestFilter);
    removeMidiCCComponent(Pro800CCMessages::LFO_MOD_SHAPE, &comboBox_Shape);
}

GroupGlide::GroupGlide(MidiHandler *midiHandler) : EqualSpacingGroupComponent(), MidiComponent(midiHandler, true)
{
    setText("Glide");
    setTextLabelPosition(juce::Justification::left);
    
    UiHelpers::setupRotarySlider(slider_GlideAmount, group_GlideAmount);

    setupMidiCCComponent(Pro800CCMessages::GLIDE_TIME, &slider_GlideAmount);
    addComponent(&group_GlideAmount);
}

GroupGlide::~GroupGlide()
{
    removeMidiCCComponent(Pro800CCMessages::GLIDE_TIME, &slider_GlideAmount);
}

GroupFilter::GroupFilter(MidiHandler *midiHandler) : juce::GroupComponent(), MidiComponent(midiHandler, true)
{
    setText("Filter");
    setTextLabelPosition(juce::Justification::left);

    UiHelpers::setupRotarySlider(slider_Cutoff, group_Cutoff);
    UiHelpers::setupRotarySlider(slider_Resonance, group_Resonance);
    UiHelpers::setupRotarySlider(slider_EnvAmount, group_EnvAmount);

    UiHelpers::setupRotarySlider(slider_Attack, group_Attack);
    UiHelpers::setupRotarySlider(slider_Decay, group_Decay);
    UiHelpers::setupRotarySlider(slider_Sustain, group_Sustain);
    UiHelpers::setupRotarySlider(slider_Release, group_Release);
    
    radio_TrackingFull.setButtonText("Full");
    radio_TrackingFull.setRadioGroupId(1000);
    radio_TrackingHalf.setButtonText("1/2");
    radio_TrackingHalf.setRadioGroupId(1000);
    radio_TrackingOff.setButtonText("Off");
    radio_TrackingOff.setRadioGroupId(1000);
    group_KeyboardTracking.addComponents({&radio_TrackingFull, &radio_TrackingHalf, &radio_TrackingOff});
    
    addAndMakeVisible(&group_Cutoff);
    addAndMakeVisible(&group_Resonance);
    addAndMakeVisible(&group_EnvAmount);
    addAndMakeVisible(&group_Attack);
    addAndMakeVisible(&group_Decay);
    addAndMakeVisible(&group_Sustain);
    addAndMakeVisible(&group_Release);
    addAndMakeVisible(&group_KeyboardTracking);

    setupMidiCCComponent(Pro800CCMessages::FILTER_CUTOFF, &slider_Cutoff);
    setupMidiCCComponent(Pro800CCMessages::FILTER_RESONANCE, &slider_Resonance);
    setupMidiCCComponent(Pro800CCMessages::FILTER_ENV_AMOUNT, &slider_EnvAmount);
    setupMidiCCComponent(Pro800CCMessages::FILTER_ATTACK, &slider_Attack);
    setupMidiCCComponent(Pro800CCMessages::FILTER_DECAY, &slider_Decay);
    setupMidiCCComponent(Pro800CCMessages::FILTER_SUSTAIN, &slider_Sustain);
    setupMidiCCComponent(Pro800CCMessages::FILTER_RELEASE, &slider_Release);

    setupMidiCCComponent(Pro800CCMessages::KEYBOARD_TRACKING, &radio_TrackingFull); // TODO: FIX
    setupMidiCCComponent(Pro800CCMessages::KEYBOARD_TRACKING, &radio_TrackingHalf);
    setupMidiCCComponent(Pro800CCMessages::KEYBOARD_TRACKING, &radio_TrackingOff);
}

GroupFilter::~GroupFilter()
{
    removeMidiCCComponent(Pro800CCMessages::FILTER_CUTOFF, &slider_Cutoff);
    removeMidiCCComponent(Pro800CCMessages::FILTER_RESONANCE, &slider_Resonance);
    removeMidiCCComponent(Pro800CCMessages::FILTER_ENV_AMOUNT, &slider_EnvAmount);
    removeMidiCCComponent(Pro800CCMessages::FILTER_ATTACK, &slider_Attack);
    removeMidiCCComponent(Pro800CCMessages::FILTER_DECAY, &slider_Decay);
    removeMidiCCComponent(Pro800CCMessages::FILTER_SUSTAIN, &slider_Sustain);
    removeMidiCCComponent(Pro800CCMessages::FILTER_RELEASE, &slider_Release);

    removeMidiCCComponent(Pro800CCMessages::KEYBOARD_TRACKING, &radio_TrackingFull);
    removeMidiCCComponent(Pro800CCMessages::KEYBOARD_TRACKING, &radio_TrackingHalf);
    removeMidiCCComponent(Pro800CCMessages::KEYBOARD_TRACKING, &radio_TrackingOff);  
}

void GroupFilter::resized()
{
    auto area = getLocalBounds().reduced(15);

    juce::Grid grid;
    
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    grid.templateRows = { Track (Fr (1)), Track (Fr (1)) };
    grid.templateColumns = { Track (Fr (1)), Track (Fr (1)), Track (Fr (1)), Track (Fr (1)) };
    
    grid.items = { juce::GridItem(group_Cutoff), juce::GridItem(group_Resonance), juce::GridItem(group_EnvAmount), juce::GridItem(group_KeyboardTracking),
                   juce::GridItem(group_Attack), juce::GridItem(group_Decay), juce::GridItem(group_Sustain), juce::GridItem(group_Release)};
    
    grid.performLayout(area);
}

GroupAmplifier::GroupAmplifier(MidiHandler *midiHandler) : EqualSpacingGroupComponent(), MidiComponent(midiHandler, true)
{
    setText("Amplifier");
    setTextLabelPosition(juce::Justification::left);
    
    UiHelpers::setupRotarySlider(slider_Attack, group_Attack);
    UiHelpers::setupRotarySlider(slider_Decay, group_Decay);
    UiHelpers::setupRotarySlider(slider_Sustain, group_Sustain);
    UiHelpers::setupRotarySlider(slider_Release, group_Release);

    setupMidiCCComponent(Pro800CCMessages::AMP_ATTACK, &slider_Attack);
    setupMidiCCComponent(Pro800CCMessages::AMP_DECAY, &slider_Decay);
    setupMidiCCComponent(Pro800CCMessages::AMP_SUSTAIN, &slider_Sustain);
    setupMidiCCComponent(Pro800CCMessages::AMP_RELEASE, &slider_Release);

    addComponents({&group_Attack, &group_Decay, &group_Sustain, &group_Release});
}

GroupAmplifier::~GroupAmplifier()
{
    removeMidiCCComponent(Pro800CCMessages::AMP_ATTACK, &slider_Attack);
    removeMidiCCComponent(Pro800CCMessages::AMP_DECAY, &slider_Decay);
    removeMidiCCComponent(Pro800CCMessages::AMP_SUSTAIN, &slider_Sustain);
    removeMidiCCComponent(Pro800CCMessages::AMP_RELEASE, &slider_Release);
}

GroupMaster::GroupMaster(MidiHandler *midiHandler) : EqualSpacingGroupComponent(), MidiComponent(midiHandler, true)
{
    setText("Master");
    setTextLabelPosition(juce::Justification::left);
    
    UiHelpers::setupRotarySlider(slider_MasterTune, group_MasterTune);
    UiHelpers::setupRotarySlider(slider_MasterVolume, group_MasterVolume);

    setupMidiCCComponent(Pro800CCMessages::MASTER_VOLUME, &slider_MasterVolume);
    setupMidiCCComponent(Pro800CCMessages::MASTER_TUNE, &slider_MasterTune);

    addComponents({&group_MasterTune, &group_MasterVolume});
}

GroupMaster::~GroupMaster()
{
    removeMidiCCComponent(Pro800CCMessages::MASTER_VOLUME, &slider_MasterVolume);
    removeMidiCCComponent(Pro800CCMessages::MASTER_TUNE, &slider_MasterTune);
}
