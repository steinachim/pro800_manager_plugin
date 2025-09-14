#include "Pro800MidiMessage.h"

#include <cstring>

Pro800MidiMessage::Pro800MidiMessage(const juce::MidiMessage &message)
{
    const int numBytes = message.getRawDataSize();
    
    this->rawData = new uint8_t[(size_t)numBytes];
    std::memcpy(rawData, message.getRawData(), (size_t)numBytes);
    this->rawDataSize = numBytes;
}

Pro800MidiMessage::~Pro800MidiMessage()
{
    delete this->rawData;
}

std::string Pro800MidiMessage::toString() const
{
    return "Pro800 SysEx Message (unknown)";
}

std::shared_ptr<juce::MidiMessage> Pro800MidiMessage::toMidiMessage() const
{
    return std::shared_ptr<juce::MidiMessage>(new juce::MidiMessage(rawData, rawDataSize));
}

uint8_t *Pro800MidiMessage::getRawData() const
{
    return this->rawData;
}

int Pro800MidiMessage::getRawDataSize() const
{
    return this->rawDataSize;
}

bool Pro800MidiMessage::isValid() const
{
    return this->isPro800Header() && this->isCorrectResponse();
}

bool Pro800MidiMessage::isCorrectResponse() const
{
    return (this->rawDataSize > POS_MESSAGE_TYPE && this->rawData[POS_MESSAGE_TYPE] == getResponseType()) || getResponseType() == RESPONSE_UNINIT;
}

unsigned char Pro800MidiMessage::getResponseType() const
{
    return RESPONSE_UNINIT;
}

bool Pro800MidiMessage::isPro800Header() const
{
    if (this->rawDataSize < POS_MESSAGE_TYPE)
        return false;

    // is it sysex?
    if ( this->rawData[0] != 0xF0 ) 
        return false;
    
    for( int i = 1; i < 7; i++ )
    {
        if ( this->rawData[i] != PRO800_HEADER[i-1] )
            return false;
    }

    return true;
}