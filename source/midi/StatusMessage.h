#pragma once

#include "Pro800MidiMessage.h"

#include <map>

class StatusMessage : public Pro800MidiMessage
{
public:
    enum Status
    {
        STATUS_OK = 0x00,
        STATUS_ERROR = 0x01,
        STATUS_INVALID = 0xFF
    };

    static const unsigned char RESPONSE_ID = 0x01;

    StatusMessage(const juce::MidiMessage &message);
    virtual Pro800MessageType getMessageType() const override { return Pro800MessageType::PRO800_STATUS_MESSAGE;}

    virtual bool isValid() const override;

    virtual juce::String toString() const override;

    Status getStatus() const;

protected:
    virtual unsigned char getResponseType() const override;

private:
    static const std::map<Status, const char *> STATUS_STRING;

    static const size_t POS_STATUS_BYTE = 0x0A;
};
