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

AdvancedTab::AdvancedTab(MidiHandler *midiHandler) : Component(), MidiComponent(midiHandler, false, {MessageType::MIDI_LOG_MESSAGE})
{
    textEdit_midiMessageLog.setReadOnly(false);
    textEdit_midiMessageLog.setMultiLine(true);
    textEdit_midiMessageLog.setReturnKeyStartsNewLine(true);

    juce::String fixedWidthFont = juce::Font::getDefaultMonospacedFontName();
    textEdit_midiMessageLog.setFont( { juce::FontOptions().withName(fixedWidthFont) } );

    button_sendMessage.setButtonText("Send");
    button_sendMessage.onClick = [this, midiHandler] {
      juce::String message = textEdit_inputMidiMessage.getText();

      juce::StringArray bytes;
      bytes.addTokens(message, false);

      juce::Array<uint8_t> midiMessage;
      for( const auto &byte : bytes )
      {
        if ( byte.length() > 2 )
        {
          addLogMessage("Invalid input: " + message);
          return;
        }

        midiMessage.add((uint8_t)byte.getHexValue32());
      }

      juce::MidiMessage messageToSend(midiMessage.data(), midiMessage.size());
      midiHandler->sendMidiMessage(messageToSend);
    };

    slider_debugInput.textFromValueFunction = [](double value)             { return juce::String::formatted("0x%02x", (uint8_t)value); };
    slider_debugInput.valueFromTextFunction = [](const juce::String &text) { return (double)text.getHexValue32(); };
    slider_debugInput.setRange(0.0, 255.0, 1.0);

    button_debug.setButtonText("Debug");
    button_debug.onClick = [this]
    {

      juce::String prefix = "f0 00 20 32 00 01 24 00 77";
      juce::String postfix = "03 f7";

      int currentTestNum = (int)slider_debugInput.getValue();

      this->textEdit_inputMidiMessage.setText( prefix + juce::String::formatted(" %02x ", currentTestNum) + postfix, false);      
      slider_debugInput.setValue(currentTestNum+1);
    };

    addAndMakeVisible(textEdit_midiMessageLog);
    addAndMakeVisible(textEdit_inputMidiMessage);
    addAndMakeVisible(button_sendMessage);
    addAndMakeVisible(button_debug);
    addAndMakeVisible(slider_debugInput);
    addAndMakeVisible(checkBox_enableLogging);
}

AdvancedTab::~AdvancedTab()
{
}

void AdvancedTab::resized()
{
    const int buttonHeight = 30;
    auto area = getLocalBounds().reduced(4);
    checkBox_enableLogging.setBounds(area.removeFromTop(buttonHeight).reduced(4));
    textEdit_midiMessageLog.setBounds(area.removeFromTop(area.getHeight()-buttonHeight).reduced(4));
    
    slider_debugInput.setBounds(area.removeFromRight(150).reduced(4));
    button_debug.setBounds(area.removeFromRight(100).reduced(4));
    button_sendMessage.setBounds(area.removeFromRight(100).reduced(4));
    textEdit_inputMidiMessage.setBounds(area.reduced(4));
}

void AdvancedTab::handleMidiLog (const juce::MidiMessage& message, const juce::String& logPrefix)
{
    if ( !checkBox_enableLogging.getToggleState() )
        return;

    juce::String messageText = message.getDescription();
    std::shared_ptr<Pro800MidiMessage> pro800Message = Pro800MessageFactory::createMidiMessage (message);

    if (pro800Message)
    {
        messageText = pro800Message->toString();
    }

    addLogMessage(logPrefix + " " + messageText);
}

void AdvancedTab::addLogMessage(const juce::String &message)
{
    textEdit_midiMessageLog.moveCaretToEnd();
    textEdit_midiMessageLog.insertTextAtCaret(message + "\n\n");
}