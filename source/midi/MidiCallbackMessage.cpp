#include "MidiCallbackMessage.h"

#include "MidiHandler.h"

MidiCallbackMessage::MidiCallbackMessage(MidiHandler *handler, juce::MidiMessage &message)
{
    this->midiHandler = handler;
    this->midiMessage = message;
}

void MidiCallbackMessage::messageCallback()
{
    this->midiHandler->handleMidiMessage(this->midiMessage);
}