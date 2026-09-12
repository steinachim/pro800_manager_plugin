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

#include "StatusMessage.h"

/**
 * 0x32 00: makes the synth recall the preset its settings block points at (Current Preset Number / Current Bank)
 * into the voice engine, discarding unsaved front-panel edits - the same as reselecting the preset on the synth.
 * It does not change the panel's mode. Answered with a status message.
 *
 * Only parameter 0x00 is safe: any other value puts the synth into a broken "8888" state (see Pro800Hazards).
 */
class ReloadMessage : public Pro800MidiMessage
{
public:
    static constexpr uint8_t REQUEST_ID = 0x32;
    static constexpr uint8_t RESPONSE_ID = StatusMessage::RESPONSE_ID;

    static juce::MidiMessage request()
    {
        return makeRequest ({ REQUEST_ID, 0x00 });
    }

private:
    // request only, no dedicated response: not meant to be instantiated
    ReloadMessage() = delete;
};
