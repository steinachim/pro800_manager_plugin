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

juce::String Pro800MidiMessage::toString() const
{
    return "Pro800 SysEx Message: " + juce::String::toHexString(rawData, rawDataSize);
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

uint8_t Pro800MidiMessage::getUint8Value(uint16_t setting, uint16_t overflow, uint8_t overflowBit) const
{
    if ( !isValid() )
    {
        return 0;
    }

    uint8_t value = getRawData()[POS_MESSAGE_START + setting];
    if ( overflow != OVERFLOW_BYTE_NONE && overflowBit != OVERFLOW_BIT_NONE )
    {
        uint8_t overflowValue = getRawData()[POS_MESSAGE_START + overflow];
        uint8_t overflowBitValue = (overflowValue & (1 << overflowBit)) ? 1 : 0;
        value |= (overflowBitValue << 7);
    }

    return value;
}

void Pro800MidiMessage::setUint8Value(uint16_t setting, uint8_t value)
{
    setUint8Value(setting, OVERFLOW_BYTE_NONE, OVERFLOW_BIT_NONE, value);
}

void Pro800MidiMessage::setUint8Value(uint16_t setting, uint16_t overflow, uint8_t overflowBit, uint8_t value)
{
    // TODO: implement builder pattern for setting
    if ( !isValid() )
    {
        return;
    }

    if ( overflow != OVERFLOW_BYTE_NONE && overflowBit != OVERFLOW_BIT_NONE )
    {
        uint8_t overflowValue = this->getRawData()[POS_MESSAGE_START + overflow];
        uint8_t overflowBitValue = (value & 0x80) ? 1 : 0; // bit 7 of value
        // clear bit, then set if required
        overflowValue &= ~(1 << overflowBit);
        overflowValue |= (overflowBitValue << overflowBit);

        this->getRawData()[POS_MESSAGE_START + overflow] = overflowValue;

        // clear bit 7 of value
        value &= 0x7f;
    }

    this->getRawData()[POS_MESSAGE_START + setting] = value;
}

uint16_t Pro800MidiMessage::getUint16Value(uint16_t msb, uint16_t lsb, const std::vector<uint16_t> &overflowBytes, const std::vector<uint8_t> &overflowBits) const
{
    if ( !isValid() )
    {  
        std::cerr << "ERROR: Trying to get value from invalid message!" << std::endl;      
        return 0;
    }

    if ( overflowBits.size() < 1 )
    {
        std::cerr << "ERROR: To few overflow bits selected!" << std::endl;
        return 0;
    }

    if ( overflowBytes.size() < 1 )
    {
        std::cerr << "ERROR: To few overflow bytes selected!" << std::endl;
        return 0;
    }

    if ( overflowBytes.size() != overflowBits.size() )
    {
        std::cerr << "ERROR: Mismatch between overflow bytes and bits vectors" << std::endl;
        return 0;
    }

    uint8_t lsbValue = getUint8Value(lsb);
    uint8_t msbValue = getUint8Value(msb);
    uint8_t overflowByte1 = getUint8Value(overflowBytes[0]);

    
    uint8_t overflowValue1 = (overflowByte1 & (1 << overflowBits.at(0))) ? 1 : 0;

    uint16_t resultValue = lsbValue;
    resultValue |= (overflowValue1 << 7);
    resultValue |= ((uint16_t)msbValue << 8);

    if ( overflowBits.size() > 1)
    {
        uint8_t overflowByte2 = getUint8Value(overflowBytes[0]);
        uint8_t overflowValue2 = (overflowByte2 & (1 << overflowBits.at(1))) ? 1 : 0;
        
        resultValue |= ((uint16_t)overflowValue2 << 15);
    }

    return resultValue;
   
}

void Pro800MidiMessage::setUint16Value(uint16_t msb, uint16_t lsb, const std::vector<uint16_t> &overflowBytes, const std::vector<uint8_t> &overflowBits, uint16_t value)
{
    if ( !isValid() )
    {
        return;
    }

    if ( overflowBytes.size() < 1 )
    {
        std::cerr << "ERROR: To few overflow bytes selected!" << std::endl;
        return;
    }

    if ( overflowBytes.size() != overflowBits.size() )
    {
        std::cerr << "ERROR: Mismatch between overflow bytes and bits vectors" << std::endl;
        return;
    }

    uint8_t lsbValue = value & 0x007f;
    uint8_t msbValue = ((value & 0x7f00) >> 8);

    setUint8Value(lsb, lsbValue);
    setUint8Value(msb, msbValue);

    for ( uint8_t i = 0; i < (uint8_t)overflowBits.size(); i++)
    {
        uint8_t overflowValue = getUint8Value(overflowBytes.at(i));
        uint8_t overflowBitPosition = overflowBits.at(i);
        uint8_t valueBitPosition = 8*(i+1)-1; // (e.g. 0x80 -> 7; or 0x8000 -> 15)
        uint8_t bitValue = (value & (1 << valueBitPosition)) ? 1 : 0;
        
        // clear bit, then set if required
        overflowValue &= ~(1 << overflowBitPosition);
        overflowValue |= (bitValue << overflowBitPosition);

        setUint8Value(overflowBytes.at(i), overflowValue);
    }
}

uint16_t Pro800MidiMessage::getUint16Value (const Parameter16Bit& param) const
{
    return getUint16Value(param.msb, param.lsb, param.overflowBytes, param.overflowBits);
}

void Pro800MidiMessage::setUint16Value (const Parameter16Bit& param, uint16_t value)
{
    setUint16Value(param.msb, param.lsb, param.overflowBytes, param.overflowBits, value);
}