#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "Pro800MidiMessage.h"

class Pro800MessageFactory
{
public:
    static std::shared_ptr<Pro800MidiMessage> createMidiMessage(const juce::MidiMessage &message);
};
