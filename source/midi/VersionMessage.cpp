/** 
 * Pro800 Manager Plugin
 * Copyright (C) 2025 Achim Stein
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

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
        ss << (int)getUint8Value(VERSION_FIELD_1) << "." 
           << (int)getUint8Value(VERSION_FIELD_2) << "." 
           << (int)getUint8Value(VERSION_FIELD_3);
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
