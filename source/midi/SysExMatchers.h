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
#include <functional>
#include <optional>

/**
 * Predicates that decide whether a received message answers a request (see SysExExchange). The Pro-800 carries no
 * request id, so a reply is recognised by its type and, where one exists, its echoed address or index. A bare
 * F0 F7 (the answer to reading an empty program slot) has neither and can only be taken as the answer to
 * whatever is in flight - which is sound because requests are serialized.
 */
class SysExMatchers
{
public:
    using Matcher = std::function<bool (const juce::MidiMessage&)>;

    /** F0 F7 and nothing else. */
    static bool isEmptySlotReply (const juce::MidiMessage& message);

    /** The message type byte if the message is a Pro-800 SysEx with one, else nullopt. */
    static std::optional<uint8_t> pro800Type (const juce::MidiMessage& message);

    static bool isStatusReply (const juce::MidiMessage& message);
    static bool isVersionReply (const juce::MidiMessage& message);

    /**
     * A 0x78 dump echoing the program's address (510 = the settings block), a bare F0 F7 (empty slot) or a status
     * (out-of-range address). A dump longer than its preset version allows is not accepted: the synth never sends
     * one, it is two replies spliced together on a shared bus.
     */
    static bool isDumpReplyFor (const juce::MidiMessage& message, int programNumber);

    /** A 0x71 reply echoing the index, or a status (the synth refuses indices it does not have). */
    static bool isPanelReplyFor (const juce::MidiMessage& message, uint8_t index);

    /** A 0x73 reply echoing the index, or a status (the synth refuses indices it does not have). */
    static bool isLiveReplyFor (const juce::MidiMessage& message, uint8_t index);

    /** A program dump whose payload is longer than a record of its declared preset version can be (docs/Pro800SysExMessages.md). */
    static bool outrunsDeclaredVersion (const juce::MidiMessage& message);
};
