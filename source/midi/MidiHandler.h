/** 
 * Pro800 Manager Plugin
 * Copyright (C) 2025 Achim Stein
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "../constants/Pro800Constants.h"

class Pro800ManagerAudioProcessor;
class MidiComponent;

namespace juce
{
    class MidiMessage;
    class Component;
}


class MidiHandler : public juce::MidiInputCallback
{
public:
    MidiHandler (Pro800ManagerAudioProcessor* processor);
    ~MidiHandler() override;

    void setMidiChannel(uint8_t channel);

    void connectMidiDevices(const juce::String& inputDeviceName, const juce::String& outputDeviceName);
    void handleIncomingMidiMessage (juce::MidiInput *source, const juce::MidiMessage& message) override;


    void handleMidiMessage(const juce::MidiMessage& message, bool sent);

    void registerMidiCCComponent(MidiComponent *component);
    void unregisterMidiCCComponent(MidiComponent *component);

    void registerMidiLogComponent(juce::Component *component);
    void unregisterMidiLogComponent(juce::Component *component);

    void registerMessageComponent(MessageType type, MidiComponent *component);
    void unregisterMessageComponent(MessageType type, MidiComponent *component);

    void sendMidiCCMessage(uint8_t midiCC, uint8_t value);
    void sendProgramChange (uint8_t program);
    void sendMidiMessage(const juce::MidiMessage& message);


private:
    juce::Array<MidiComponent *> midiCCComponents;
    juce::HashMap<MessageType, juce::Array<MidiComponent *>> midiComponents;
    Pro800ManagerAudioProcessor* processor = nullptr;

    std::unique_ptr<juce::MidiInput> midiInput;
    std::unique_ptr<juce::MidiOutput> midiOutput;

    uint8_t midiChannel = 0;

};