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

#include <string>

#include "../tailoring/Pro800CCConstants.h"
#include "../tailoring/Pro800ProgramConstants.h"
#include "Pro800DataMessage.h"

class ProgramMessage : public Pro800DataMessage
{
public:
    static constexpr uint16_t NUM_PROGRAMS = 400;

    static constexpr uint8_t SUPPORTED_PRESET_VERSION = 111;
    static constexpr size_t PROGRAM_MESSAGE_SIZE = 210; // complete size of a SUPPORTED_PRESET_VERSION program dump

    /** Requests the dump of one program (0 .. NUM_PROGRAMS - 1). */
    static juce::MidiMessage request (int programNumber);

    ProgramMessage();
    explicit ProgramMessage (const juce::MidiMessage& message);
    ProgramMessage (const uint8_t* newRawData, int newRawDataSize);
    virtual MessageType getMessageType() const override { return MessageType::PRO800_PROGRAM; }

    virtual bool isValid() const override;

    uint16_t getProgramNumber() const;
    /** The program number as shown on the synth: bank letter A-D and slot 00-99, e.g. "B07". */
    std::string getProgramBankNumber() const;
    void setProgramNumber (uint16_t programNumber);

    std::string getProgramName() const;
    void setProgramName (const std::string& newName);

    virtual juce::String toString() const override;

    bool isLfoDestinationEnabled (Pro800ProgramLfoDestinationBitMask destination) const;
    void setLfoDestinationEnabled (Pro800ProgramLfoDestinationBitMask destination, bool enabled);

    int getLfoDestinationValue (Pro800CCMessages ccNumber) const;

    /** The CC value the LFO destination bitmask means for the given CC (the three on/off destinations, or LFO_TARGET). */
    static int lfoDestinationValue (uint8_t lfoDestinations, Pro800CCMessages ccNumber);

    int getValue (Pro800ProgramField field) const;
    void setValue (Pro800ProgramField field, int value);

    /**
     * The semitone count (0-31) a Pitchbend Range field value means: its top five bits. The low eleven bits depend on
     * who wrote the field - the 0x11 message leaves them zero (semitones x 2048), a received CC 42 stores the CC byte
     * replicated into 16 bits (cc x 516 + 3), the front panel's wheel any value inside the semitone's band - and the
     * synth reads all of them this way (reverse-engineering doc, section 13; measured in its session 19).
     */
    static int pitchBendRangeSemitones (int fieldValue);
    int getPitchBendRangeSemitones() const;

private:
    void upgradeOlderPresetVersion();
};
