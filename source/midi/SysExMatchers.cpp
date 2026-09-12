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

#include "SysExMatchers.h"

#include "PanelMessage.h"
#include "Pro800DataMessage.h"
#include "Pro800MidiMessage.h"
#include "ProgramMessage.h"
#include "StatusMessage.h"
#include "VersionMessage.h"

#include <map>

bool SysExMatchers::isEmptySlotReply (const juce::MidiMessage& message)
{
    return message.getRawDataSize() == 2 && message.getRawData()[0] == 0xF0 && message.getRawData()[1] == 0xF7;
}

std::optional<uint8_t> SysExMatchers::pro800Type (const juce::MidiMessage& message)
{
    const Pro800MidiMessage generic (message); // base isValid(): frame, header, and a type byte
    if (!generic.isValid())
    {
        return std::nullopt;
    }

    return generic.getRawData()[Pro800MidiMessage::POS_MESSAGE_TYPE];
}

bool SysExMatchers::isStatusReply (const juce::MidiMessage& message)
{
    return pro800Type (message) == StatusMessage::RESPONSE_ID;
}

bool SysExMatchers::isVersionReply (const juce::MidiMessage& message)
{
    return pro800Type (message) == VersionMessage::RESPONSE_ID;
}

bool SysExMatchers::isDumpReplyFor (const juce::MidiMessage& message, int programNumber)
{
    if (isEmptySlotReply (message) || isStatusReply (message))
    {
        return true;
    }

    if (pro800Type (message) != Pro800DataMessage::RESPONSE_ID)
    {
        return false;
    }

    const auto* rawData = message.getRawData();
    if (message.getRawDataSize() <= (int) Pro800DataMessage::ADDRESS_MSB_POS + 1)
    {
        return false; // no complete address before the F7
    }

    const uint8_t addressLSB = (uint8_t) (programNumber & 0x7F);
    const uint8_t addressMSB = (uint8_t) (programNumber >> 7);
    if (rawData[Pro800DataMessage::ADDRESS_LSB_POS] != addressLSB || rawData[Pro800DataMessage::ADDRESS_MSB_POS] != addressMSB)
    {
        return false;
    }

    return !outrunsDeclaredVersion (message);
}

bool SysExMatchers::isPanelReplyFor (const juce::MidiMessage& message, uint8_t index)
{
    if (isStatusReply (message))
    {
        return true;
    }

    if (pro800Type (message) != PanelMessage::RESPONSE_ID)
    {
        return false;
    }

    const PanelMessage panel (message);
    return panel.isValid() && panel.getIndex() == index;
}

bool SysExMatchers::outrunsDeclaredVersion (const juce::MidiMessage& message)
{
    // the longest raw payload (address bytes and F7 excluded) a record of each preset version can have
    static const std::map<uint8_t, int> MAX_RAW_PAYLOAD_LENGTH = { { 109, 190 }, { 110, 192 }, { 111, 198 } };

    if (pro800Type (message) != Pro800DataMessage::RESPONSE_ID)
    {
        return false;
    }

    const int dataStart = (int) Pro800DataMessage::DATA_START_POS;
    const int versionPos = dataStart + (int) PRO800_PROGRAM_FIELDS.at (Pro800ProgramField::PRESET_VERSION).firstByte;
    if (message.getRawDataSize() <= versionPos + 1)
    {
        return false; // too short to carry a version byte: not this check's business
    }

    const auto limit = MAX_RAW_PAYLOAD_LENGTH.find (message.getRawData()[versionPos]);
    if (limit == MAX_RAW_PAYLOAD_LENGTH.end())
    {
        return false;
    }

    const int payloadLength = message.getRawDataSize() - dataStart - 1; // minus the F7
    return payloadLength > limit->second;
}
