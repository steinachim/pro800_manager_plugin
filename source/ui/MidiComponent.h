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
class PanelMessage;
class SettingsMessage;
class SynthSession;
class VersionMessage;
class ProgramMessage;
class Pro800MidiMessage;

/**
 * Base class for every UI component that talks to the synth.
 *
 * A component registers with the MidiHandler for the message types it wants to receive (constructor
 * argument) and, if it mirrors program parameters, for incoming CCs (registerMidiCC). Its controls are
 * linked to a CC / program field / setting with setupMidiComponent(): moving a control sends the CC,
 * an incoming CC or a loaded program moves the control.
 */
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

    /** registerMidiCC: receive incoming CCs and mirror loaded programs; messageTypes: the Pro-800 messages to receive. */
    MidiComponent (MidiHandler* midiHandler, SynthSession& synthSession, bool registerMidiCC = false, const juce::Array<MessageType> messageTypes = juce::Array<MessageType>());
    virtual ~MidiComponent();

    /** Dispatches a received Pro-800 message to the matching handlePro800*() callback. */
    void handlePro800Message (MessageType type, const std::shared_ptr<Pro800MidiMessage>& message);
    void handleMidiCCMessage (Pro800CCMessages midiCC, uint8_t value);

    virtual void handlePro800SettingsUpdate();
    virtual void handlePro800VersionUpdate();
    virtual void handlePro800ProgramDump (const std::shared_ptr<ProgramMessage>& programMessage);
    virtual void handlePro800PanelUpdate (const std::shared_ptr<PanelMessage>& panelMessage);
    /** isPolling: routine background traffic (the session keeping the current preset up to date), worth hiding from a log. */
    virtual void handleMidiLog (const juce::MidiMessage& message, const juce::String& logPrefix, bool isPolling);

    void requestFactoryReset();
    void requestProgramDump();

    /** Writes the programs to the synth, paced in the background so that the UI stays responsive. */
    void sendPrograms (const std::vector<std::shared_ptr<ProgramMessage>>& programs);

    /** Sends an arbitrary message to the synth immediately. */
    void sendMidiMessage (const juce::MidiMessage& message);

    /** Sets this component's CC-linked controls from the program. */
    virtual void loadFromProgram (const ProgramMessage& program);

protected:
    MidiHandler& getMidiHandler() const;
    SynthSession& getSynthSession() const;

    /** The program field / CC a control was linked to via setupMidiComponent() (NONE if not linked). */
    static Pro800ProgramField getProgramField (const juce::Component& component);
    static Pro800CCMessages getMidiCC (const juce::Component& component);

    void setupMidiComponent (juce::Component* component, Pro800CCMessages midiCC, Pro800ProgramField programField, Pro800Settings settingsField = Pro800Settings::NONE);

    /** The synth's settings block as the session last read it (nullptr before the first read). */
    std::shared_ptr<SettingsMessage> getCurrentSettings() const;

    /** Changes one setting on the synth: the session patches its block, writes it and keeps the value until the synth confirms it. */
    void updateSettings (Pro800Settings setting, int value);

    std::shared_ptr<VersionMessage>& getCurrentVersion();

    virtual void setComponentValue (juce::Component* component, int value, int maxValue = -1);

    /**
     * A control whose value has not come from anywhere yet (no preset loaded, nothing received) is shown dimmed,
     * so that its resting position is not mistaken for the synth's state. setComponentValue() and the user's own
     * gesture clear that.
     */
    static void setControlKnown (juce::Component* component, bool known);
    static constexpr float UNKNOWN_CONTROL_ALPHA = 0.45f;

private:
    juce::Array<MessageType> registeredMessageTypes = juce::Array<MessageType>();
    std::map<Pro800CCMessages, juce::Array<juce::Component*>> registeredCCComponents;

    MidiHandler* midiHandler; // non-owning: lifetime managed by the audio processor
    SynthSession* synthSession; // non-owning: lifetime managed by the audio processor

    std::shared_ptr<VersionMessage> currentVersion = std::shared_ptr<VersionMessage>();
};