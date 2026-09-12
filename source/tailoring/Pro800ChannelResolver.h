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

#include "Pro800PanelConstants.h"

#include <map>
#include <optional>
#include <string>

/**
 * One of the synth's MIDI channel settings (MIDI RX Channel at settings offset 10, MIDI TX Channel at offset 12),
 * decoded as far as the wire allows. The "DIP switches" case is resolved from a 0x70 read of the four rear
 * switches, see Pro800ChannelResolver.
 */
struct Pro800MidiChannel
{
    enum class Kind {
        CHANNEL, // a fixed channel 1-16
        ALL, // RX only: the synth listens on every channel
        THRU, // TX only: the synth transmits nothing and passes incoming MIDI through
        DIP, // set by the rear DIP switches; channel holds the resolved value, 0 if they could not be read
        OFF, // RX only: the synth ignores channel-voice MIDI
        INVALID // outside the documented range; the synth stores it and behaves as OFF
    };

    Kind kind = Kind::INVALID;
    int channel = 0; // 1-16 for CHANNEL and a resolved DIP, otherwise 0
    int raw = 0; // the settings byte as read

    /** The channel (1-16) to send channel-voice messages on so that the synth hears them; nullopt if none would work. ALL -> 1. */
    std::optional<int> sendChannel() const;

    /** e.g. "channel 3", "DIP switches (channel 3)", "OFF", "invalid (249)". */
    std::string toString() const;
};

class Pro800ChannelResolver
{
public:
    /** What to send on when the synth listens on every channel. */
    static constexpr int CHANNEL_FOR_ALL = 1;

    /** The 0-based channel the DIP switches select (the sum of their weights), or nullopt if a reading is missing. */
    static std::optional<int> dipChannelFromPanel (const std::map<Pro800PanelIndex, int>& panelValues);

    /** MIDI RX Channel: 0 = ALL, 1 = DIP switches (resolved with dipSum), 2-17 = channel 1-16, 18 = OFF, anything else = INVALID. */
    static Pro800MidiChannel resolveRx (int raw, std::optional<int> dipSum);

    /** MIDI TX Channel: 0 = THRU, 1 = DIP switches (resolved with dipSum), 2-17 = channel 1-16, anything else = INVALID. */
    static Pro800MidiChannel resolveTx (int raw, std::optional<int> dipSum);
};
