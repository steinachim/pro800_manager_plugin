#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Pro800MidiMessage.h"

class ProgramMessage : public Pro800MidiMessage
{
public:
    static const unsigned short NUM_PROGRAMS = 400;
    static const unsigned char REQUEST_ID = 0x77;
    static const unsigned char RESPONSE_ID = 0x78;

    static const unsigned short MESSAGE_SIZE_VERSION_111 = 210;

    static juce::MidiMessage request(int programNumber);

    ProgramMessage(const juce::MidiMessage &message);
    ProgramMessage(const uint8_t *newRawData, int newRawDataSize);
    virtual MessageType getMessageType() const override { return MessageType::PRO800_PROGRAM_MESSAGE;}

    virtual bool isValid() const override;

    uint16_t getProgramNumber() const;
    std::string getProgramBankNumber() const;
    void setProgramNumber(uint16_t programNumber);

    std::string getProgramName() const;
    void setProgramName(const std::string &newName);

    virtual juce::String toString() const override;

    bool isLfoDestinationEnabled(Pro800ProgramLfoDestination destination) const;
    void setLfoDestinationEnabled(Pro800ProgramLfoDestination destination, bool enabled);

    int getValue(Pro800ProgramField field) const;
    void setValue(Pro800ProgramField field, int value);

protected:
    unsigned char getResponseType() const override;
};
