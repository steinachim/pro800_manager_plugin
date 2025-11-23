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

#include "Pro800DataMessage.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

juce::MidiMessage Pro800DataMessage::request(uint8_t addressLSB, uint8_t addressMSB)
{
    std::vector<uint8_t> request;
    request.insert(request.end(), std::begin(PRO800_HEADER), std::end(PRO800_HEADER));
    request.insert(request.end(), {REQUEST_ID, addressLSB, addressMSB});
    return juce::MidiMessage::createSysExMessage(request.data(), (int)request.size());
}


Pro800DataMessage::Pro800DataMessage() : Pro800MidiMessage(EMPTY_MESSAGE.data(), (int)EMPTY_MESSAGE.size())
{
}

Pro800DataMessage::Pro800DataMessage(const juce::MidiMessage &message) : Pro800DataMessage(message.getRawData(), message.getRawDataSize())
{
}

Pro800DataMessage::Pro800DataMessage(const uint8_t *newRawData, int newRawDataSize) : Pro800MidiMessage(newRawData, newRawDataSize)
{
}

Pro800DataMessage::Pro800DataMessage(const Pro800DataMessage &other) : Pro800MidiMessage(other)
{
}

unsigned char Pro800DataMessage::getResponseType() const
{
    return RESPONSE_ID;
}

int Pro800DataMessage::getValue(int firstByte, int numBytes, bool isSigned) const
{
    if ( !isValid() )
    {
        return 0;
    }

    if ( numBytes > 4 )
    {
        std::cerr << "Pro800DataMessage::getValue() only implemented for maximum of 4 byte values" << std::endl;
        return 0;
    }

    int value = 0;
    int skippedBytes = 0;
    for ( int i = 0; i < numBytes; i++ )
    {
        int offset_byte = firstByte + i;;
        if ( offset_byte % 8 == 0 )
        {
            // this is an overflow byte - skip to next
            skippedBytes++;
        }

        offset_byte += skippedBytes;
        uint8_t byteValue = this->getUint8Value(DATA_START_POS + offset_byte);

        
        int overflowByte = (offset_byte / 8) * 8;
        uint8_t overflowBit = (uint8_t)((offset_byte % 8) - 1);

        uint8_t overflowValue = this->getUint8Value(DATA_START_POS + overflowByte);
        overflowValue = (overflowValue & (1 << overflowBit)) ? 1 : 0;

        byteValue = byteValue | (uint8_t)(overflowValue << 7);

        value = value | (byteValue << i*8);

        if( isSigned && overflowValue == 1 )
        {
            // propagate highest bit to top bytes           
            for ( int j = i+1; j < 4; j++ )
            {
                value = value | (0xFF << j*8);
            }
        }
    }

    return value;
}

void Pro800DataMessage::setValue(int firstByte, int numBytes, int value)
{
    if ( !isValid() )
    {
        std::cerr << "Pro800DataMessage::setValue() - cannot set value on invalid message!" << std::endl;
        return;
    }

    if ( numBytes > 4 )
    {
        std::cerr << "Pro800DataMessage::setValue() only implemented for maximum of 4 byte values" << std::endl;
        return;
    }

    int skippedBytes = 0;
    for ( int i = 0; i < numBytes; i++ )
    {
        uint8_t byteValue = (value >> i*8) & 0xFF;
        uint8_t overflowBitValue = (byteValue & 0x80) >> 7;
        byteValue &= 0x7F; // limit to 127, highest byte can never be set in sysex and is covered by overflowBitValue

        int offset_byte = firstByte + i;
        if ( offset_byte % 8 == 0 )
        {
            // this is an overflow byte - skip to next
            skippedBytes++;
        }

        offset_byte += skippedBytes;
        this->setUint8Value(DATA_START_POS + offset_byte, byteValue);

        int overflowByte = (offset_byte / 8) * 8;
        uint8_t overflowBit = (uint8_t)((offset_byte % 8) - 1);

        uint8_t overflowValue = this->getUint8Value(DATA_START_POS + overflowByte);

        // clear bit, then set if required
        overflowValue &= ~(1 << overflowBit);
        overflowValue |= (overflowBitValue << overflowBit);

        setUint8Value(DATA_START_POS + overflowByte, overflowValue);
    }
}

std::string Pro800DataMessage::getStringValue(int firstByte, int lastByte) const
{
    if ( !isValid() )
    {
        return std::string();
    }

    std::string value = "";
    for ( int pos = firstByte; pos <= lastByte; pos++)
    {
        if ( pos % 8 == 0 )
        {
            // this is an overflow byte. Skip.
            continue; 
        }

        char nameChar = (char)getUint8Value(DATA_START_POS + pos);
        // sanitize non-printable characters
        /*if ( nameChar < 32 || nameChar > 126 )
        {
            nameChar = '?';
        }*/

        value += nameChar;
    }
    
    // remove potential trailing null values
    value.erase(std::remove(value.begin(), value.end(), 0x00), value.end());

    return value;
}



void Pro800DataMessage::setStringValue(int firstByte, int lastByte, const std::string &newValue)
{
    if ( !isValid() )
    {
        std::cerr << "Pro800MidiMessage::setStringValue() - cannot set value on invalid message!" << std::endl;
        return;
    }

    // resize new value to full range
    std::string resizedValue = newValue;
    resizedValue.resize((size_t)(lastByte - firstByte + 1), 0x00);

    int numOverflowBytes = 0;
    for ( int pos = firstByte; pos <= lastByte; pos++ )
    {
        if ( pos % 8 == 0 )
        {
            // this is an overflow byte. Skip.
            numOverflowBytes++;
            pos++;
        }
        
        setUint8Value(DATA_START_POS + pos, (uint8_t)resizedValue[(size_t)(pos - firstByte - numOverflowBytes)]);
    }
}
