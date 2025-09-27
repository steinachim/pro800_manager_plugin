/*
  ==============================================================================

    AdvancedTab.cpp
    Created: 9 Sep 2025 11:12:00am
    Author:  Achim Stein

  ==============================================================================
*/

#include "AdvancedTab.h"
#include "UiHelpers.h"
#include "../midi/MidiHandler.h"

AdvancedTab::AdvancedTab(MidiHandler *midiHandler) : Component(), MidiComponent(midiHandler)
{
    textEdit_midiMessageLog.setReadOnly(false);
    textEdit_midiMessageLog.setMultiLine(true);
    textEdit_midiMessageLog.setReturnKeyStartsNewLine(true);

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
          addMidiMessage("Invalid input: " + message);
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

      juce::String prefix = "f0 00 20 32 00 01 24 00";
      juce::String postfix = "00 f7";

      int currentTestNum = (int)slider_debugInput.getValue();

      this->textEdit_inputMidiMessage.setText( prefix + juce::String::formatted(" %02x ", currentTestNum) + postfix, false);      
      slider_debugInput.setValue(currentTestNum+1);
    };

    addAndMakeVisible(textEdit_midiMessageLog);
    addAndMakeVisible(textEdit_inputMidiMessage);
    addAndMakeVisible(button_sendMessage);
    addAndMakeVisible(button_debug);
    addAndMakeVisible(slider_debugInput);

    setupMidiLogComponent(&textEdit_midiMessageLog);
}

AdvancedTab::~AdvancedTab()
{
  removeMidiLogComponent(&textEdit_midiMessageLog);
}

void AdvancedTab::resized()
{
    juce::FlexBox input;
    input.flexDirection = juce::FlexBox::Direction::row;
    input.items = {
        juce::FlexItem(textEdit_inputMidiMessage).withFlex(1.0f),
        juce::FlexItem(button_sendMessage).withFlex(0.1f),
        juce::FlexItem(button_debug).withFlex(0.1f),
        juce::FlexItem(slider_debugInput).withFlex(0.1f)
    };

    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.items = {
        juce::FlexItem(textEdit_midiMessageLog).withFlex(1.0f),
        juce::FlexItem(input).withFlex(0.0f).withMinHeight(30)
    };
    
    fb.performLayout(getLocalBounds().reduced(10));
}

void AdvancedTab::addMidiMessage(const juce::String &message)
{
    textEdit_midiMessageLog.moveCaretToEnd();
    textEdit_midiMessageLog.insertTextAtCaret(message + "\n\n");
}