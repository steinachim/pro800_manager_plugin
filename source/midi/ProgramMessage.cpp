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

#include "ProgramMessage.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

juce::MidiMessage ProgramMessage::request(int programNumber)
{
    uint8_t programLSB = (programNumber & 0x7F);
    uint8_t programMSB = (uint8_t)(programNumber >> 7);

    std::vector<uint8_t> request;
    request.insert(request.end(), std::begin(PRO800_HEADER), std::end(PRO800_HEADER));
    request.insert(request.end(), {REQUEST_ID, programLSB, programMSB});
    return juce::MidiMessage::createSysExMessage(request.data(), (int)request.size());
}

ProgramMessage::ProgramMessage() : Pro800MidiMessage(EMPTY_PROGRAM.data(), (int)EMPTY_PROGRAM.size())
{
}

ProgramMessage::ProgramMessage(const juce::MidiMessage &message) : ProgramMessage(message.getRawData(), message.getRawDataSize())
{
}

ProgramMessage::ProgramMessage(const uint8_t *newRawData, int newRawDataSize) : Pro800MidiMessage(newRawData, newRawDataSize)
{
    // if this message is an older version (pre 111), then update version to 111 and reserve memory accordingly
    if ( newRawDataSize < PROGRAM_MESSAGE_SIZE )
    {   
        // resize data array to new size     
        this->getRawData()->resize(PROGRAM_MESSAGE_SIZE, 0);

        // move 0xF7 from previous last position to new last position
        this->getRawData()->at((size_t)(newRawDataSize-1)) = 0x00;
        this->getRawData()->at(getRawDataSize()-1) = 0xF7;

        // update program version info
        this->setValue(PROGRAM_FIELD_VERSION, SUPPORTED_PRESET_VERSION);
    }

    isInitialized = true;
}

ProgramMessage::ProgramMessage(const ProgramMessage &other) : Pro800MidiMessage(other)
{
    isInitialized = other.isInitialized;
}

bool ProgramMessage::isValid() const
{
    if ( !Pro800MidiMessage::isValid() )
        return false;

    if ( getRawDataSize() <= POS_MESSAGE_START + PROGRAM_FIELD_VERSION )
        return false;

    return true;
}

uint16_t ProgramMessage::getProgramNumber() const
{
    uint8_t programLSB = (uint8_t)getValue(PROGRAM_FIELD_NUM_LSB);
    uint8_t programMSB = (uint8_t)getValue(PROGRAM_FIELD_NUM_MSB);
    return (uint16_t)((programMSB << 7) | programLSB);
}

std::string ProgramMessage::getProgramBankNumber() const
{
    uint16_t programNumber = getProgramNumber();
    uint8_t bank = (uint8_t)(programNumber / 100); // 0-3 = A-D
    uint8_t program = (uint8_t)(programNumber % 100); // 0-99

    char bankName = 'A' + bank;

    std::stringstream ss;
    ss << bankName;
    ss << std::setfill('0') << std::setw(2) << (int)program;
    return ss.str();
}

void ProgramMessage::setProgramNumber(uint16_t programNumber)
{
    uint8_t programLSB = programNumber & 0x7F;
    uint8_t programMSB = (programNumber >> 7) & 0x7F;
    setValue(PROGRAM_FIELD_NUM_LSB, programLSB);
    setValue(PROGRAM_FIELD_NUM_MSB, programMSB);
}

std::string ProgramMessage::getProgramName() const
{
    if ( !isValid() )
    {
        return "--- Uninitialized ---";
    }

    int firstByte = PRO800_PROGRAM_FIELDS.at(PROGRAM_FIELD_NAME_FIRST_CHAR).firstByte;
    int lastByte = PRO800_PROGRAM_FIELDS.at(PROGRAM_FIELD_NAME_LAST_CHAR).firstByte;
    return getStringValue(firstByte, lastByte);
}

void ProgramMessage::setProgramName(const std::string &newName)
{
    int firstByte = PRO800_PROGRAM_FIELDS.at(PROGRAM_FIELD_NAME_FIRST_CHAR).firstByte;
    int lastByte = PRO800_PROGRAM_FIELDS.at(PROGRAM_FIELD_NAME_LAST_CHAR).firstByte;
    setStringValue(firstByte, lastByte, newName);
}

bool ProgramMessage::isLfoDestinationEnabled(Pro800ProgramLfoDestination destination) const
{
    const uint8_t lfoDestinations = (uint8_t)getValue(PROGRAM_FIELD_LFO_DEST);
    return lfoDestinations & destination;
}

void ProgramMessage::setLfoDestinationEnabled(Pro800ProgramLfoDestination destination, bool enabled)
{
    uint8_t lfoDestinations = (uint8_t)getValue(PROGRAM_FIELD_LFO_DEST);
    uint8_t targetValue = (enabled ? destination : 0);

    lfoDestinations = (lfoDestinations & ~destination) | targetValue;
    setValue(PROGRAM_FIELD_LFO_DEST, lfoDestinations);
}

juce::String ProgramMessage::toString() const
{
    std::stringstream ss;
    ss << "Pro800 Program Dump: "
       << getProgramBankNumber() << " - '" << getProgramName() << "'\n";

    for ( auto param : PRO800_PROGRAM_FIELDS )
    {
        int value = getValue(param.first);
        int maxValue = (1 << param.second.numBytes*8) - 1;

        ss << param.second.name << ": " << getValue(param.first) << " (display: " << value * 999 / maxValue << ")\n";
    }
       
    return ss.str();
}

int ProgramMessage::getValue(Pro800ProgramField field) const
{
    if ( PRO800_PROGRAM_FIELDS.contains(field) )
    {
        Pro800Parameter param = PRO800_PROGRAM_FIELDS.at(field);
        return Pro800MidiMessage::getValue(param.firstByte, param.numBytes, param.isSigned);
    }
    else if ( field == PROGRAM_FIELD_NUM )
    {
        return (int)getProgramNumber();
    }
    else
    {
        std::cerr << "ProgramMessage::getValue(): No getter for field defined: " << field << std::endl;
    }

    return 0;
}

void ProgramMessage::setValue(Pro800ProgramField field, int value)
{
    if ( PRO800_PROGRAM_FIELDS.contains(field) )
    {
        Pro800Parameter param = PRO800_PROGRAM_FIELDS.at(field);
        Pro800MidiMessage::setValue(param.firstByte, param.numBytes, value);
    }
    else if ( field == PROGRAM_FIELD_NUM )
    {
        setProgramNumber((uint16_t)value);
    }
    else
    {
        std::cerr << "ProgramMessage::setValue(): No setter for field defined: " << field << std::endl;
    }    
}

unsigned char ProgramMessage::getResponseType() const
{
    return RESPONSE_ID;
}