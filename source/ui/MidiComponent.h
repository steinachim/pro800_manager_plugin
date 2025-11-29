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

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../constants/Pro800Constants.h"
#include "../constants/Pro800CCConstants.h"
#include "../constants/Pro800ProgramConstants.h"
#include "../constants/Pro800SettingsConstants.h"

class MidiHandler;
class SettingsMessage;
class VersionMessage;
class ProgramMessage;
class Pro800MidiMessage;

class MidiComponent
{
public:
    const juce::String RADIO_VALUE_PROPERTY {"radioValue"};
    const juce::String MIDI_CC_PROPERTY {"midiCC"};
    const juce::String PROGRAM_FIELD_PROPERTY {"programField"};
    const juce::String SETTINGS_FIELD_PROPERTY {"settingsField"};


    MidiComponent(MidiHandler *midiHandler, bool registerMidiCC = false, const juce::Array<MessageType> messageTypes = juce::Array<MessageType>());
    virtual ~MidiComponent();

    void handlePro800Message(MessageType type, std::shared_ptr<Pro800MidiMessage> &settingsMessage);
    void handleMidiCCMessage(uint8_t midiCC, uint8_t value);

    virtual void handlePro800SettingsUpdate();
    virtual void handlePro800VersionUpdate();
    virtual void handlePro800ProgramDump(std::shared_ptr<ProgramMessage> &programMessage);
    virtual void handleMidiLog(const juce::MidiMessage &message, const juce::String &logPrefix);

    void requestFactoryReset();
    void requestProgramDump();
    void loadProgram(uint16_t programNumber);
    void sendProgram(std::shared_ptr<ProgramMessage> &message);

    virtual void loadFromProgram(const std::shared_ptr<ProgramMessage> &programMessage);

protected:
    void setupMidiComponent(juce::Component *component, Pro800CCMessages midiCC, Pro800ProgramField programField, Pro800Settings settingsField = SETTINGS_FIELD_NONE);
    void removeMidiComponent(juce::Component *component);

    std::shared_ptr<SettingsMessage> &getCurrentSettings();
    void updateSettings(Pro800Settings setting, int value);

    std::shared_ptr<VersionMessage> &getCurrentVersion();

    virtual void setComponentValue(juce::Component *component, int value, int maxValue = -1);
    
private:
    juce::Array<MessageType> registeredMessageTypes = juce::Array<MessageType>();
    juce::HashMap<Pro800CCMessages, juce::Array<juce::Component*>> registeredCCComponents;

    MidiHandler *midiHandler;

    std::shared_ptr<SettingsMessage> currentSettings = std::shared_ptr<SettingsMessage>();
    std::shared_ptr<VersionMessage> currentVersion = std::shared_ptr<VersionMessage>();

    class MidiDumpRequestThread : public juce::Thread
    {
    public:
        MidiDumpRequestThread (MidiHandler* handler)
            : juce::Thread ("MidiRequestThread"), midiHandler (handler)
        {
        }

        void run() override;
        

    private:
        MidiHandler *midiHandler;
    };

    std::unique_ptr<MidiDumpRequestThread> midiDumpRequestThread;
};