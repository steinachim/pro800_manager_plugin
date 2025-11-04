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

        juce::TextEditor textEdit_inputMidiMessage;
        juce::TextEditor textEdit_midiMessageLog;

        juce::TextButton button_sendMessage;
        juce::TextButton button_debug;

        juce::ToggleButton checkBox_enableLogging { "Enable Logging" };

        juce::Slider slider_debugInput { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdvancedTab)
};
