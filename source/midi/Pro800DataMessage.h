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

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "Pro800MidiMessage.h"

class Pro800DataMessage : public Pro800MidiMessage
{
public:
    static constexpr uint8_t REQUEST_ID = 0x77;
    static constexpr uint8_t RESPONSE_ID = 0x78;

    static constexpr size_t ADDRESS_LSB_POS = 0x09;
    static constexpr size_t ADDRESS_MSB_POS = 0x0A;
    static constexpr size_t DATA_START_POS = 0x0B;

    // clang-format off
    static inline const std::vector<uint8_t> EMPTY_MESSAGE = {
        0xF0,
        0x00, 0x20, 0x32,      // Brand ID (Behringer)
        0x00, 0x01, 0x24,      // Product ID (Pro-800)
        0x00,                  // CPU ID,
        RESPONSE_ID,           // command
        0x7F, 0x7F,            // program number (invalid)
        0xF7
    };
    // clang-format on

    static juce::MidiMessage request (uint8_t addressLSB, uint8_t addressMSB);

    Pro800DataMessage();
    explicit Pro800DataMessage (const juce::MidiMessage& message);
    Pro800DataMessage (const uint8_t* newRawData, int newRawDataSize);

protected:
    uint8_t getResponseType() const override;

    int getValue (size_t firstByte, size_t numBytes, bool isSigned = false) const;
    void setValue (size_t firstByte, size_t numBytes, int value);

    std::string getStringValue (size_t firstByte, size_t lastByte) const;
    void setStringValue (size_t firstByte, size_t lastByte, const std::string& newValue);

    // Field-table based access, shared by the program and settings messages. FieldEnum is the
    // message's field enum, fields the table that maps it to byte positions.

    template <typename FieldEnum>
    int getFieldValue (const std::map<FieldEnum, Pro800Parameter>& fields, FieldEnum field) const
    {
        const auto entry = fields.find (field);
        if (entry == fields.end())
        {
            juce::Logger::writeToLog ("Pro800DataMessage::getFieldValue(): no field defined for " + juce::String ((int) field));
            return 0;
        }

        const Pro800Parameter& param = entry->second;
        return getValue (param.firstByte, param.numBytes, param.isSigned);
    }

    template <typename FieldEnum>
    void setFieldValue (const std::map<FieldEnum, Pro800Parameter>& fields, FieldEnum field, int value)
    {
        const auto entry = fields.find (field);
        if (entry == fields.end())
        {
            juce::Logger::writeToLog ("Pro800DataMessage::setFieldValue(): no field defined for " + juce::String ((int) field));
            return;
        }

        const Pro800Parameter& param = entry->second;
        setValue (param.firstByte, param.numBytes, value);
    }

    /** One line per field: "<name>: <raw value> (display: <value scaled to the synth's 0-999 display>)". */
    template <typename FieldEnum>
    juce::String fieldsToString (const std::map<FieldEnum, Pro800Parameter>& fields) const
    {
        juce::String result;
        for (const auto& [field, param] : fields)
        {
            const int value = getValue (param.firstByte, param.numBytes, param.isSigned);
            const double maxValue = std::pow (256.0, param.numBytes) - 1.0; // double: 4-byte fields would overflow int
            const int displayValue = juce::roundToInt (value * 999.0 / maxValue);

            result << param.name << ": " << value << " (display: " << displayValue << ")\n";
        }
        return result;
    }
};
