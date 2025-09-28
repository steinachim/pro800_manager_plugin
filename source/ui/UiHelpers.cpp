/*
  ==============================================================================

    UiHelpers.cpp
    Created: 13 Sep 2025 9:26:33am
    Author:  Achim Stein

  ==============================================================================
*/

#include "UiHelpers.h"

#include "EqualSpacingGroupComponent.h"
#include "../midi/MidiHandler.h"
#include "../midi/Pro800MidiMessage.h"
#include "../midi/Pro800MessageFactory.h"



void UiHelpers::setComponentCCValue(juce::Component *component, uint8_t midiCC, uint8_t value)
{
    if (juce::Slider* slider = dynamic_cast<juce::Slider*> (component))
    {
        slider->setValue ((double) value / 127.0 * (slider->getMaximum() - slider->getMinimum()) + slider->getMinimum(), juce::dontSendNotification);
    }
    else if (juce::Button* button = dynamic_cast<juce::Button*> (component))
    {
        button->setToggleState (value > 63, juce::dontSendNotification);
    }
    else if (juce::ComboBox* comboBox = dynamic_cast<juce::ComboBox*> (component))
    {
        comboBox->setSelectedId ((int) value + 1, juce::dontSendNotification); // +1 because ComboBox IDs start at 1
    }
    else if (juce::TextEditor* textEditor = dynamic_cast<juce::TextEditor*> (component))
    {
        textEditor->moveCaretToEnd();
        textEditor->insertTextAtCaret ("Received MIDI CC message: CC = " + juce::String ((int) midiCC) + ", value = " + juce::String ((int) value) + "\n\n");
    }
}
