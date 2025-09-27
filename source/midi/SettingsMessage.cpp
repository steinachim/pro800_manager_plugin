#include "SettingsMessage.h"

#include <sstream>
#include <list>
#include <bitset>
#include <iostream>

juce::MidiMessage SettingsMessage::request()
{
    std::vector<uint8_t> request;
    request.insert(request.end(), std::begin(PRO800_HEADER), std::end(PRO800_HEADER));
    request.insert(request.end(), {REQUEST_ID, ADDRESS_LOW, ADDRESS_HIGH});
    return juce::MidiMessage::createSysExMessage(request.data(), (int)request.size());
}

SettingsMessage::SettingsMessage(const juce::MidiMessage &message) : Pro800MidiMessage(message)
{

}

bool SettingsMessage::isValid() const
{
    return Pro800MidiMessage::isValid() && (getRawDataSize() >= POS_MESSAGE_START + SETTINGS_LENGTH);
}

juce::String SettingsMessage::toString() const
{
    uint16_t extCvAmount = getExternalCVAmount();
    unsigned short extCvAmountDisplayValue = (extCvAmount * 999)/65535;

    uint16_t bpmLsb = getUint8Value(SETTINGS_SYNC_CLOCK_BPM_LSB);
    uint16_t bpmMsb = getUint8Value(SETTINGS_SYNC_CLOCK_BPM_MSB);
    uint16_t bpmOverflow = (getUint8Value(SETTINGS_OVERFLOW_BPM_VOICE8) & 0xF0) >> 4; // upper nibble

    uint16_t bpm = getClockBPM();

    std::stringstream ss;
    ss << "Pro800 Settings Message\n"
       << "Brightness: " << (unsigned int)getUint8Value(SETTINGS_BRIGHTNESS) << "\n"
       << "External CV Amount: " << extCvAmount << " (display: " << extCvAmountDisplayValue << ")\n"
       << "Clock BPM: " << bpm << std::hex << " - MSB: "<< (unsigned int)bpmMsb << ", LSB: " << (unsigned int)bpmLsb << ", overflow: " << (unsigned int)bpmOverflow << std::dec << "\n"
       << "Sync In Polarity: " << (unsigned int)getUint8Value(SETTINGS_SYNC_IN_POLARITY) << "\n"
       << "raw: " << juce::String::toHexString(getRawData(), getRawDataSize());

    return ss.str();
}

unsigned char SettingsMessage::getResponseType() const
{
    return RESPONSE_ID;
}

uint16_t SettingsMessage::getExternalCVAmount() const
{
    return getUint16Value(SETTINGS_EXTERNAL_CV_AMOUNT_MSB, SETTINGS_EXTERNAL_CV_AMOUNT_LSB, SETTINGS_EXTERNAL_CV_AMOUNT_OVERFLOW, {OVERFLOW_EXTERNAL_CV_AMOUNT_BIT8, OVERFLOW_EXTERNAL_CV_AMOUNT_BIT16});
}

void SettingsMessage::setExternalCVAmount(uint16_t value)
{
    setUint16Value(SETTINGS_EXTERNAL_CV_AMOUNT_MSB, SETTINGS_EXTERNAL_CV_AMOUNT_LSB, SETTINGS_EXTERNAL_CV_AMOUNT_OVERFLOW, {OVERFLOW_EXTERNAL_CV_AMOUNT_BIT8, OVERFLOW_EXTERNAL_CV_AMOUNT_BIT16}, value);
}

uint16_t SettingsMessage::getClockBPM() const
{
    return getUint16Value(SETTINGS_SYNC_CLOCK_BPM_MSB, SETTINGS_SYNC_CLOCK_BPM_LSB, SETTINGS_OVERFLOW_BPM_VOICE8, {OVERFLOW_BPM_BIT8});
}

void SettingsMessage::setClockBPM(uint16_t value)
{
    setUint16Value(SETTINGS_SYNC_CLOCK_BPM_MSB, SETTINGS_SYNC_CLOCK_BPM_LSB, SETTINGS_OVERFLOW_BPM_VOICE8, {OVERFLOW_BPM_BIT8}, value);
}

uint8_t SettingsMessage::getVoiceStatus() const
{
    return getUint8Value(SETTINGS_VOICE_KILL, SETTINGS_OVERFLOW_BPM_VOICE8, OVERFLOW_VOICE8_BIT8);
}

void SettingsMessage::setVoiceStatus(uint8_t value)
{
    setUint8Value(SETTINGS_VOICE_KILL, SETTINGS_OVERFLOW_BPM_VOICE8, OVERFLOW_VOICE8_BIT8, value);
}   

int8_t SettingsMessage::getTranspose() const
{
    uint8_t value = getUint8Value(SETTINGS_TRANSPOSE, SETTINGS_TRANSPOSE_OVERFLOW, OVERFLOW_TRANSPOSE_BIT8);
    return (int8_t)value;
}

