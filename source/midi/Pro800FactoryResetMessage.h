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
 * 0x7D: factory reset. No parameter (docs/Pro800SysExMessages.md), no confirmation from the synth, no undo.
 * Pro800Hazards refuses it in the Advanced tab; the Settings tab sends it only after the user has confirmed.
 */
class Pro800FactoryResetMessage : public Pro800MidiMessage
{
public:
    static constexpr uint8_t REQUEST_ID = 0x7D;
    static constexpr uint8_t RESPONSE_ID = StatusMessage::RESPONSE_ID;

    static juce::MidiMessage request()
    {
        return makeRequest ({ REQUEST_ID });
    }

private:
    // request only, no dedicated response: not meant to be instantiated
    Pro800FactoryResetMessage() = delete;
};