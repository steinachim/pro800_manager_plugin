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

#include "ProgramMessage.h"

juce::MidiMessage ProgramMessage::request(int programNumber)
{
    uint8_t programLSB = (programNumber & 0x7F);
    uint8_t programMSB = (uint8_t)(programNumber >> 7);

    return Pro800DataMessage::request(programLSB, programMSB);
}

ProgramMessage::ProgramMessage() : Pro800DataMessage()
{
}

ProgramMessage::ProgramMessage(const juce::MidiMessage &message) : ProgramMessage(message.getRawData(), message.getRawDataSize())
{
}

ProgramMessage::ProgramMessage(const uint8_t *newRawData, int newRawDataSize) : Pro800DataMessage(newRawData, newRawDataSize)
{
    upgradeOlderPresetVersion();
}

void ProgramMessage::upgradeOlderPresetVersion()
{
    // Presets stored by older firmwares (preset version < SUPPORTED_PRESET_VERSION) are shorter,
    // because the trailing fields did not exist yet. Grow them to the current layout (new fields = 0)
    // and stamp the current version so that all getters/setters work on them.
    //
    // Anything that is not a well-formed program dump carrying a version byte is left untouched;
    // in particular the 12-byte "empty slot" placeholder must stay invalid.
    const size_t oldSize = getRawDataSize();
    if ( oldSize >= PROGRAM_MESSAGE_SIZE || !Pro800MidiMessage::isValid() )
    {
        return;
    }

    const size_t versionPos = DATA_START_POS + PRO800_PROGRAM_FIELDS.at(Pro800ProgramField::PRESET_VERSION).firstByte;
    if ( oldSize <= versionPos + 1 ) // version byte plus the trailing 0xF7
    {
        return;
    }

    // raw read: the message is not a valid current-layout program yet, so getValue() would refuse.
    // Preset versions are < 128, so the overflow bit is irrelevant here.
    const uint8_t version = getUint8Value(versionPos);
    if ( version == 0 || version >= SUPPORTED_PRESET_VERSION )
    {
        return;
    }

    resizeRawData(PROGRAM_MESSAGE_SIZE); // zero-filled

    // move 0xF7 from previous last position to new last position
    setUint8Value(oldSize - 1, 0x00);
    setUint8Value(PROGRAM_MESSAGE_SIZE - 1, 0xF7);

    setValue(Pro800ProgramField::PRESET_VERSION, SUPPORTED_PRESET_VERSION);
}

bool ProgramMessage::isValid() const
{
    // a program dump must have the complete current layout (older layouts are upgraded in the constructor)
    return Pro800MidiMessage::isValid() && getRawDataSize() >= PROGRAM_MESSAGE_SIZE;
}

uint16_t ProgramMessage::getProgramNumber() const
{
    uint8_t programLSB = (uint8_t)getUint8Value(ADDRESS_LSB_POS);
    uint8_t programMSB = (uint8_t)getUint8Value(ADDRESS_MSB_POS);
    return (uint16_t)((programMSB << 7) | programLSB);
}

std::string ProgramMessage::getProgramBankNumber() const
{
    const uint16_t programNumber = getProgramNumber();
    const int bank = programNumber / 100;    // 0-3 = A-D
    const int program = programNumber % 100; // 0-99

    return juce::String::formatted("%c%02d", 'A' + bank, program).toStdString();
}

void ProgramMessage::setProgramNumber(uint16_t programNumber)
{
    uint8_t programLSB = programNumber & 0x7F;
    uint8_t programMSB = (programNumber >> 7) & 0x7F;
    setUint8Value(ADDRESS_LSB_POS, programLSB);
    setUint8Value(ADDRESS_MSB_POS, programMSB);
}

std::string ProgramMessage::getProgramName() const
{
    if ( !isValid() )
    {
        return "--- Uninitialized ---";
    }

    size_t firstByte = PRO800_PROGRAM_FIELDS.at(Pro800ProgramField::NAME_FIRST_CHAR).firstByte;
    size_t lastByte = PRO800_PROGRAM_FIELDS.at(Pro800ProgramField::NAME_LAST_CHAR).firstByte;
    return getStringValue(firstByte, lastByte);
}

void ProgramMessage::setProgramName(const std::string &newName)
{
    size_t firstByte = PRO800_PROGRAM_FIELDS.at(Pro800ProgramField::NAME_FIRST_CHAR).firstByte;
    size_t lastByte = PRO800_PROGRAM_FIELDS.at(Pro800ProgramField::NAME_LAST_CHAR).firstByte;
    setStringValue(firstByte, lastByte, newName);
}

bool ProgramMessage::isLfoDestinationEnabled(Pro800ProgramLfoDestinationBitMask destination) const
{
    const uint8_t lfoDestinations = (uint8_t)getValue(Pro800ProgramField::LFO_DEST);
    return lfoDestinations & destination;
}

void ProgramMessage::setLfoDestinationEnabled(Pro800ProgramLfoDestinationBitMask destination, bool enabled)
{
    uint8_t lfoDestinations = (uint8_t)getValue(Pro800ProgramField::LFO_DEST);
    uint8_t targetValue = (enabled ? destination : 0);

    lfoDestinations = (lfoDestinations & ~destination) | targetValue;
    setValue (Pro800ProgramField::LFO_DEST, lfoDestinations);
}

int ProgramMessage::getLfoDestinationValue (Pro800CCMessages ccNumber) const
{
    if ( ccNumber == Pro800CCMessages::LFO_MOD_DEST_FREQ_AB )
    {
        return isLfoDestinationEnabled (PROGRAM_LFO_DEST_FREQ_AB) ? CC_ON : CC_OFF;
    }

    if ( ccNumber == Pro800CCMessages::LFO_MOD_DEST_PW_AB )
    {
        return isLfoDestinationEnabled (PROGRAM_LFO_DEST_PW_AB) ? CC_ON : CC_OFF;
    }

    if ( ccNumber == Pro800CCMessages::LFO_MOD_DEST_FILTER )
    {
        return isLfoDestinationEnabled (PROGRAM_LFO_DEST_FILTER) ? CC_ON : CC_OFF;
    }

    if ( ccNumber == Pro800CCMessages::LFO_TARGET )
    {
        if (isLfoDestinationEnabled (PROGRAM_LFO_DEST_FREQ_A))
        {
            return CC_LFO_TARGET_OSC_A;
        }
        if (isLfoDestinationEnabled (PROGRAM_LFO_DEST_FREQ_B))
        {
            return CC_LFO_TARGET_OSC_B;
        }
        if (isLfoDestinationEnabled (PROGRAM_LFO_DEST_FREQ_AB_VCA))
        {
            return CC_LFO_TARGET_VCA;
        }
        return CC_LFO_TARGET_OSC_AB;
    }

    juce::Logger::writeToLog("ProgramMessage::getLfoDestinationValue(): Unsupported CC number: " + juce::String(static_cast<int>(ccNumber)));
    return 0;
}

juce::String ProgramMessage::toString() const
{
    juce::String header = "Pro800 Program Dump: ";
    header << getProgramBankNumber() << " - '" << getProgramName() << "'\n";
    return header + fieldsToString(PRO800_PROGRAM_FIELDS);
}

int ProgramMessage::getValue(Pro800ProgramField field) const
{
    return getFieldValue(PRO800_PROGRAM_FIELDS, field);
}

void ProgramMessage::setValue(Pro800ProgramField field, int value)
{
    setFieldValue(PRO800_PROGRAM_FIELDS, field, value);
}
