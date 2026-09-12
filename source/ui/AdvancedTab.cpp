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

#include "AdvancedTab.h"
#include "../midi/MidiHandler.h"
#include "../midi/Pro800MessageFactory.h"

AdvancedTab::AdvancedTab (MidiHandler* midiHandler) : Component(), MidiComponent (midiHandler, false, { MessageType::MIDI_LOG })
{
    juce::String comboToolTip = "";
    for (const auto& [id, preparedMessage] : PREPARED_MESSAGES)
    {
        combo_PreparedMessages.addItem (preparedMessage.name, (int) id);
        comboToolTip += preparedMessage.name + ":\n" + preparedMessage.description + "\n\n";
    }
    combo_PreparedMessages.setTooltip (comboToolTip.trimEnd());
    combo_PreparedMessages.onChange = [this]() {
        PreparedMessageId selectedId = (PreparedMessageId) combo_PreparedMessages.getSelectedId();
        if (PREPARED_MESSAGES.contains (selectedId))
        {
            textEdit_inputMidiMessage.setText (PREPARED_MESSAGES.at (selectedId).byteString, false);
        }
    };

    codeEditor_midiMessageLog.setReadOnly (true); // selecting/copying still works
    codeEditor_midiMessageLog.setLineNumbersShown (false);
    codeEditor_midiMessageLog.setScrollbarThickness (12);

    button_sendMessage.setButtonText ("Send");
    button_sendMessage.onClick = [this] { sendInputMessage(); };

#if JUCE_DEBUG
    // clang-format off
    slider_debugInput.textFromValueFunction = [](double value)             { return juce::String::formatted("0x%02x", (uint8_t)value); };
    slider_debugInput.valueFromTextFunction = [](const juce::String &text) { return (double)text.getHexValue32(); };
    // clang-format on
    slider_debugInput.setRange (0.0, 255.0, 1.0);

    button_debug.setButtonText ("Debug");
    button_debug.onClick = [this] {
        juce::String prefix = "f0 00 20 32 00 01 24 00 77";
        juce::String postfix = "03 f7";

        int currentTestNum = (int) slider_debugInput.getValue();

        this->textEdit_inputMidiMessage.setText (prefix + juce::String::formatted (" %02x ", currentTestNum) + postfix, false);
        slider_debugInput.setValue (currentTestNum + 1);
    };

    addAndMakeVisible (button_debug);
    addAndMakeVisible (slider_debugInput);
#endif

    button_clearLog.onClick = [this] {
        logDocument.replaceAllContent ({});
    };

    addAndMakeVisible (codeEditor_midiMessageLog);

    addAndMakeVisible (combo_PreparedMessages);
    addAndMakeVisible (textEdit_inputMidiMessage);
    addAndMakeVisible (button_sendMessage);
    addAndMakeVisible (checkBox_enableLogging);
    addAndMakeVisible (button_clearLog);
}

void AdvancedTab::sendInputMessage()
{
    const juce::String input = textEdit_inputMidiMessage.getText();

    juce::StringArray tokens;
    tokens.addTokens (input, false);
    tokens.removeEmptyStrings();

    if (tokens.isEmpty())
    {
        addLogMessage ("Nothing to send: enter the message as hex bytes, e.g. 'F0 00 20 32 00 01 24 00 08 00 F7'");
        return;
    }

    std::vector<uint8_t> bytes;
    bytes.reserve ((size_t) tokens.size());
    for (const auto& token : tokens)
    {
        // one byte = one or two hex digits, nothing else
        if (token.length() > 2 || !token.containsOnly ("0123456789abcdefABCDEF"))
        {
            addLogMessage ("Invalid input: '" + token + "' is not a hex byte");
            return;
        }

        bytes.push_back ((uint8_t) token.getHexValue32());
    }

    if (bytes.front() == 0xF0 && bytes.back() != 0xF7)
    {
        addLogMessage ("Invalid input: SysEx message must end with F7");
        return;
    }

    sendMidiMessage (juce::MidiMessage (bytes.data(), (int) bytes.size()));
}

AdvancedTab::~AdvancedTab()
{
}

void AdvancedTab::resized()
{
    const int buttonHeight = 30;
    auto area = getLocalBounds().reduced (4);

    auto logTopArea = area.removeFromTop (buttonHeight).reduced (4);
    button_clearLog.setBounds (logTopArea.removeFromRight (100));
    checkBox_enableLogging.setBounds (logTopArea);
    codeEditor_midiMessageLog.setBounds (area.removeFromTop (area.getHeight() - buttonHeight).reduced (4));

#if JUCE_DEBUG
    slider_debugInput.setBounds (area.removeFromRight (150).reduced (4));
    button_debug.setBounds (area.removeFromRight (100).reduced (4));
#endif
    button_sendMessage.setBounds (area.removeFromRight (100).reduced (4));
    combo_PreparedMessages.setBounds (area.removeFromLeft (200).reduced (4));
    textEdit_inputMidiMessage.setBounds (area.reduced (4));
}

void AdvancedTab::handleMidiLog (const juce::MidiMessage& message, const juce::String& logPrefix)
{
    if (!checkBox_enableLogging.getToggleState())
        return;

    // always show the raw bytes (the ground truth when reverse-engineering the protocol),
    // then what we made of them
    juce::String messageText = message.isSysEx() ? juce::String::toHexString (message.getRawData(), message.getRawDataSize())
                                                 : message.getDescription();

    if (auto pro800Message = Pro800MessageFactory::createMidiMessage (message))
    {
        messageText << "\n"
                    << pro800Message->toString();
    }

    addLogMessage (logPrefix + " " + messageText);
}

void AdvancedTab::addLogMessage (const juce::String& message)
{
    logDocument.insertText (logDocument.getNumCharacters(), message + "\n\n");

    const int excessLines = logDocument.getNumLines() - MAX_LOG_LINES;
    if (excessLines > 0)
    {
        // drop the oldest lines
        logDocument.deleteSection (juce::CodeDocument::Position (logDocument, 0, 0),
            juce::CodeDocument::Position (logDocument, excessLines, 0));
    }

    // follow the newest entry
    codeEditor_midiMessageLog.moveCaretToEnd (false);
    codeEditor_midiMessageLog.scrollToKeepCaretOnScreen();
}