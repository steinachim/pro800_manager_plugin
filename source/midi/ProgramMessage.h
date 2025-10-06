#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Pro800MidiMessage.h"

class ProgramMessage : public Pro800MidiMessage
{
public:
    
    static const uint16_t NUM_PROGRAMS = 400;
    static const uint8_t REQUEST_ID = 0x77;
    static const uint8_t RESPONSE_ID = 0x78;

    const static inline std::vector<uint8_t> EMPTY_PROGRAM = {
        0xF0,
        0x00, 0x20, 0x32,      // Brand ID (Behringer)
        0x00, 0x01, 0x24,      // Product ID (Pro-800)
        0x00,                  // CPU ID,
        RESPONSE_ID,           // command
        0x7F, 0x7F,            // program number (invalid)
        0xF7
    };

    static const uint8_t SUPPORTED_PRESET_VERSION = 111;
    static const int PROGRAM_MESSAGE_SIZE = 210;

    static juce::MidiMessage request(int programNumber);

    ProgramMessage();
    ProgramMessage(const juce::MidiMessage &message);
    ProgramMessage(const uint8_t *newRawData, int newRawDataSize);
    ProgramMessage(const ProgramMessage &other);
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

private:
    bool isInitialized = false;
};
