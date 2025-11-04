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

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "midi/VersionMessage.h"

#include <iostream>

Pro800ManagerEditor::Pro800ManagerEditor (MidiHandler *midiHandler, Pro800ManagerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    tabBar = new MainWidget(midiHandler);

    keyboardState.addListener(this);

    keyboardPanel.setKeyPressBaseOctave(3);
    keyboardPanel.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, getLookAndFeel().findColour(juce::Slider::backgroundColourId));
    keyboardPanel.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    keyboardPanel.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, getLookAndFeel().findColour(juce::Slider::thumbColourId));

    button_ShowHideKeyboard.setAlwaysOnTop(true);
    button_ShowHideKeyboard.onClick = [this] {
        keyboardPanel.setVisible (!keyboardPanel.isVisible() );
        button_ShowHideKeyboard.setButtonText( keyboardPanel.isVisible() ? "Hide Keyboard" : "Show Keyboard") ;
        resized();
    };

    addAndMakeVisible(button_ShowHideKeyboard);
    addAndMakeVisible(tabBar);
    addAndMakeVisible(keyboardPanel);
    
    setSize (1400, 900);
    setResizable(true, true);
}

Pro800ManagerEditor::~Pro800ManagerEditor()
{
    keyboardState.removeListener(this);
    delete tabBar;
    this->tabBar = nullptr;

}

void Pro800ManagerEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void Pro800ManagerEditor::resized()
{
    auto area = getLocalBounds().reduced(4);
    const double mainWindowPercentage = 9.0/10.0;

    int tabBarHeight = (int)(mainWindowPercentage * area.getHeight());

    if ( !this->keyboardPanel.isVisible() )
    {
        tabBarHeight = area.getHeight() - 30;
    }
        
    if ( this->tabBar != nullptr )
    {
        this->tabBar->setBounds( area.removeFromTop( tabBarHeight ));
    }

    this->keyboardPanel.setBounds(area.removeFromLeft( area.getWidth() - 100));

    this->button_ShowHideKeyboard.setBounds(area.removeFromBottom(30));
}

void Pro800ManagerEditor::handleNoteOn(juce::MidiKeyboardState* /*source*/, int midiChannel, int midiNoteNumber, float velocity)
{
    audioProcessor.sendMidiMessage(juce::MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
}


void Pro800ManagerEditor::handleNoteOff(juce::MidiKeyboardState* /*source*/, int midiChannel, int midiNoteNumber, float velocity)
{
    audioProcessor.sendMidiMessage(juce::MidiMessage::noteOff (midiChannel, midiNoteNumber, velocity));
}
    