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

void UiHelpers::setupRotarySlider(juce::Slider &slider, EqualSpacingGroupComponent &parent)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setRange(0.0, 127.0, 1.0);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 15);
    slider.setPopupDisplayEnabled(false, false, &parent);
    slider.setValue(1.0);
    
    parent.addComponent(&slider);
}

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

void UiHelpers::setComponentLogValue(juce::Component *component, const juce::MidiMessage &midiMessage, const juce::String &prefix)
{
    if (juce::TextEditor* textEditor = dynamic_cast<juce::TextEditor*> (component))
    {
        juce::String messageText = midiMessage.getDescription();
        std::shared_ptr<Pro800MidiMessage> pro800Message = Pro800MessageFactory::createMidiMessage(midiMessage);

        if ( pro800Message )
        {
            messageText = pro800Message->toString();
        }

        textEditor->moveCaretToEnd();
        textEditor->insertTextAtCaret ("\n");
        textEditor->insertTextAtCaret (prefix + " " + messageText + "\n\n");
    }
    else
    {
        std::cerr << "[WARNING] setComponentLogValue(): Unknown target component type" << std::endl;
    }
}