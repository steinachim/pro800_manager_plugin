#include "MidiCallbackMessage.h"

#include "MidiHandler.h"

MidiCallbackMessage::MidiCallbackMessage(MidiHandler *handler, const juce::MidiMessage &message, bool sent)
{
    this->midiHandler = handler;
    this->midiMessage = message;
    this->sentMessage = sent;
}

void MidiCallbackMessage::messageCallback()
{
    this->midiHandler->handleMidiMessage(this->midiMessage, this->sentMessage);
}