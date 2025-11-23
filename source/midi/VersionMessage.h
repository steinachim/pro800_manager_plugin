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
    virtual MessageType getMessageType() const override { return MessageType::PRO800_VERSION_MESSAGE;}

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
