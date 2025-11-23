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

#include "Pro800DataMessage.h"
#include "../Pro800Constants.h"

#include <vector>

class SettingsMessage : public Pro800DataMessage
{
public:
    static const unsigned short SETTINGS_MESSAGE_SIZE = 58;

    static const unsigned char ADDRESS_LOW = 0x7E;
    static const unsigned char ADDRESS_HIGH = 0x03;

    static juce::MidiMessage request();

    SettingsMessage(const juce::MidiMessage &message);
    virtual MessageType getMessageType() const override { return MessageType::PRO800_SETTINGS_MESSAGE;}

    virtual bool isValid() const override;
    
    virtual juce::String toString() const override;

    void setValue(Pro800Settings setting, int value);
    int getValue(Pro800Settings) const;

};
