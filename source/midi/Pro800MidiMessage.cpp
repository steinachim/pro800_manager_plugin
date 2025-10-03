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

uint8_t Pro800MidiMessage::getUint8Value(uint16_t setting, const std::vector<uint16_t> &overflow, const std::vector<uint8_t> &overflowBit) const
{
    if ( !isValid() )
    {
        return 0;
    }

    uint8_t value = getRawData()[POS_MESSAGE_START + setting];
    if ( !overflow.empty() && !overflowBit.empty() )
    {
        uint8_t overflowValue = getRawData()[POS_MESSAGE_START + overflow[0]];
        uint8_t overflowBitValue = (overflowValue & (1 << overflowBit[0])) ? 1 : 0;
        value |= (overflowBitValue << 7);
    }

    return value;
}

void Pro800MidiMessage::setUint8Value(uint16_t setting, uint8_t value)
{
    setUint8Value(setting, {}, {}, value);
}

void Pro800MidiMessage::setUint8Value(uint16_t setting, const std::vector<uint16_t> &overflow, const std::vector<uint8_t> &overflowBit, uint8_t value)
{
    // TODO: implement builder pattern for setting
    if ( !isValid() )
    {
        return;
    }

    if ( !overflow.empty() && !overflowBit.empty() )
    {
        uint8_t overflowValue = this->getRawData()[POS_MESSAGE_START + overflow[0]];
        uint8_t overflowBitValue = (value & 0x80) ? 1 : 0; // bit 7 of value
        // clear bit, then set if required
        overflowValue &= ~(1 << overflowBit[0]);
        overflowValue |= (overflowBitValue << overflowBit[0]);

        this->getRawData()[POS_MESSAGE_START + overflow[0]] = overflowValue;

        // clear bit 7 of value
        value &= 0x7f;
    }

    this->getRawData()[POS_MESSAGE_START + setting] = value;
}

uint16_t Pro800MidiMessage::getUint16Value(uint16_t lsb, uint16_t msb, const std::vector<uint16_t> &overflowBytes, const std::vector<uint8_t> &overflowBits) const
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


    /*std::cout << "getUint16Value(lsb = " << lsb << ", msb = " << msb << ", ";
    for ( size_t i = 0; i < overflowBytes.size(); i++ )
    {
        std::cout << "overflowByte[" << i << "] = " << (int)overflowBytes[i] << ", overflowBit[" << i << "] = " << (int)overflowBits[i] << ", ";
    }
    std::cout << ")\n";
    
    uint8_t num_bytes = 2;
    uint8_t pos_offset = 10;
    int bytes[] = {lsb, lsb + num_bytes -1 };
    
    std::cout << "--> calculated(lsb = " << bytes[0] << ", msb = " << bytes[1] << ", ";
    for ( int i = 0; i < num_bytes; i++ )
    {
        uint8_t offset_byte = bytes[i] - pos_offset;
        uint8_t overflowByte = (offset_byte / 8) * 8 + pos_offset;
        uint8_t overflowBit = (offset_byte % 8) - 1;

        std::cout << "overflowByte[" << i << "] = " << (int)overflowByte << ", overflowBit[" << i << "] = " << (int)overflowBit << ", ";
    }
    std::cout << ")\n";*/



    uint8_t lsbValue = getUint8Value(lsb);
    uint8_t msbValue = getUint8Value(msb);
    uint8_t overflowByte1 = getUint8Value(overflowBytes[0]);

    uint8_t overflowValue1 = (overflowByte1 & (1 << overflowBits.at(0))) ? 1 : 0;

    uint16_t resultValue = lsbValue;
    resultValue |= (overflowValue1 << 7);
    resultValue |= ((uint16_t)msbValue << 8);

    if ( overflowBits.size() > 1)
    {
        uint8_t overflowByte2 = getUint8Value(overflowBytes[1]);
        uint8_t overflowValue2 = (overflowByte2 & (1 << overflowBits.at(1))) ? 1 : 0;
        
        resultValue |= ((uint16_t)overflowValue2 << 15);
    }

    return resultValue;
   
}

int Pro800MidiMessage::getValue(uint16_t firstByte, uint8_t numBytes) const
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

void Pro800MidiMessage::setUint16Value(uint16_t lsb, uint16_t msb, const std::vector<uint16_t> &overflowBytes, const std::vector<uint8_t> &overflowBits, uint16_t value)
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
