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

#include "Pro800MidiMessage.h"
#include <juce_audio_basics/juce_audio_basics.h>

class Pro800MessageFactory
{
public:
    /**
     * Parses a received message into the matching Pro800MidiMessage subclass. Returns nullptr if it is
     * not a well-formed Pro-800 SysEx message at all, and a plain Pro800MidiMessage (PRO800_UNKNOWN)
     * for message types this plugin does not know.
     */
    static std::shared_ptr<Pro800MidiMessage> createMidiMessage (const juce::MidiMessage& message);
};
