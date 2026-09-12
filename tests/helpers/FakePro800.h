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

#include "TestMessages.h"

#include "midi/LiveParameterMessage.h"
#include "midi/MidiHandler.h"
#include "midi/PanelMessage.h"
#include "midi/ProgramMessage.h"
#include "midi/ReloadMessage.h"
#include "midi/SettingsMessage.h"
#include "midi/StatusMessage.h"
#include "midi/VersionMessage.h"

#include <map>
#include <vector>

namespace TestMessages
{
    /**
     * A Pro-800 behind MidiHandler's test transport: answers identity, settings and program reads, settings writes
     * (with an optional commit lag, as the real one has), the reload and panel reads. Replies are injected through
     * MidiHandler::handleIncomingMidiMessage(), so they arrive on the next run of the message loop.
     */
    struct FakePro800
    {
        explicit FakePro800 (MidiHandler& handler) : midiHandler (handler)
        {
            settings = settingsDump();
            midiHandler.setTestTransport ([this] (const juce::MidiMessage& message) { handle (message); });
        }

        ~FakePro800()
        {
            midiHandler.setTestTransport (nullptr);
        }

        //==============================================================================
        // the synth's state

        std::vector<uint8_t> settings; // the settings block as readable now (a complete 0x78 message)
        std::map<int, std::vector<uint8_t>> programs; // stored records by program number (complete 0x78 messages)
        std::map<uint8_t, int> panel; // 0x70 index -> value
        std::map<uint8_t, int> knobs; // 0x72 index -> value (0-127)

        int settingsCommitLagMs = 0; // how long after a settings write the new block becomes readable
        bool ignoreSettingsWrites = false; // accept them with OK but never commit (a menu open on the synth, say)
        bool ignoreProgramWrites = false; // accept them with OK but store nothing
        int dropNextRequests = 0; // lose the next n requests (the synth stays silent)
        int dropFromProgram = -1; // from this program on, program reads and writes go unanswered (-1 = answer them all)

        int reloads = 0;
        int settingsWrites = 0;
        std::vector<std::vector<uint8_t>> received; // every message the plugin sent, in order

        //==============================================================================
        SettingsMessage settingsMessage() const { return SettingsMessage (toMidi (settings)); }

        void setSetting (Pro800Settings setting, int value)
        {
            SettingsMessage message (toMidi (settings));
            message.setValue (setting, value);
            settings = message.getRawData();
        }

        int getSetting (Pro800Settings setting)
        {
            commitIfDue();
            return settingsMessage().getValue (setting);
        }

        void storeProgram (int number, const std::string& name)
        {
            ProgramMessage program (toMidi (programDump ((uint16_t) number)));
            program.setProgramName (name);
            programs[number] = program.getRawData();
        }

        /** Every CC the plugin sent, as controller number -> the last value sent for it. */
        std::map<int, int> receivedCCs() const
        {
            std::map<int, int> ccs;
            for (const auto& bytes : received)
            {
                if (bytes.size() == 3 && (bytes[0] & 0xF0) == 0xB0)
                {
                    ccs[bytes[1]] = bytes[2];
                }
            }
            return ccs;
        }

        /** The index in `received` of the first message of the given type (and address bytes, if given), or -1. */
        int indexOfSent (uint8_t type, std::vector<uint8_t> params = {}) const
        {
            for (size_t i = 0; i < received.size(); i++)
            {
                const auto& m = received[i];
                if (m.size() > Pro800MidiMessage::POS_MESSAGE_TYPE + params.size() && m[Pro800MidiMessage::POS_MESSAGE_TYPE] == type
                    && std::equal (params.begin(), params.end(), m.begin() + (long) Pro800MidiMessage::POS_MESSAGE_TYPE + 1))
                {
                    return (int) i;
                }
            }
            return -1;
        }

    private:
        void commitIfDue()
        {
            if (!pendingSettings.empty() && juce::Time::getMillisecondCounterHiRes() >= commitAt)
            {
                settings = pendingSettings;
                pendingSettings.clear();
            }
        }

        void reply (const std::vector<uint8_t>& bytes)
        {
            midiHandler.handleIncomingMidiMessage (nullptr, toMidi (bytes));
        }

        void handle (const juce::MidiMessage& message)
        {
            const auto bytes = bytesOf (message);
            received.push_back (bytes);

            if (dropNextRequests > 0)
            {
                dropNextRequests--;
                return;
            }

            if (!message.isSysEx() || bytes.size() <= Pro800MidiMessage::POS_MESSAGE_TYPE)
            {
                return; // channel-voice: the real synth echoes it, which is not what these tests look at
            }

            const uint8_t type = bytes[Pro800MidiMessage::POS_MESSAGE_TYPE];
            const auto param = [&] (size_t i) { return bytes.size() > Pro800MidiMessage::POS_MESSAGE_TYPE + 1 + i + 1 ? bytes[Pro800MidiMessage::POS_MESSAGE_TYPE + 1 + i] : (uint8_t) 0; };

            switch (type)
            {
                case VersionMessage::REQUEST_ID:
                    reply (sysEx ({ VersionMessage::RESPONSE_ID, 0x00, 1, 4, 6 }));
                    break;

                case Pro800DataMessage::REQUEST_ID:
                {
                    const int address = param (0) | (param (1) << 7);
                    if (dropFromProgram >= 0 && address != SettingsMessage::ADDRESS && address >= dropFromProgram)
                    {
                        return;
                    }

                    if (address == SettingsMessage::ADDRESS)
                    {
                        commitIfDue();
                        reply (settings);
                    }
                    else if (address >= ProgramMessage::NUM_PROGRAMS)
                    {
                        reply (statusReply (0x01));
                    }
                    else if (programs.count (address))
                    {
                        reply (programs.at (address));
                    }
                    else
                    {
                        reply (emptySlotReply());
                    }
                    break;
                }

                case Pro800DataMessage::RESPONSE_ID: // a write
                {
                    const int address = param (0) | (param (1) << 7);
                    if (address == SettingsMessage::ADDRESS)
                    {
                        settingsWrites++;
                        if (!ignoreSettingsWrites)
                        {
                            pendingSettings = bytes;
                            commitAt = juce::Time::getMillisecondCounterHiRes() + settingsCommitLagMs;
                        }
                    }
                    else if (!ignoreProgramWrites)
                    {
                        programs[address] = bytes;
                    }
                    reply (statusReply (0x00));
                    break;
                }

                case ReloadMessage::REQUEST_ID:
                    reloads++;
                    reply (statusReply (0x00));
                    break;

                case PanelMessage::REQUEST_ID:
                {
                    const uint8_t index = param (0);
                    if (index >= (uint8_t) Pro800PanelIndex::NUM_INDICES)
                    {
                        reply (statusReply (0x01));
                    }
                    else
                    {
                        reply (panelReply (index, (uint8_t) (panel.count (index) ? panel.at (index) : 0)));
                    }
                    break;
                }

                case LiveParameterMessage::REQUEST_ID:
                {
                    const uint8_t index = param (0);
                    if (index >= (uint8_t) Pro800LiveIndex::NUM_INDICES)
                    {
                        reply (statusReply (0x01));
                    }
                    else
                    {
                        reply (sysEx ({ LiveParameterMessage::RESPONSE_ID, index, (uint8_t) (knobs.count (index) ? knobs.at (index) : 0) }));
                    }
                    break;
                }

                default:
                    reply (statusReply (0x00));
                    break;
            }
        }

        MidiHandler& midiHandler;
        std::vector<uint8_t> pendingSettings;
        double commitAt = 0.0;
    };
}
