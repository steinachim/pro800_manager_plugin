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
#include <memory>

class MidiHandler;

class Pro800ManagerEditor  : public juce::AudioProcessorEditor, public juce::MidiKeyboardState::Listener, public MidiComponent
{
public:
    Pro800ManagerEditor (MidiHandler *midiHandler, Pro800ManagerAudioProcessor&);
    ~Pro800ManagerEditor() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;

    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;

    void refreshMidiDeviceLists();
    void connectMidiDevices();

    void handlePro800VersionUpdate() override;

    
private:
    juce::TooltipWindow tooltipWindow { this, 500 };

    MidiHandler *midiHandler;
    
    juce::MidiKeyboardState keyboardState;

    juce::Label label_FirmwareVersion { "", "Not Connected" };
    juce::Label label_MidiChannel { "", "MIDI Channel:" };
    juce::Slider spinBox_MidiChannel { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft };
    juce::Label label_MidiInput { "", "MIDI Input:" };
    juce::ComboBox combo_MidiInputList;
    juce::Label label_MidiOutput { "", "MIDI Output:" };
    juce::ComboBox combo_MidiOutputList;
    juce::TextButton button_RefreshMidi { "Refresh" };
    juce::TextButton button_ConnectMidi { "Connect" };

    std::unique_ptr<MainWidget> tabBar;
    juce::MidiKeyboardComponent keyboardPanel { keyboardState, juce::MidiKeyboardComponent::Orientation::horizontalKeyboard };
    juce::TextButton button_ShowHideKeyboard { "Hide Keyboard" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pro800ManagerEditor)
};
