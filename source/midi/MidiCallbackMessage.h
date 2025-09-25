#pragma once

#include <juce_events/juce_events.h>
#include <juce_audio_basics/juce_audio_basics.h>

class MidiHandler;

class MidiCallbackMessage : public juce::CallbackMessage
{
public:
    MidiCallbackMessage(MidiHandler *handler, const juce::MidiMessage &message, bool sent = false);
    void messageCallback() override;

private:
    MidiHandler *midiHandler;
    juce::MidiMessage midiMessage;
    bool sentMessage = false;
};