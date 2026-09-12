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

#include "Pro800MidiMessage.h"

#include <algorithm>

Pro800MidiMessage::Pro800MidiMessage (const juce::MidiMessage& message) : Pro800MidiMessage (message.getRawData(), message.getRawDataSize())
{
}

Pro800MidiMessage::Pro800MidiMessage (const uint8_t* newRawData, int newRawDataSize)
{
    if (newRawData != nullptr && newRawDataSize > 0)
    {
        this->rawData.assign (newRawData, newRawData + newRawDataSize);
    }
}

juce::MidiMessage Pro800MidiMessage::makeRequest (std::initializer_list<uint8_t> payload)
{
    std::vector<uint8_t> request;
    request.reserve (PRO800_HEADER.size() + payload.size());
    request.insert (request.end(), PRO800_HEADER.begin(), PRO800_HEADER.end());
    request.insert (request.end(), payload.begin(), payload.end());
    return juce::MidiMessage::createSysExMessage (request.data(), (int) request.size());
}

juce::String Pro800MidiMessage::toString() const
{
    return "Pro800 SysEx Message: " + juce::String::toHexString (rawData.data(), (int) rawData.size());
}

juce::MidiMessage Pro800MidiMessage::toMidiMessage() const
{
    return juce::MidiMessage (rawData.data(), (int) rawData.size());
}

const std::vector<uint8_t>& Pro800MidiMessage::getRawData() const
{
    return this->rawData;
}

size_t Pro800MidiMessage::getRawDataSize() const
{
    return this->rawData.size();
}

bool Pro800MidiMessage::isDataPosition (size_t position) const
{
    return position > 0 && position + 1 < this->rawData.size();
}

// clang-format off
bool Pro800MidiMessage::isValid() const
{
    if( this->rawData.size() <= POS_MESSAGE_TYPE )// long enough to at least have a response type?
        return false;

    if( this->rawData.front() != 0xF0                      // valid sysex start
           || this->rawData.back() != 0xF7 )               // valid sysex end
        return false;

    if ( !std::equal(PRO800_HEADER.begin(), PRO800_HEADER.end(), this->rawData.begin()+1) ) // valid Pro800 header
        return false;

    if ( !this->isCorrectResponse()) // valid response to query
        return false;

    return true;
}
// clang-format on

bool Pro800MidiMessage::isCorrectResponse() const
{
    return (this->rawData.at (POS_MESSAGE_TYPE) == getResponseType()) || getResponseType() == RESPONSE_UNINIT;
}

uint8_t Pro800MidiMessage::getResponseType() const
{
    return RESPONSE_UNINIT;
}

uint8_t Pro800MidiMessage::getUint8Value (size_t position) const
{
    // low-level function: ignore validity check
    if (position >= getRawDataSize())
    {
        juce::Logger::writeToLog ("Pro800MidiMessage::getUint8Value() - reading out of range. Returning 0!");
        return 0;
    }

    return this->rawData[position];
}

void Pro800MidiMessage::setUint8Value (size_t position, uint8_t value)
{
    // low-level function: ignore validity check
    if (position >= getRawDataSize())
    {
        juce::Logger::writeToLog ("Pro800MidiMessage::setUint8Value() - cannot set value outside of data range!");
        return;
    }

    this->rawData[position] = value;
}

void Pro800MidiMessage::resizeRawData (size_t newSize)
{
    this->rawData.resize (newSize, 0);
}
