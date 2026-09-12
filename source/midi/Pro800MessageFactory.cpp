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

#include "Pro800MessageFactory.h"
#include <memory>


#include "ProgramMessage.h"
#include "SettingsMessage.h"
#include "VersionMessage.h"
#include "StatusMessage.h"

std::shared_ptr<Pro800MidiMessage> Pro800MessageFactory::createMidiMessage(const juce::MidiMessage &midiMessage)
{
    std::shared_ptr<Pro800MidiMessage> pro800Message = std::make_shared<Pro800MidiMessage>(midiMessage);
    if ( !pro800Message->isValid() )
    {
        return std::shared_ptr<Pro800MidiMessage>();
    }

    // isValid() guarantees that the message type byte exists; anything beyond it must be checked here
    const auto &rawData = pro800Message->getRawData();
    const uint8_t messageType = rawData[Pro800MidiMessage::POS_MESSAGE_TYPE];

    switch (messageType)
    {
    case SettingsMessage::RESPONSE_ID: {
        // note: SettingsMessage is a program message with a specific address
        const size_t addressLowPos = Pro800MidiMessage::POS_MESSAGE_TYPE + 1;
        const size_t addressHighPos = Pro800MidiMessage::POS_MESSAGE_TYPE + 2;
        if ( !pro800Message->isDataPosition(addressHighPos) )
        {
            // too short to carry an address: neither a settings nor a program dump
            return pro800Message;
        }

        const uint8_t addressLow = rawData[addressLowPos];
        const uint8_t addressHigh = rawData[addressHighPos];

        if ( addressLow == SettingsMessage::ADDRESS_LOW && addressHigh == SettingsMessage::ADDRESS_HIGH )
        {
            return std::make_shared<SettingsMessage>(midiMessage);
        }
        else
        {
            return std::make_shared<ProgramMessage>(midiMessage);
        }
    }

    case VersionMessage::RESPONSE_ID:
        return std::make_shared<VersionMessage>(midiMessage);

    case StatusMessage::RESPONSE_ID:
        return std::make_shared<StatusMessage>(midiMessage);

    default:
        return pro800Message;
    }
}
