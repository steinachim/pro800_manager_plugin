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

#include <memory>
#include <string>
#include <vector>

#include "../tailoring/Pro800CCConstants.h"
#include "../tailoring/Pro800ProgramConstants.h"
#include "Pro800DataMessage.h"

class ProgramMessage : public Pro800DataMessage
{
public:
    static constexpr uint16_t NUM_PROGRAMS = 400;

    static constexpr uint8_t SUPPORTED_PRESET_VERSION = 111;
    static constexpr size_t PROGRAM_MESSAGE_SIZE = 210; // complete size of a SUPPORTED_PRESET_VERSION program dump

    static juce::MidiMessage request (int programNumber);

    ProgramMessage();
    explicit ProgramMessage (const juce::MidiMessage& message);
    ProgramMessage (const uint8_t* newRawData, int newRawDataSize);
    virtual MessageType getMessageType() const override { return MessageType::PRO800_PROGRAM; }

    virtual bool isValid() const override;

    uint16_t getProgramNumber() const;
    std::string getProgramBankNumber() const;
    void setProgramNumber (uint16_t programNumber);

    std::string getProgramName() const;
    void setProgramName (const std::string& newName);

    virtual juce::String toString() const override;

    bool isLfoDestinationEnabled (Pro800ProgramLfoDestinationBitMask destination) const;
    void setLfoDestinationEnabled (Pro800ProgramLfoDestinationBitMask destination, bool enabled);

    int getLfoDestinationValue (Pro800CCMessages ccNumber) const;

    int getValue (Pro800ProgramField field) const;
    void setValue (Pro800ProgramField field, int value);

private:
    void upgradeOlderPresetVersion();
};
