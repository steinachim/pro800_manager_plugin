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
 * The state of one front-panel control: the reply (0x71 <index> <value>) to a 0x70 <index> request.
 * Indices and their meaning: Pro800PanelIndex. The same 0x71 type is also the "press button" request, which is
 * why a sent press shows up here as an incomplete (value-less) message in the log.
 */
class PanelMessage : public Pro800MidiMessage
{
public:
    static constexpr uint8_t REQUEST_ID = 0x70;
    static constexpr uint8_t RESPONSE_ID = 0x71;

    static constexpr size_t POS_INDEX = 0x09;
    static constexpr size_t POS_VALUE = 0x0A;

    /** Reads one control. Indices from Pro800PanelIndex::NUM_INDICES on (and a request without an index) answer a failure status. */
    static juce::MidiMessage request (Pro800PanelIndex index);
    static juce::MidiMessage request (uint8_t index);

    explicit PanelMessage (const juce::MidiMessage& message);
    virtual MessageType getMessageType() const override { return MessageType::PRO800_PANEL; }

    virtual bool isValid() const override;

    virtual juce::String toString() const override;

    uint8_t getIndex() const;
    int getValue() const;

    /** The name of the control the message reports, or the index in hex if it has no known name. */
    juce::String getIndexName() const;

protected:
    uint8_t getResponseType() const override;
};
