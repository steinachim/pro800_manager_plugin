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

    return Pro800DataMessage::request(programLSB, programMSB);
}

ProgramMessage::ProgramMessage() : Pro800DataMessage()
{
}

ProgramMessage::ProgramMessage(const juce::MidiMessage &message) : ProgramMessage(message.getRawData(), message.getRawDataSize())
{
}

ProgramMessage::ProgramMessage(const uint8_t *newRawData, int newRawDataSize) : Pro800DataMessage(newRawData, newRawDataSize)
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
}

ProgramMessage::ProgramMessage(const ProgramMessage &other) : Pro800DataMessage(other)
{
}

bool ProgramMessage::isValid() const
{
    if ( !Pro800MidiMessage::isValid() )
        return false;

    if ( getRawDataSize() <= DATA_START_POS + PROGRAM_FIELD_VERSION )
        return false;

    return true;
}

uint16_t ProgramMessage::getProgramNumber() const
{
    uint8_t programLSB = (uint8_t)getUint8Value(ADDRESS_LSB_POS);
    uint8_t programMSB = (uint8_t)getUint8Value(ADDRESS_MSB_POS);
    return (uint16_t)((programMSB << 7) | programLSB);
}

std::string ProgramMessage::getProgramBankNumber() const
{
    uint16_t programNumber = getProgramNumber();
    uint8_t bank = (uint8_t)(programNumber / 100); // 0-3 = A-D
    uint8_t program = (uint8_t)(programNumber % 100); // 0-99

    char bankName = 'A' + (char)bank;

    std::stringstream ss;
    ss << bankName;
    ss << std::setfill('0') << std::setw(2) << (int)program;
    return ss.str();
}

void ProgramMessage::setProgramNumber(uint16_t programNumber)
{
    uint8_t programLSB = programNumber & 0x7F;
    uint8_t programMSB = (programNumber >> 7) & 0x7F;
    setUint8Value(ADDRESS_LSB_POS, programLSB);
    setUint8Value(ADDRESS_MSB_POS, programMSB);
}

std::string ProgramMessage::getProgramName() const
{
    if ( !isValid() )
    {
        return "--- Uninitialized ---";
    }

    size_t firstByte = PRO800_PROGRAM_FIELDS.at(PROGRAM_FIELD_NAME_FIRST_CHAR).firstByte;
    size_t lastByte = PRO800_PROGRAM_FIELDS.at(PROGRAM_FIELD_NAME_LAST_CHAR).firstByte;
    return getStringValue(firstByte, lastByte);
}

void ProgramMessage::setProgramName(const std::string &newName)
{
    size_t firstByte = PRO800_PROGRAM_FIELDS.at(PROGRAM_FIELD_NAME_FIRST_CHAR).firstByte;
    size_t lastByte = PRO800_PROGRAM_FIELDS.at(PROGRAM_FIELD_NAME_LAST_CHAR).firstByte;
    setStringValue(firstByte, lastByte, newName);
}

bool ProgramMessage::isLfoDestinationEnabled(Pro800ProgramLfoDestinationBitMask destination) const
{
    const uint8_t lfoDestinations = (uint8_t)getValue(PROGRAM_FIELD_LFO_DEST);
    return lfoDestinations & destination;
}

void ProgramMessage::setLfoDestinationEnabled(Pro800ProgramLfoDestinationBitMask destination, bool enabled)
{
    uint8_t lfoDestinations = (uint8_t)getValue(PROGRAM_FIELD_LFO_DEST);
    uint8_t targetValue = (enabled ? destination : 0);

    lfoDestinations = (lfoDestinations & ~destination) | targetValue;
    setValue (PROGRAM_FIELD_LFO_DEST, lfoDestinations);
}

int ProgramMessage::getLfoDestinationValue (Pro800CCMessages ccNumber) const
{
    int value = 0;

    switch ((int)ccNumber)
    {
        case CC_LFO_MOD_DEST_FREQ_AB:
            value = isLfoDestinationEnabled (PROGRAM_LFO_DEST_FREQ_AB) ? CC_ON : CC_OFF;
            break;

        case CC_LFO_MOD_DEST_PW_AB:
            value = isLfoDestinationEnabled (PROGRAM_LFO_DEST_PW_AB) ? CC_ON : CC_OFF;
            break;

        case CC_LFO_MOD_DEST_FILTER:
            value = isLfoDestinationEnabled (PROGRAM_LFO_DEST_FILTER) ? CC_ON : CC_OFF;
            break;

        case CC_LFO_TARGET:
            value = CC_LFO_TARGET_OSC_AB;
            if (isLfoDestinationEnabled (PROGRAM_LFO_DEST_FREQ_A))
            {
                value = CC_LFO_TARGET_OSC_A;
            }
            else if (isLfoDestinationEnabled (PROGRAM_LFO_DEST_FREQ_B))
            {
                value = CC_LFO_TARGET_OSC_B;
            }
            else if (isLfoDestinationEnabled (PROGRAM_LFO_DEST_FREQ_AB_VCA))
            {
                value = CC_LFO_TARGET_VCA;
            }
            break;

        default:
            juce::Logger::writeToLog("ProgramMessage::getLfoDestinationValue(): Unsupported CC number: " + juce::String((int)ccNumber));
            break;
    }
    return value;
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
        return Pro800DataMessage::getValue(param.firstByte, param.numBytes, param.isSigned);
    }
    else
    {
        juce::Logger::writeToLog("ProgramMessage::getValue(): No getter for field defined: " + juce::String((int)field));
    }

    return 0;
}

void ProgramMessage::setValue(Pro800ProgramField field, int value)
{
    if ( PRO800_PROGRAM_FIELDS.contains(field) )
    {
        Pro800Parameter param = PRO800_PROGRAM_FIELDS.at(field);
        Pro800DataMessage::setValue(param.firstByte, param.numBytes, value);
    }
    else
    {
        juce::Logger::writeToLog("ProgramMessage::setValue(): No setter for field defined: " + juce::String((int)field));
    }    
}
