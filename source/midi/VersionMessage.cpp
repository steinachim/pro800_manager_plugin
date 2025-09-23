#include "VersionMessage.h"

#include <sstream>

juce::MidiMessage VersionMessage::request()
{
    std::vector<uint8_t> request;
    request.insert(request.end(), std::begin(PRO800_HEADER), std::end(PRO800_HEADER));
    request.insert(request.end(), {REQUEST_ID, 0x00});
    return juce::MidiMessage::createSysExMessage(request.data(), (int)request.size());
}

VersionMessage::VersionMessage(const juce::MidiMessage &message) : Pro800MidiMessage(message)
{

}

bool VersionMessage::isValid() const
{
    return Pro800MidiMessage::isValid() && (getRawDataSize() >= VERSION_FIELD_3);
}

juce::String VersionMessage::toString() const
{
    std::stringstream ss;
    ss << "Pro-800 Firmware Version: " + getVersionString();
    if (isSupported())
    {
        ss << " (supported)";
    }
    else
    {
        ss << " (unsupported!)";
    }

    return ss.str();
}

std::string VersionMessage::getVersionString() const
{
    if (!isValid())
    {
        return "UNKNOWN VERSION";
    }
    else
    {
        std::stringstream ss;
        ss << (int)getRawData()[VERSION_FIELD_1] << "." << (int)getRawData()[VERSION_FIELD_2] << "." << (int)getRawData()[VERSION_FIELD_3];
        return ss.str();
    }
}

std::string VersionMessage::getSupportedVersions() const
{
    std::stringstream ss;
    for ( const auto &version : SUPPORTED_FIRMWARE_VERSIONS )
    {
        ss << version << ", ";
    }
    std::string versionString = ss.str();
    versionString.erase(versionString.end()-2, versionString.end());
    return versionString;
}

bool VersionMessage::isSupported() const
{
    return SUPPORTED_FIRMWARE_VERSIONS.find(getVersionString()) != SUPPORTED_FIRMWARE_VERSIONS.end();
}

unsigned char VersionMessage::getResponseType() const
{
    return RESPONSE_ID;
}
