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

    static const uint8_t RESPONSE_ID = 0x01;

    StatusMessage(const juce::MidiMessage &message);
    virtual MessageType getMessageType() const override { return MessageType::PRO800_STATUS_MESSAGE;}

    virtual bool isValid() const override;

    virtual juce::String toString() const override;

    Status getStatus() const;

protected:
    virtual uint8_t getResponseType() const override;

private:
    static const std::map<Status, const char *> STATUS_STRING;

    static const int POS_STATUS_BYTE = 0x01; // relative to POS_MESSAGE_START
};
