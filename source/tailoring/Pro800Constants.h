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

#include <cstddef>
#include <cstdint>
#include <string>

/**
 * Describes one field of a program or settings dump. Positions count from the start of the data block
 * (after the address bytes) and include the overflow bytes that sit at every multiple of 8; numBytes
 * counts value bytes only, so a value may straddle an overflow byte. See docs/Pro800SysExMessages.md.
 */
struct Pro800Parameter
{
    size_t firstByte; // position of the first value byte in the data block
    uint8_t numBytes; // value bytes (1, 2 or 4), little-endian, overflow bytes not counted
    std::string name;
    uint8_t numValues = 0; // number of enum values, 0 = continuous (see Pro800CCUtils for the CC mapping)
    bool isSigned = false; // two's complement, int values only
};

// clang-format off
enum class MessageType
{
    MIDI_LOG,        // every message sent or received, for logging
    PRO800_UNKNOWN,  // a Pro-800 SysEx message of a type this plugin does not know
    PRO800_SETTINGS,
    PRO800_VERSION,
    PRO800_STATUS,
    PRO800_PROGRAM,
    PRO800_PANEL     // 0x71: the state of one front-panel control (reply to 0x70)
};
// clang-format on
