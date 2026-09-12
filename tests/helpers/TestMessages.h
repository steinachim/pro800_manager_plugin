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

#include <juce_audio_basics/juce_audio_basics.h>

#include <cstdint>
#include <initializer_list>
#include <vector>

#include "midi/Pro800DataMessage.h"
#include "midi/ProgramMessage.h"
#include "midi/SettingsMessage.h"

// Builders for raw Pro-800 SysEx messages, so that the tests can describe the wire format byte by byte.
namespace TestMessages
{
    /** F0 <Pro-800 header> <payload> F7 */
    inline std::vector<uint8_t> sysEx(std::initializer_list<uint8_t> payload)
    {
        std::vector<uint8_t> bytes = { 0xF0 };
        bytes.insert(bytes.end(), Pro800MidiMessage::PRO800_HEADER.begin(), Pro800MidiMessage::PRO800_HEADER.end());
        bytes.insert(bytes.end(), payload.begin(), payload.end());
        bytes.push_back(0xF7);
        return bytes;
    }

    inline juce::MidiMessage toMidi(const std::vector<uint8_t> &bytes)
    {
        return juce::MidiMessage(bytes.data(), (int) bytes.size());
    }

    /**
     * A program dump (0x78) with the given number of zeroed data bytes and the given preset version.
     * 198 data bytes give the complete current layout (ProgramMessage::PROGRAM_MESSAGE_SIZE == 210 bytes).
     */
    inline std::vector<uint8_t> programDump(uint16_t programNumber, uint8_t version = ProgramMessage::SUPPORTED_PRESET_VERSION, size_t numDataBytes = 198)
    {
        std::vector<uint8_t> bytes = { 0xF0 };
        bytes.insert(bytes.end(), Pro800MidiMessage::PRO800_HEADER.begin(), Pro800MidiMessage::PRO800_HEADER.end());
        bytes.push_back(Pro800DataMessage::RESPONSE_ID);
        bytes.push_back((uint8_t) (programNumber & 0x7F));
        bytes.push_back((uint8_t) (programNumber >> 7));
        bytes.insert(bytes.end(), numDataBytes, 0x00);

        const size_t versionOffset = PRO800_PROGRAM_FIELDS.at(Pro800ProgramField::PRESET_VERSION).firstByte;
        bytes[Pro800DataMessage::DATA_START_POS + versionOffset] = version;

        bytes.push_back(0xF7);
        return bytes;
    }

    /** A settings dump (0x78 at address 7E 03) of the exact size SettingsMessage expects. */
    inline std::vector<uint8_t> settingsDump()
    {
        std::vector<uint8_t> bytes = { 0xF0 };
        bytes.insert(bytes.end(), Pro800MidiMessage::PRO800_HEADER.begin(), Pro800MidiMessage::PRO800_HEADER.end());
        bytes.push_back(Pro800DataMessage::RESPONSE_ID);
        bytes.push_back(SettingsMessage::ADDRESS_LOW);
        bytes.push_back(SettingsMessage::ADDRESS_HIGH);
        bytes.resize(SettingsMessage::SETTINGS_MESSAGE_SIZE - 1, 0x00);
        bytes.push_back(0xF7);
        return bytes;
    }

    /** SysEx data bytes must not have the high bit set; only F0/F7 at the ends may. */
    inline bool allDataBytesAre7Bit(const std::vector<uint8_t> &bytes)
    {
        for ( size_t i = 1; i + 1 < bytes.size(); i++ )
        {
            if ( bytes[i] > 0x7F )
            {
                return false;
            }
        }
        return true;
    }
}
