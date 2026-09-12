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
#include "session/SynthSession.h"
#include "ui/MainWidget.h"
#include "ui/MidiDeviceComboBox.h"
#include "ui/StatusStrip.h"
#include <memory>

class MidiHandler;

class Pro800ManagerEditor : public juce::AudioProcessorEditor, public juce::MidiKeyboardState::Listener, private SynthSession::Listener
{
public:
    Pro800ManagerEditor (MidiHandler* midiHandler, Pro800ManagerAudioProcessor&);
    ~Pro800ManagerEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void handleNoteOn (juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;

private:
    /** Combo box item ids: this one is "Auto", 1-16 are the channels. */
    static constexpr int CHANNEL_ITEM_AUTO = 100;
    static constexpr int CHANNEL_ITEM_ALL = 101;

    /** The synth's USB-MIDI port is "BEHRINGER PRO 800" (the exact spelling varies by OS and hub). */
    static inline const juce::String PRO800_PORT_NAME_FRAGMENT { "PRO 800" };

    void refreshMidiDeviceLists();
    void connectMidiDevices();

    // SynthSession::Listener: connection badge, channel combo, keyboard
    void synthSessionChanged() override;
    void warnAboutUnsupportedFirmware();

    juce::String warnedFirmwareVersion; // the unsupported version already shown in a dialog

    // both owned by the processor, which outlives the editor
    MidiHandler& midiHandler;
    SynthSession& synthSession;

    juce::TooltipWindow tooltipWindow { this, 500 };

    juce::MidiKeyboardState keyboardState;

    juce::Label label_Connection { "", "Not connected" };
    juce::Label label_MidiChannel { "", "Send:" };
    juce::ComboBox combo_MidiChannel;
    juce::Label label_ReceiveChannel { "", "Receive:" };
    juce::ComboBox combo_ReceiveChannel;
    juce::Label label_MidiInput { "", "In:" };
    MidiDeviceComboBox combo_MidiInputList;
    juce::Label label_MidiOutput { "", "Out:" };
    MidiDeviceComboBox combo_MidiOutputList;
    juce::TextButton button_RefreshMidi { "Refresh" };
    juce::TextButton button_ConnectMidi { "Connect" };

    StatusStrip statusStrip;
    std::unique_ptr<MainWidget> tabBar;
    juce::MidiKeyboardComponent keyboardPanel { keyboardState, juce::MidiKeyboardComponent::Orientation::horizontalKeyboard };
    juce::TextButton button_ShowHideKeyboard { "Hide Keyboard" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pro800ManagerEditor)
};
