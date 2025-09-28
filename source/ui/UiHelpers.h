/*
  ==============================================================================

    UiHelpers.h
    Created: 13 Sep 2025 9:18:10am
    Author:  Achim Stein

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>

class EqualSpacingGroupComponent;
class MidiHandler;

class UiHelpers
{
public:
    static void setComponentCCValue(juce::Component* component, uint8_t midiCC, uint8_t value);

private:
    UiHelpers()
    {
        // only static members in this helper class
    }
};
