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

#pragma once

#include "juce_audio_utils/juce_audio_utils.h"

#include "PluginProcessor.h"
#include "ui/MainWidget.h"

class MidiHandler;

class Pro800ManagerEditor  : public juce::AudioProcessorEditor, public juce::MidiKeyboardState::Listener
{
public:
    Pro800ManagerEditor (MidiHandler *midiHandler, Pro800ManagerAudioProcessor&);
    ~Pro800ManagerEditor() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;

    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    
private:
    Pro800ManagerAudioProcessor& audioProcessor;
    
    juce::MidiKeyboardState keyboardState;

    MainWidget *tabBar;
    juce::MidiKeyboardComponent keyboardPanel { keyboardState, juce::MidiKeyboardComponent::Orientation::horizontalKeyboard };
    juce::TextButton button_ShowHideKeyboard { "Hide Keyboard" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pro800ManagerEditor)
};
