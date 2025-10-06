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
    return std::shared_ptr<juce::MidiMessage>(new juce::MidiMessage(rawData->data(), (int)rawData->size()));
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
    if ( POS_MESSAGE_START + position >= getRawDataSize() )
    {
        return 0;
    }

    return this->rawData->at(POS_MESSAGE_START + position);
}

void Pro800MidiMessage::setUint8Value(size_t position, uint8_t value)
{
    // low-level function: ignore validity check
    if ( POS_MESSAGE_START + position >= getRawDataSize() ) 
    {
        std::cerr << "Pro800MidiMessage::setUint8Value() - cannot set value outside of data range!" << std::endl;
        return;
    }

    this->rawData->at(POS_MESSAGE_START + position) = value;
}

int Pro800MidiMessage::getValue(int firstByte, int numBytes, bool isSigned) const
{
    if ( !isValid() )
    {
        return 0;
    }

    if ( numBytes > 4 )
    {
        std::cerr << "Pro800MidiMessage::getValue() only implemented for maximum of 4 byte values" << std::endl;
        return 0;
    }

    int value = 0;
    int skippedBytes = 0;
    for ( int i = 0; i < numBytes; i++ )
    {
        int offset_byte = firstByte + i - POS_OFFSET;
        if ( offset_byte % 8 == 0 )
        {
            // this is an overflow byte - skip to next
            skippedBytes++;
        }

        offset_byte += skippedBytes;
        uint8_t byteValue = this->getUint8Value((size_t)(offset_byte + POS_OFFSET));

        
        int overflowByte = (offset_byte / 8) * 8 + POS_OFFSET;
        uint8_t overflowBit = (uint8_t)((offset_byte % 8) - 1);

        uint8_t overflowValue = this->getUint8Value((size_t)overflowByte);
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

void Pro800MidiMessage::setValue(int firstByte, int numBytes, int value)
{
    if ( !isValid() )
    {
        std::cerr << "Pro800MidiMessage::setValue() - cannot set value on invalid message!" << std::endl;
        return;
    }

    if ( numBytes > 4 )
    {
        std::cerr << "Pro800MidiMessage::setValue() only implemented for maximum of 4 byte values" << std::endl;
        return;
    }

    int skippedBytes = 0;
    for ( int i = 0; i < numBytes; i++ )
    {
        uint8_t byteValue = (value >> i*8) & 0xFF;
        uint8_t overflowBitValue = (byteValue & 0x80) >> 7;
        byteValue &= 0x7F; // limit to 127, highest byte can never be set in sysex and is covered by overflowBitValue

        int offset_byte = firstByte + i - POS_OFFSET;
        if ( offset_byte % 8 == 0 )
        {
            // this is an overflow byte - skip to next
            skippedBytes++;
        }

        offset_byte += skippedBytes;
        this->setUint8Value((size_t)(offset_byte + POS_OFFSET), byteValue);

        int overflowByte = (offset_byte / 8) * 8 + POS_OFFSET;
        uint8_t overflowBit = (uint8_t)((offset_byte % 8) - 1);

        uint8_t overflowValue = this->getUint8Value((size_t)overflowByte);

        // clear bit, then set if required
        overflowValue &= ~(1 << overflowBit);
        overflowValue |= (overflowBitValue << overflowBit);

        setUint8Value((size_t)overflowByte, overflowValue);
    }
}

std::string Pro800MidiMessage::getStringValue(int firstByte, int lastByte) const
{
    if ( !isValid() )
    {
        return std::string();
    }

    std::string value = "";
    for ( int pos = firstByte; pos <= lastByte; pos++)
    {
        if ( (pos - POS_OFFSET) % 8 == 0 )
        {
            // this is an overflow byte. Skip.
            continue; 
        }
        value += (char)getUint8Value((size_t)pos);
    }
    
    // remove potential trailing null values
    value.erase(std::remove(value.begin(), value.end(), 0x00), value.end());

    return value;
}



void Pro800MidiMessage::setStringValue(int firstByte, int lastByte, const std::string &newValue)
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
        if ( (pos - POS_OFFSET) % 8 == 0 )
        {
            // this is an overflow byte. Skip.
            numOverflowBytes++;
            pos++;
        }
        
        setUint8Value((size_t)pos, (uint8_t)resizedValue[(size_t)(pos - firstByte - numOverflowBytes)]);
    }
}
