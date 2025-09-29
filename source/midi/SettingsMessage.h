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
    virtual MessageType getMessageType() const override { return MessageType::PRO800_SETTINGS_MESSAGE;}


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

    uint16_t getCurrentPreset() const;
    void setCurrentPreset(uint16_t value);

    void setValue(Pro800Settings setting, int value);
    int getValue(Pro800Settings) const;

protected:
    unsigned char getResponseType() const override;
};
