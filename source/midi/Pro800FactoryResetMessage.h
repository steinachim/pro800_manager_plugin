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

#include "StatusMessage.h"

class Pro800FactoryResetMessage : public Pro800MidiMessage
{
public:
    static const unsigned char REQUEST_ID = 0x7D;
    static const unsigned char RESPONSE_ID = StatusMessage::RESPONSE_ID;

    static juce::MidiMessage request()
    {
        std::vector<uint8_t> request;
        request.insert(request.end(), std::begin(PRO800_HEADER), std::end(PRO800_HEADER));
        request.insert(request.end(), {REQUEST_ID, 0x00});
        return juce::MidiMessage::createSysExMessage(request.data(), (int)request.size());
    }

private:
    Pro800FactoryResetMessage(const juce::MidiMessage &message) : Pro800MidiMessage(message)
    {        
        // is request only, no dedicated response
    }
};