#pragma once

#include <juce_core/juce_core.h>

#include "../Pro800Constants.h"

class Pro800ManagerAudioProcessor;
class MidiComponent;

namespace juce
{
    class MidiMessage;
    class Component;
}


class MidiHandler
{
public:
    MidiHandler(Pro800ManagerAudioProcessor* processor);
    ~MidiHandler();


    void handleMidiMessage(const juce::MidiMessage& message);

    void registerMidiCCComponent(uint8_t midiCC, juce::Component *component);
    void unregisterMidiCCComponent(uint8_t midiCC, juce::Component *component);

    void registerMidiLogComponent(juce::Component *component);
    void unregisterMidiLogComponent(juce::Component *component);

    void registerPro800MessageComponent(Pro800MessageType type, MidiComponent *component);
    void unregisterPro800MessageComponent(Pro800MessageType type, MidiComponent *component);

    void sendMidiCCMessage(uint8_t midiCC, uint8_t value);
    void sendMidiMessage(const juce::MidiMessage& message);


private:
    juce::HashMap<uint8_t, juce::Array<juce::Component *>> midiCCComponents;
    juce::HashMap<Pro800MessageType, juce::Array<MidiComponent *>> pro800Components;
    juce::Array<juce::Component*> logComponents;
    Pro800ManagerAudioProcessor* processor = nullptr;

};