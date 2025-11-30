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

struct Pro800Parameter
{    
    size_t firstByte;
    uint8_t numBytes;
    std::string name;
    uint8_t numValues = 0; // 0 = continuous, used for enum values
    bool isSigned = false; // for int values only
};

enum MessageType {
    MIDI_CC_MESSAGE,
    MIDI_LOG_MESSAGE,
    PRO800_UNKNOWN_MESSAGE,
    PRO800_SETTINGS_MESSAGE,
    PRO800_VERSION_MESSAGE,
    PRO800_STATUS_MESSAGE,
    PRO800_PROGRAM_MESSAGE
};

