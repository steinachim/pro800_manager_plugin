#pragma once

#include "Pro800MidiMessage.h"
#include "../Pro800Constants.h"

#include <vector>

class SettingsMessage : public Pro800MidiMessage
{
public:
    static const unsigned char REQUEST_ID = 0x77;
    static const unsigned char RESPONSE_ID = 0x78;

    static const unsigned char ADDRESS_LOW = 0x7E;
    static const unsigned char ADDRESS_HIGH = 0x03;

    static juce::MidiMessage request();

    SettingsMessage(const juce::MidiMessage &message);
    virtual Pro800MessageType getMessageType() const override { return Pro800MessageType::PRO800_SETTINGS_MESSAGE;}


    virtual bool isValid() const override;
    
    virtual juce::String toString() const override;

    uint16_t getExternalCVAmount() const;
    void setExternalCVAmount(uint16_t value);

    uint16_t getClockBPM() const;
    void setClockBPM(uint16_t value);

    uint8_t getVoiceStatus() const;
    void setVoiceStatus(uint8_t value);

    int8_t getTranspose() const;
    void setTranspose(int8_t value);

    uint8_t getUint8Value(Pro800Settings setting, Pro800Settings overflow = SETTINGS_NONE, Pro800OverflowBitPosition overflowBit = OVERFLOW_NONE) const;
    void setUint8Value(Pro800Settings setting, Pro800Settings overflow, Pro800OverflowBitPosition overflowBit, uint8_t value);
    void setUint8Value(Pro800Settings setting, uint8_t value);

    void setValue(Pro800Settings setting, int value);
    int getValue(Pro800Settings) const;

protected:
    uint16_t getUint16Value(Pro800Settings msb, Pro800Settings lsb, Pro800Settings overflow, std::vector<Pro800OverflowBitPosition> overflowBits) const;
    void setUint16Value(Pro800Settings msb, Pro800Settings lsb, Pro800Settings overflow, std::vector<Pro800OverflowBitPosition> overflowBits, uint16_t value);

    unsigned char getResponseType() const override;
};
