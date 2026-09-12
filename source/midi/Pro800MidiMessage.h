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

#include "../tailoring/Pro800Constants.h"
#include <juce_audio_basics/juce_audio_basics.h>

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <vector>

class Pro800MidiMessage
{
public:
    // clang-format off
    static inline const std::vector<uint8_t> PRO800_HEADER = {
        0x00, 0x20, 0x32,      // Brand ID (Behringer)
        0x00, 0x01, 0x24,      // Product ID (Pro-800)
        0x00                   // CPU ID
    };
    // clang-format on

    static constexpr size_t POS_MESSAGE_TYPE = 0x08;
    static constexpr uint8_t RESPONSE_UNINIT = 0xFF;

    explicit Pro800MidiMessage (const juce::MidiMessage& message);
    Pro800MidiMessage (const uint8_t* newRawData, int newRawDataSize);

    // copy/move are deep by default: the whole state is a std::vector
    Pro800MidiMessage (const Pro800MidiMessage&) = default;
    Pro800MidiMessage& operator= (const Pro800MidiMessage&) = default;
    Pro800MidiMessage (Pro800MidiMessage&&) noexcept = default;
    Pro800MidiMessage& operator= (Pro800MidiMessage&&) noexcept = default;
    virtual ~Pro800MidiMessage() = default;

    virtual MessageType getMessageType() const { return MessageType::PRO800_UNKNOWN; }

    virtual juce::String toString() const;
    juce::MidiMessage toMidiMessage() const;

    virtual bool isValid() const;

    const std::vector<uint8_t>& getRawData() const;
    size_t getRawDataSize() const;

    /** True if the position holds a data byte, i.e. lies strictly between the leading F0 and the trailing F7. */
    bool isDataPosition (size_t position) const;

protected:
    // builds a complete SysEx request: F0 <PRO800_HEADER> <payload> F7
    static juce::MidiMessage makeRequest (std::initializer_list<uint8_t> payload);

    uint8_t getUint8Value (size_t position) const;
    void setUint8Value (size_t position, uint8_t value);

    // grows/shrinks the raw buffer (zero-filled); used to upgrade older message layouts
    void resizeRawData (size_t newSize);

    virtual uint8_t getResponseType() const;

private:
    bool isCorrectResponse() const;

    std::vector<uint8_t> rawData;
};
