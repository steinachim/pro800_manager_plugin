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
#include "../Pro800Constants.h"

class Pro800MidiMessage
{
public:
    const uint8_t POS_OFFSET = 10; // TODO: cleanup and remove

    const static inline std::vector<uint8_t> PRO800_HEADER = {
        0x00, 0x20, 0x32,      // Brand ID (Behringer)
        0x00, 0x01, 0x24,      // Product ID (Pro-800)
        0x00                   // CPU ID
    };

    static const int POS_MESSAGE_TYPE = 0x08;
    static const int POS_MESSAGE_START = 0x09;

    static const uint8_t RESPONSE_UNINIT = 0xFF;

    Pro800MidiMessage(const juce::MidiMessage &message);
    Pro800MidiMessage(const uint8_t *newRawData, int newRawDataSize);
    Pro800MidiMessage(const Pro800MidiMessage &other);
    
    virtual ~Pro800MidiMessage();

    virtual MessageType getMessageType() const { return MessageType::PRO800_UNKNOWN_MESSAGE;}

    virtual juce::String toString() const;
    std::shared_ptr<juce::MidiMessage> toMidiMessage() const;

    virtual bool isValid() const;

    std::shared_ptr<std::vector<uint8_t>> &getRawData();
    size_t getRawDataSize() const;

protected:
    int getValue(int firstByte, int numBytes, bool isSigned = false) const;
    void setValue(int firstByte, int numBytes, int value);

    std::string getStringValue(int firstByte, int lastByte) const;
    void setStringValue(int firstByte, int lastByte, const std::string &newValue);

    uint8_t getUint8Value(size_t position) const;
    void setUint8Value(size_t position, uint8_t value);

    virtual unsigned char getResponseType() const;

private:


    bool isCorrectResponse() const;

    std::shared_ptr<std::vector<uint8_t>> rawData;
};
