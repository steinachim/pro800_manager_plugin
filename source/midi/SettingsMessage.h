#pragma once

#include "Pro800MidiMessage.h"
#include "../Pro800Constants.h"

#include <vector>

class SettingsMessage : public Pro800MidiMessage
{
    static const unsigned short SETTINGS_MESSAGE_SIZE = 58;

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

    void setValue(Pro800Settings setting, int value);
    int getValue(Pro800Settings) const;

protected:
    unsigned char getResponseType() const override;
};
