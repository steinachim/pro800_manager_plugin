#pragma once

#include "Pro800MidiMessage.h"

#include <set>

class VersionMessage : public Pro800MidiMessage
{
public:
    static const unsigned char REQUEST_ID = 0x08;
    static const unsigned char RESPONSE_ID = 0x09;

    static juce::MidiMessage request();

    VersionMessage(const juce::MidiMessage &message);
    virtual Pro800MessageType getMessageType() const override { return Pro800MessageType::PRO800_VERSION_MESSAGE;}

    virtual bool isValid() const override;

    virtual juce::String toString() const override;

    std::string getVersionString() const;
    std::string getSupportedVersions() const;
    bool isSupported() const;

protected:
    unsigned char getResponseType() const override;

private:
    enum VersionPosition
    {
        VERSION_FIELD_1 = 0x0A,
        VERSION_FIELD_2 = 0x0B,
        VERSION_FIELD_3 = 0x0C
    };

    const std::set<std::string> SUPPORTED_FIRMWARE_VERSIONS = {"1.4.6"};
};
