#pragma once

#include <juce_core/juce_core.h>

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

    void registerSettingsComponent(MidiComponent *component);
    void unregisterSettingsComponent(MidiComponent *component);

    void sendMidiCCMessage(uint8_t midiCC, uint8_t value);
    void sendMidiMessage(const juce::MidiMessage& message);


private:
    juce::HashMap<uint8_t, juce::Array<juce::Component *>> midiCCComponents;
    juce::Array<MidiComponent *> settingsComponents;
    juce::Array<juce::Component*> logComponents;
    Pro800ManagerAudioProcessor* processor = nullptr;

};