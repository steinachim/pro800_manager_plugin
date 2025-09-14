/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <iostream>

//==============================================================================
Pro800ManagerEditor::Pro800ManagerEditor (MidiHandler *midiHandler, Pro800ManagerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    tabBar = new MainWidget(midiHandler);
    addAndMakeVisible(tabBar);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1200, 700);
    setResizable(true, true);
}

Pro800ManagerEditor::~Pro800ManagerEditor()
{
    delete tabBar;
    this->tabBar = nullptr;
}

//==============================================================================
void Pro800ManagerEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void Pro800ManagerEditor::resized()
{
    if ( this->tabBar != nullptr )
    {
        this->tabBar->setBounds (getLocalBounds().reduced (4));
    }
}