void SettingsMessage::setTranspose(int8_t value)
{
    setUint8Value(SETTINGS_TRANSPOSE, SETTINGS_TRANSPOSE_OVERFLOW, OVERFLOW_TRANSPOSE_BIT8, (uint8_t)value);
}



uint8_t SettingsMessage::getUint8Value(Pro800Settings setting, Pro800Settings overflow, Pro800OverflowBitPosition overflowBit) const
{
    if ( !isValid() )
    {
        return 0;
    }

    uint8_t value = getRawData()[POS_MESSAGE_START + setting];
    if ( overflow != SETTINGS_NONE && overflowBit != OVERFLOW_NONE )
    {
        uint8_t overflowValue = getRawData()[POS_MESSAGE_START + overflow];
        uint8_t overflowBitValue = (overflowValue & (1 << overflowBit)) ? 1 : 0;
        value |= (overflowBitValue << 7);
    }

    return value;
}

void SettingsMessage::setUint8Value(Pro800Settings setting, uint8_t value)
{
    setUint8Value(setting, SETTINGS_NONE, OVERFLOW_NONE, value);
}

void SettingsMessage::setUint8Value(Pro800Settings setting, Pro800Settings overflow, Pro800OverflowBitPosition overflowBit, uint8_t value)
{
    // TODO: implement builder pattern for setting
    if ( !isValid() )
    {
        return;
    }

    if ( overflow != SETTINGS_NONE && overflowBit != OVERFLOW_NONE )
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

uint16_t SettingsMessage::getUint16Value(Pro800Settings msb, Pro800Settings lsb, Pro800Settings overflow, std::vector<Pro800OverflowBitPosition> overflowBits) const
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

    uint8_t lsbValue = getUint8Value(lsb);
    uint8_t msbValue = getUint8Value(msb);
    uint8_t overflowValue = getUint8Value(overflow);

    
    uint8_t overflowValue1 = (overflowValue & (1 << overflowBits.at(0))) ? 1 : 0;

    uint16_t resultValue = lsbValue;
    resultValue |= (overflowValue1 << 7);
    resultValue |= ((uint16_t)msbValue << 8);

    if ( overflowBits.size() > 1)
    {
        uint8_t overflowValue2 = (overflowValue & (1 << overflowBits.at(1))) ? 1 : 0;
        
        resultValue |= ((uint16_t)overflowValue2 << 15);
    }

    return resultValue;
   
}

void SettingsMessage::setUint16Value(Pro800Settings msb, Pro800Settings lsb, Pro800Settings overflow, std::vector<Pro800OverflowBitPosition> overflowBits, uint16_t value)
{
    if ( !isValid() )
    {
        return;
    }

    uint8_t lsbValue = value & 0x007f;
    uint8_t msbValue = ((value & 0x7f00) >> 8);

    uint8_t overflowValue = getUint8Value(overflow);
    for ( uint8_t i = 0; i < (uint8_t)overflowBits.size(); i++)
    {
        Pro800OverflowBitPosition overflowBitPosition = overflowBits.at(i);
        uint8_t valueBitPosition = 8*(i+1)-1; // (e.g. 0x80 -> 7; or 0x8000 -> 15)
        uint8_t bitValue = (value & (1 << valueBitPosition)) ? 1 : 0;
        
        // clear bit, then set if required
        overflowValue &= ~(1 << overflowBitPosition);
        overflowValue |= (bitValue << overflowBitPosition);
    }

    setUint8Value(lsb, lsbValue);
    setUint8Value(msb, msbValue);
    setUint8Value(overflow, overflowValue);
}

void SettingsMessage::setValue(Pro800Settings setting, int value)
{
    switch(setting)
    {
        case SETTINGS_TRANSPOSE:
            setTranspose((int8_t)value);
            break;

        case SETTINGS_EXTERNAL_CV_AMOUNT:
            setExternalCVAmount((uint16_t)value);
            break;

        case SETTINGS_SYNC_CLOCK_BPM:
            setClockBPM((uint16_t)value);
            break;

        case SETTINGS_VOICE_KILL:
            setVoiceStatus((uint8_t)value);
            break;

        default:
            setUint8Value(setting, (uint8_t)value);
            break;
    }
}

int SettingsMessage::getValue(Pro800Settings setting) const
{
    switch(setting)
    {
        case SETTINGS_TRANSPOSE:
            return getTranspose();

        case SETTINGS_EXTERNAL_CV_AMOUNT:
            return getExternalCVAmount();

        case SETTINGS_SYNC_CLOCK_BPM:
            return getClockBPM();

        case SETTINGS_VOICE_KILL:
            return getVoiceStatus();

        default:
            return (int)getUint8Value(setting);
    }
}
