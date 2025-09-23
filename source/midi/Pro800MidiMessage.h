#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../Pro800Constants.h"

class Pro800MidiMessage
{
public:
    static constexpr uint8_t PRO800_HEADER[] =
    {
        0x00, 0x20, 0x32,      // Brand ID (Behringer)
        0x00, 0x01, 0x24,      // Product ID (Pro-800)
        0x00                   // CPU ID
    };

    static const int POS_MESSAGE_TYPE = 0x08;
    static const int POS_MESSAGE_START = 0x09;

    static const uint8_t RESPONSE_UNINIT = 0xFF;

    Pro800MidiMessage(const juce::MidiMessage &message);
    virtual ~Pro800MidiMessage();

    virtual Pro800MessageType getMessageType() const { return Pro800MessageType::PRO800_UNKNOWN_MESSAGE;}

    virtual juce::String toString() const;
    std::shared_ptr<juce::MidiMessage> toMidiMessage() const;

    uint8_t *getRawData() const;
    int getRawDataSize() const;

    virtual bool isValid() const;

protected:
    bool isCorrectResponse() const;
    virtual unsigned char getResponseType() const;

private:
    bool isPro800Header() const;

    uint8_t *rawData;
    int rawDataSize;
};
