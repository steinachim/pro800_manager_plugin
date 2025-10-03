#include "Pro800MidiMessage.h"

#include <cstring>

Pro800MidiMessage::Pro800MidiMessage(const juce::MidiMessage &message) : Pro800MidiMessage(message.getRawData(), message.getRawDataSize())
{
}

Pro800MidiMessage::Pro800MidiMessage(const uint8_t *newRawData, int newRawDataSize)
{
    this->rawData = std::make_unique<std::vector<uint8_t>>();
    this->rawData->insert(this->rawData->end(), &newRawData[0], &newRawData[newRawDataSize]);  
}


Pro800MidiMessage::~Pro800MidiMessage()
{

}

juce::String Pro800MidiMessage::toString() const
{
    return "Pro800 SysEx Message: " + juce::String::toHexString(rawData->data(), rawData->size());
}

std::shared_ptr<juce::MidiMessage> Pro800MidiMessage::toMidiMessage() const
{
    return std::shared_ptr<juce::MidiMessage>(new juce::MidiMessage(rawData->data(), rawData->size()));
}

uint8_t *Pro800MidiMessage::getRawData() const
{
    return this->rawData->data();
}

int Pro800MidiMessage::getRawDataSize() const
{
    return this->rawData->size();
}

bool Pro800MidiMessage::isValid() const
{
    return this->isPro800Header() && this->isCorrectResponse();
}

bool Pro800MidiMessage::isCorrectResponse() const
{
    return (this->getRawDataSize() > POS_MESSAGE_TYPE && this->rawData->at(POS_MESSAGE_TYPE) == getResponseType()) || getResponseType() == RESPONSE_UNINIT;
}

unsigned char Pro800MidiMessage::getResponseType() const
{
    return RESPONSE_UNINIT;
}

bool Pro800MidiMessage::isPro800Header() const
{
    if (this->getRawDataSize() < POS_MESSAGE_TYPE)
        return false;

    // is it sysex?
    if ( this->rawData->at(0) != 0xF0 ) 
        return false;
    
    for( int i = 1; i < 7; i++ )
    {
        if ( this->rawData->at(i) != PRO800_HEADER[i-1] )
            return false;
    }

    return true;
}

uint8_t Pro800MidiMessage::getUint8Value(uint16_t setting) const
{
    if ( !isValid() )
    {
        return 0;
    }

    return getRawData()[POS_MESSAGE_START + setting];
}

void Pro800MidiMessage::setUint8Value(uint16_t setting, uint8_t value)
{
    if ( !isValid() )
    {
        return;
    }

    this->getRawData()[POS_MESSAGE_START + setting] = value;
}

int Pro800MidiMessage::getValue(uint16_t firstByte, uint8_t numBytes, bool isSigned) const
{
    if ( numBytes > 4 )
    {
        std::cerr << "Pro800MidiMessage::getValue() only implemented for maximum of 4 byte values" << std::endl;
        return 0;
    }
    const uint8_t pos_offset = 10;

    int value = 0;
    int skippedBytes = 0;
    for ( int i = 0; i < numBytes; i++ )
    {
        uint16_t offset_byte = firstByte + i - pos_offset;
        if ( offset_byte % 8 == 0 )
        {
            // this is an overflow byte - skip to next
            skippedBytes++;
        }

        offset_byte += skippedBytes;
        uint8_t byteValue = this->getUint8Value(offset_byte + pos_offset);

        
        uint16_t overflowByte = (offset_byte / 8) * 8 + pos_offset;
        uint8_t overflowBit = (offset_byte % 8) - 1;

        uint8_t overflowValue = this->getUint8Value(overflowByte);
        overflowValue = (overflowValue & (1 << overflowBit)) ? 1 : 0;

        byteValue = byteValue | (overflowValue << 7);

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

void Pro800MidiMessage::setValue(uint16_t firstByte, uint8_t numBytes, int value)
{
    if ( numBytes > 4 )
    {
        std::cerr << "Pro800MidiMessage::setValue() only implemented for maximum of 4 byte values" << std::endl;
        return;
    }
    const uint8_t pos_offset = 10;

    int skippedBytes = 0;
    for ( int i = 0; i < numBytes; i++ )
    {
        uint8_t byteValue = (value >> i*8) & 0xFF;
        uint8_t overflowBitValue = (byteValue & 0x80) >> 7;
        byteValue &= 0x7F; // limit to 127, highest byte can never be set in sysex and is covered by overflowBitValue

        uint16_t offset_byte = firstByte + i - pos_offset;
        if ( offset_byte % 8 == 0 )
        {
            // this is an overflow byte - skip to next
            skippedBytes++;
        }

        offset_byte += skippedBytes;
        this->setUint8Value(offset_byte + pos_offset, byteValue);

        uint16_t overflowByte = (offset_byte / 8) * 8 + pos_offset;
        uint8_t overflowBit = (offset_byte % 8) - 1;

        uint8_t overflowValue = this->getUint8Value(overflowByte);

        // clear bit, then set if required
        overflowValue &= ~(1 << overflowBit);
        overflowValue |= (overflowBitValue << overflowBit);

        setUint8Value(overflowByte, overflowValue);
    }
}
