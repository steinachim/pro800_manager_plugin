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

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiComponent.h"

class MidiHandler;

class AdvancedTab : public juce::Component, public MidiComponent
{
    public:
        AdvancedTab(MidiHandler *parent);
        virtual ~AdvancedTab() override;

        void resized() override;
        void handleMidiLog(const juce::MidiMessage &message, const juce::String &logPrefix) override;

    private:
        void addLogMessage(const juce::String &newMessage);

        juce::ComboBox combo_PreparedMessages;
        juce::TextEditor textEdit_inputMidiMessage;
        juce::TextEditor textEdit_midiMessageLog;

        juce::TextButton button_sendMessage;
        juce::TextButton button_debug;

        juce::ToggleButton checkBox_enableLogging { "Enable Logging" };
        juce::TextButton button_clearLog { "Clear Log" };

        juce::Slider slider_debugInput { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

        enum PreparedMessageId
        {
            Custom = 1,
            DumpProgram,
            RequestVersion,
            GetSettings,
            PressButton,
            NoteOn,
            NoteOff
        };

        struct PreparedMessage
        {
            juce::String name;
            juce::String byteString;
            juce::String description;
        };

        const std::map<PreparedMessageId, PreparedMessage> PREPARED_MESSAGES = {
            {Custom,         {"Custom", "", "Enter your own command"}},
            {DumpProgram,    {"Dump Program", "F0 00 20 32 00 01 24 00 77 XX XX F7", "Replace XX XX with the program number"}},
            {RequestVersion, {"Request Version", "F0 00 20 32 00 01 24 00 08 00 F7", "Report program version, also checks compatibility"}},
            {GetSettings,    {"Get Settings", "F0 00 20 32 00 01 24 00 77 7E 03 F7", "Get global system status"}},
            {PressButton,    {"Press Button", "F0 00 20 32 00 01 24 00 71 XX F7", "Emulate button press: \n00-09 = 0-9\n0C = PRESET\n0D = REC\n0E = PERF\n0F = SETTINGS\n10 = SEQ1\n11 = SEQ2\n13 = SYNC CLOCK\n14 = SYNC SOURCE"}},
            {NoteOn,         {"Note On", "9[channel] KK VV", "Send note-on"}},
            {NoteOff,        {"Note Off", "8[channel] KK VV", "Send note-off"}}
        };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdvancedTab)
};
