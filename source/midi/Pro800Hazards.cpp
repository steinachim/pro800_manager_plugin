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

#include "Pro800Hazards.h"

#include "Pro800MidiMessage.h"

#include <algorithm>

std::optional<std::pair<uint8_t, std::optional<uint8_t>>> Pro800Hazards::typeAndParam (const std::vector<uint8_t>& rawMessage)
{
    const size_t typePos = Pro800MidiMessage::POS_MESSAGE_TYPE;
    if (rawMessage.size() <= typePos + 1 || rawMessage.front() != 0xF0 || rawMessage.back() != 0xF7)
    {
        return std::nullopt;
    }

    const auto& header = Pro800MidiMessage::PRO800_HEADER;
    if (!std::equal (header.begin(), header.end(), rawMessage.begin() + 1))
    {
        return std::nullopt;
    }

    const uint8_t type = rawMessage[typePos];
    std::optional<uint8_t> param;
    if (rawMessage.size() > typePos + 2) // a byte between the type and the F7
    {
        param = rawMessage[typePos + 1];
    }

    return std::make_pair (type, param);
}

std::optional<juce::String> Pro800Hazards::hazardReason (const std::vector<uint8_t>& rawMessage)
{
    const auto parsed = typeAndParam (rawMessage);
    if (!parsed)
    {
        return std::nullopt;
    }

    const auto [type, param] = *parsed;

    switch (type)
    {
        case TYPE_FACTORY_RESET:
            return juce::String ("0x7D is the factory reset: no confirmation from the synth, no undo.");

        case TYPE_UNKNOWN_03:
            if (param == PARAM_BOOTLOADER)
            {
                return juce::String ("0x03 with parameter 0x30 reboots the synth into its bootloader: the display shows 'boot' and only a "
                                     "power cycle brings it back (presets and firmware stay intact). Every other parameter does nothing.");
            }
            return std::nullopt;

        case TYPE_RELOAD:
            if (param.has_value() && *param != 0x00)
            {
                return juce::String ("0x32 with a non-zero parameter puts the synth into a state that shows '8888' and stops responding "
                                     "properly to its own controls. Only 0x32 00 (reload the stored preset) is safe.");
            }
            return std::nullopt;

        case TYPE_CHANNEL_WRITE:
            return juce::String ("0x0E rewrites both MIDI channel fields; any non-zero parameter sets an invalid RX channel (249) that "
                                 "makes the synth deaf to notes, program changes and CC. Recovery needs a settings write from the Settings tab.");

        case TYPE_SET_NAME:
            return juce::String ("0x50 writes preset name bytes directly and leaves the record inconsistent; sent with only an address it "
                                 "blanks the name (a sweep once blanked every preset name in the library). Rename by rewriting the whole "
                                 "program instead.");

        default:
            return std::nullopt;
    }
}

bool Pro800Hazards::isIrreversible (const std::vector<uint8_t>& rawMessage)
{
    const auto parsed = typeAndParam (rawMessage);
    if (!parsed)
    {
        return false;
    }

    const auto [type, param] = *parsed;
    return type == TYPE_FACTORY_RESET || (type == TYPE_UNKNOWN_03 && param == PARAM_BOOTLOADER);
}
