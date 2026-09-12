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

#include "../tailoring/Pro800CCConstants.h"
#include "../tailoring/Pro800Constants.h"
#include "../tailoring/Pro800ProgramConstants.h"
#include "../tailoring/Pro800SettingsConstants.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <map>
#include <memory>
#include <vector>

class MidiHandler;
class SettingsMessage;
class VersionMessage;
class ProgramMessage;
class Pro800MidiMessage;

class MidiComponent
{
public:
    // keys of the component properties that link a control to its CC / program field / setting
    static inline const juce::Identifier RADIO_VALUE_PROPERTY { "radioValue" };
    static inline const juce::Identifier MIDI_CC_PROPERTY { "midiCC" };
    static inline const juce::Identifier PROGRAM_FIELD_PROPERTY { "programField" };
    static inline const juce::Identifier SETTINGS_FIELD_PROPERTY { "settingsField" };

    static constexpr int PROGRAM_SEND_INTERVAL_MS = 20; // pause between two program dumps sent to the synth

    /** Combo box item ids are the enum (or CC) value plus this offset, because item ids must be non-zero. */
    static constexpr int COMBO_BOX_ID_OFFSET = 1;

    struct EnumItem
    {
        const char* name;
        int value;
    };

    /** Adds the items to the combo box in the given order, with ids derived from the values (see COMBO_BOX_ID_OFFSET). */
    static void addEnumItems (juce::ComboBox& comboBox, const std::vector<EnumItem>& items);

    MidiComponent (MidiHandler* midiHandler, bool registerMidiCC = false, const juce::Array<MessageType> messageTypes = juce::Array<MessageType>());
    virtual ~MidiComponent();

    void handlePro800Message (MessageType type, std::shared_ptr<Pro800MidiMessage>& settingsMessage);
    void handleMidiCCMessage (Pro800CCMessages midiCC, uint8_t value);

    virtual void handlePro800SettingsUpdate();
    virtual void handlePro800VersionUpdate();
    virtual void handlePro800ProgramDump (std::shared_ptr<ProgramMessage>& programMessage);
    virtual void handleMidiLog (const juce::MidiMessage& message, const juce::String& logPrefix);

    void requestFactoryReset();
    void requestProgramDump();
    /** Switches the synth to the program and updates the mirroring controls, see MidiHandler::loadProgram(). */
    void loadProgram (const ProgramMessage& program);

    /** Writes the programs to the synth, paced in the background so that the UI stays responsive. */
    void sendPrograms (const std::vector<std::shared_ptr<ProgramMessage>>& programs);

    /** Sends an arbitrary message to the synth immediately. */
    void sendMidiMessage (const juce::MidiMessage& message);

    /** Sets this component's CC-linked controls from the program. */
    virtual void loadFromProgram (const ProgramMessage& program);

protected:
    MidiHandler& getMidiHandler() const;

    /** The program field / CC a control was linked to via setupMidiComponent() (NONE if not linked). */
    static Pro800ProgramField getProgramField (const juce::Component& component);
    static Pro800CCMessages getMidiCC (const juce::Component& component);

    void setupMidiComponent (juce::Component* component, Pro800CCMessages midiCC, Pro800ProgramField programField, Pro800Settings settingsField = Pro800Settings::NONE);

    std::shared_ptr<SettingsMessage>& getCurrentSettings();
    void updateSettings (Pro800Settings setting, int value);

    std::shared_ptr<VersionMessage>& getCurrentVersion();

    virtual void setComponentValue (juce::Component* component, int value, int maxValue = -1);

private:
    juce::Array<MessageType> registeredMessageTypes = juce::Array<MessageType>();
    std::map<Pro800CCMessages, juce::Array<juce::Component*>> registeredCCComponents;

    MidiHandler* midiHandler; // non-owning: lifetime managed by the audio processor/editor

    std::shared_ptr<SettingsMessage> currentSettings = std::shared_ptr<SettingsMessage>();
    std::shared_ptr<VersionMessage> currentVersion = std::shared_ptr<VersionMessage>();
};