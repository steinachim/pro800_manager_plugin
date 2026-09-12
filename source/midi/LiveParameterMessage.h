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

#include "../tailoring/Pro800PanelConstants.h"
#include "Pro800MidiMessage.h"

/**
 * The position of one knob: the reply (0x73 <index> <value>, 7-bit) to a 0x72 <index> request. Indices and their
 * meaning: Pro800LiveIndex. The same 0x73 type with an index and a value is also the request that sets a knob's
 * value in the edit buffer (writeRequest()).
 */
class LiveParameterMessage : public Pro800MidiMessage
{
public:
    static constexpr uint8_t REQUEST_ID = 0x72;
    static constexpr uint8_t RESPONSE_ID = 0x73;

    static constexpr size_t POS_INDEX = 0x09;
    static constexpr size_t POS_VALUE = 0x0A;
    static constexpr int MAX_VALUE = 0x7F;

    /** Reads one knob. Indices from Pro800LiveIndex::NUM_INDICES on answer a failure status. */
    static juce::MidiMessage request (Pro800LiveIndex index);
    static juce::MidiMessage request (uint8_t index);

    /** Sets one knob's value (0-127) in the edit buffer; answered with a status. */
    static juce::MidiMessage writeRequest (Pro800LiveIndex index, uint8_t value);

    explicit LiveParameterMessage (const juce::MidiMessage& message);
    virtual MessageType getMessageType() const override { return MessageType::PRO800_LIVE_PARAMETER; }

    virtual bool isValid() const override;

    virtual juce::String toString() const override;

    uint8_t getIndex() const;
    int getValue() const;

    /** The name of the knob the message reports, or the index in hex if it has no known name. */
    juce::String getIndexName() const;

protected:
    uint8_t getResponseType() const override;
};
