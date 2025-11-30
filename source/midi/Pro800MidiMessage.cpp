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

#include <cstring>
#include <memory>

Pro800MidiMessage::Pro800MidiMessage(const juce::MidiMessage &message) : Pro800MidiMessage(message.getRawData(), message.getRawDataSize())
{
}

Pro800MidiMessage::Pro800MidiMessage(const uint8_t *newRawData, int newRawDataSize)
{
    this->rawData = std::make_unique<std::vector<uint8_t>>();
    this->rawData->insert(this->rawData->end(), &newRawData[0], &newRawData[newRawDataSize]);  
}

Pro800MidiMessage::Pro800MidiMessage(const Pro800MidiMessage &other)
{
    this->rawData = std::make_unique<std::vector<uint8_t>>();
    this->rawData->insert(this->rawData->end(), other.rawData->begin(), other.rawData->end());
}


Pro800MidiMessage::~Pro800MidiMessage()
{

}

juce::String Pro800MidiMessage::toString() const
{
    return "Pro800 SysEx Message: " + juce::String::toHexString(rawData->data(), (int)rawData->size());
}

std::shared_ptr<juce::MidiMessage> Pro800MidiMessage::toMidiMessage() const
{
    return std::make_shared<juce::MidiMessage>(rawData->data(), (int)rawData->size());
}

std::shared_ptr<std::vector<uint8_t>> &Pro800MidiMessage::getRawData()
{
    return this->rawData;
}

size_t Pro800MidiMessage::getRawDataSize() const
{
    return this->rawData->size();
}

bool Pro800MidiMessage::isValid() const
{
    if( this->rawData->size() <= POS_MESSAGE_TYPE )// long enough to at least have a response type?
        return false;

    if( this->rawData->at(0) != 0xF0                       // valid sysex start
           && this->rawData->at(this->rawData->size()-1) != 0xF7 )// valid sysex end
        return false;

    if ( !std::equal(PRO800_HEADER.begin(), PRO800_HEADER.end(), this->rawData->begin()+1) ) // valid Pro800 header
        return false;

    if ( !this->isCorrectResponse()) // valid response to query
        return false;

    return true;
}

bool Pro800MidiMessage::isCorrectResponse() const
{
    return (this->rawData->at(POS_MESSAGE_TYPE) == getResponseType()) || getResponseType() == RESPONSE_UNINIT;
}

unsigned char Pro800MidiMessage::getResponseType() const
{
    return RESPONSE_UNINIT;
}

uint8_t Pro800MidiMessage::getUint8Value(size_t position) const
{
    // low-level function: ignore validity check
    if ( position >= getRawDataSize() )
    {
        juce::Logger::writeToLog("Pro800MidiMessage::getUint8Value() - reading out of range. Returning 0!");
        return 0;
    }

    return this->rawData->at(position);
}

void Pro800MidiMessage::setUint8Value(size_t position, uint8_t value)
{
    // low-level function: ignore validity check
    if ( position >= getRawDataSize() ) 
    {
        juce::Logger::writeToLog("Pro800MidiMessage::setUint8Value() - cannot set value outside of data range!");
        return;
    }

    this->rawData->at(position) = value;
}
