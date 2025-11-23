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

#include <string>
#include <memory>
#include <vector>

#include "Pro800MidiMessage.h"

class Pro800DataMessage : public Pro800MidiMessage
{
public:
    static const uint8_t REQUEST_ID = 0x77;
    static const uint8_t RESPONSE_ID = 0x78;

    static const uint8_t ADDRESS_LSB_POS = 0x09;
    static const uint8_t ADDRESS_MSB_POS = 0x0A;
    static const uint8_t DATA_START_POS = 0x0B;
   
    const static inline std::vector<uint8_t> EMPTY_MESSAGE = {
        0xF0,
        0x00, 0x20, 0x32,      // Brand ID (Behringer)
        0x00, 0x01, 0x24,      // Product ID (Pro-800)
        0x00,                  // CPU ID,
        RESPONSE_ID,           // command
        0x7F, 0x7F,            // program number (invalid)
        0xF7
    };

    static juce::MidiMessage request(uint8_t addressLSB, uint8_t addressMSB);

    Pro800DataMessage();
    Pro800DataMessage(const juce::MidiMessage &message);
    Pro800DataMessage(const uint8_t *newRawData, int newRawDataSize);
    Pro800DataMessage(const Pro800DataMessage &other);

protected:
    unsigned char getResponseType() const override;

    int getValue(int firstByte, int numBytes, bool isSigned = false) const;
    void setValue(int firstByte, int numBytes, int value);

    std::string getStringValue(int firstByte, int lastByte) const;
    void setStringValue(int firstByte, int lastByte, const std::string &newValue);
};
