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

#include <catch2/catch_test_macros.hpp>

#include "helpers/TestMessages.h"

#include "midi/ProgramMessage.h"
#include "tailoring/Pro800ProgramConstants.h"

using namespace TestMessages;

namespace
{
    // a distinct, non-trivial bit pattern per field that fits its width
    int testValueFor (int fieldIndex, uint8_t numBytes)
    {
        const uint32_t pattern = 0x9E3779B1u * (uint32_t) (fieldIndex + 1);
        const uint32_t mask = numBytes >= 4 ? 0x7FFFFFFFu : (1u << (numBytes * 8)) - 1u;
        return (int) (pattern & mask);
    }

    ProgramMessage validProgram (uint16_t number = 0)
    {
        const auto bytes = programDump (number);
        return ProgramMessage (bytes.data(), (int) bytes.size());
    }
}

TEST_CASE ("ProgramMessage: validity", "[midi][program]")
{
    REQUIRE (validProgram().isValid());
    REQUIRE (validProgram().getRawDataSize() == ProgramMessage::PROGRAM_MESSAGE_SIZE);

    SECTION ("the default-constructed placeholder is invalid but carries a program number")
    {
        ProgramMessage empty;
        empty.setProgramNumber (123);
        REQUIRE_FALSE (empty.isValid());
        REQUIRE (empty.getProgramNumber() == 123);
        REQUIRE (empty.getProgramName() == "--- Uninitialized ---");
    }

    SECTION ("a settings dump is not a program")
    {
        const auto bytes = settingsDump();
        REQUIRE_FALSE (ProgramMessage (bytes.data(), (int) bytes.size()).isValid());
    }
}

TEST_CASE ("ProgramMessage: program number and bank name", "[midi][program]")
{
    ProgramMessage program = validProgram (0);

    REQUIRE (program.getProgramBankNumber() == "A00");

    program.setProgramNumber (99);
    REQUIRE (program.getProgramNumber() == 99);
    REQUIRE (program.getProgramBankNumber() == "A99");

    program.setProgramNumber (100);
    REQUIRE (program.getProgramBankNumber() == "B00");

    program.setProgramNumber (399); // 0x18F: LSB 0x0F, MSB 0x03 - both 7-bit
    REQUIRE (program.getProgramNumber() == 399);
    REQUIRE (program.getProgramBankNumber() == "D99");
    REQUIRE (allDataBytesAre7Bit (program.getRawData()));
}

TEST_CASE ("ProgramMessage: every field survives a set/get round trip", "[midi][program]")
{
    ProgramMessage program = validProgram();

    int index = 0;
    for (const auto& [field, param] : PRO800_PROGRAM_FIELDS)
    {
        program.setValue (field, testValueFor (index++, param.numBytes));
    }

    // all values are still intact, i.e. no field overwrote a neighbour or an overflow bit of another field
    index = 0;
    for (const auto& [field, param] : PRO800_PROGRAM_FIELDS)
    {
        INFO ("field: " << param.name);
        REQUIRE (program.getValue (field) == testValueFor (index++, param.numBytes));
    }

    // and the result is still a legal SysEx message: high bits went into the overflow bytes
    REQUIRE (allDataBytesAre7Bit (program.getRawData()));
    REQUIRE (program.getRawData().front() == 0xF0);
    REQUIRE (program.getRawData().back() == 0xF7);
}

TEST_CASE ("ProgramMessage: values that straddle an overflow byte", "[midi][program]")
{
    ProgramMessage program = validProgram();

    // Filter Sustain sits at data bytes 31 and 33 (32 is an overflow byte)
    program.setValue (Pro800ProgramField::FILTER_SUSTAIN, 0xABCD);
    REQUIRE (program.getValue (Pro800ProgramField::FILTER_SUSTAIN) == 0xABCD);

    // Tune E is a 4-byte value at 126, 127, 129, 130 (128 is an overflow byte)
    program.setValue (Pro800ProgramField::TUNING_E, 0x7F123456);
    REQUIRE (program.getValue (Pro800ProgramField::TUNING_E) == 0x7F123456);

    // the neighbours on both sides of the overflow byte are untouched
    REQUIRE (program.getValue (Pro800ProgramField::FILTER_RESONANCE) == 0);
    REQUIRE (program.getValue (Pro800ProgramField::FILTER_DECAY) == 0);
    REQUIRE (program.getValue (Pro800ProgramField::TUNING_D_SHARP) == 0);
    REQUIRE (program.getValue (Pro800ProgramField::TUNING_F) == 0);

    REQUIRE (allDataBytesAre7Bit (program.getRawData()));
}

TEST_CASE ("ProgramMessage: program name", "[midi][program]")
{
    ProgramMessage program = validProgram();

    SECTION ("empty by default")
    {
        REQUIRE (program.getProgramName().empty());
    }

    SECTION ("round trip, including the maximum of 16 characters across two overflow bytes")
    {
        program.setProgramName ("Hello");
        REQUIRE (program.getProgramName() == "Hello");

        program.setProgramName ("ABCDEFGHIJKLMNOP");
        REQUIRE (program.getProgramName() == "ABCDEFGHIJKLMNOP");
        REQUIRE (allDataBytesAre7Bit (program.getRawData()));
    }

    SECTION ("a shorter name clears the rest of the field")
    {
        program.setProgramName ("ABCDEFGHIJKLMNOP");
        program.setProgramName ("Short");
        REQUIRE (program.getProgramName() == "Short");
    }

    SECTION ("longer names are truncated to the field")
    {
        program.setProgramName ("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        REQUIRE (program.getProgramName() == "ABCDEFGHIJKLMNOP");
    }

    SECTION ("the name does not disturb the fields around it")
    {
        program.setValue (Pro800ProgramField::ARP_MODE, PROGRAM_ARP_MODE_RANDOM);
        program.setValue (Pro800ProgramField::LFO_AFTERTOUCH_AMOUNT, 0x1234);
        program.setProgramName ("ABCDEFGHIJKLMNOP");
        REQUIRE (program.getValue (Pro800ProgramField::ARP_MODE) == PROGRAM_ARP_MODE_RANDOM);
        REQUIRE (program.getValue (Pro800ProgramField::LFO_AFTERTOUCH_AMOUNT) == 0x1234);
    }
}

TEST_CASE ("ProgramMessage: LFO destination bit field", "[midi][program]")
{
    ProgramMessage program = validProgram();

    program.setLfoDestinationEnabled (PROGRAM_LFO_DEST_FILTER, true);
    program.setLfoDestinationEnabled (PROGRAM_LFO_DEST_FREQ_B, true);

    REQUIRE (program.isLfoDestinationEnabled (PROGRAM_LFO_DEST_FILTER));
    REQUIRE (program.isLfoDestinationEnabled (PROGRAM_LFO_DEST_FREQ_B));
    REQUIRE_FALSE (program.isLfoDestinationEnabled (PROGRAM_LFO_DEST_FREQ_AB));
    REQUIRE_FALSE (program.isLfoDestinationEnabled (PROGRAM_LFO_DEST_PW_AB));

    REQUIRE (program.getLfoDestinationValue (Pro800CCMessages::LFO_MOD_DEST_FILTER) == CC_ON);
    REQUIRE (program.getLfoDestinationValue (Pro800CCMessages::LFO_MOD_DEST_FREQ_AB) == CC_OFF);
    REQUIRE (program.getLfoDestinationValue (Pro800CCMessages::LFO_TARGET) == CC_LFO_TARGET_OSC_B);

    program.setLfoDestinationEnabled (PROGRAM_LFO_DEST_FILTER, false);
    REQUIRE_FALSE (program.isLfoDestinationEnabled (PROGRAM_LFO_DEST_FILTER));
    REQUIRE (program.isLfoDestinationEnabled (PROGRAM_LFO_DEST_FREQ_B)); // clearing one bit keeps the others
}

TEST_CASE ("ProgramMessage: presets from older firmwares are upgraded to the current layout", "[midi][program]")
{
    SECTION ("a version 110 preset (without the trailing fields) is grown and re-stamped")
    {
        auto bytes = programDump (5, 110, 193); // 193 data bytes: everything up to the overflow byte at 192
        const size_t oldSize = bytes.size();
        REQUIRE (oldSize < ProgramMessage::PROGRAM_MESSAGE_SIZE);

        ProgramMessage program (bytes.data(), (int) bytes.size());

        REQUIRE (program.isValid());
        REQUIRE (program.getRawDataSize() == ProgramMessage::PROGRAM_MESSAGE_SIZE);
        REQUIRE (program.getValue (Pro800ProgramField::PRESET_VERSION) == ProgramMessage::SUPPORTED_PRESET_VERSION);
        REQUIRE (program.getRawData()[oldSize - 1] == 0x00); // the old end-of-SysEx position is data now
        REQUIRE (program.getRawData().back() == 0xF7);
        REQUIRE (program.getProgramNumber() == 5);
        REQUIRE (program.getValue (Pro800ProgramField::GLIDE_MODE) == 0); // new field, zero-initialised
    }

    SECTION ("the data of an older preset is preserved")
    {
        auto bytes = programDump (5, 110, 193);
        {
            // write a name into the raw bytes via a temporary full-size message with the same layout
            auto full = programDump (5, 110);
            ProgramMessage writer (full.data(), (int) full.size());
            writer.setProgramName ("Old Preset");
            std::copy (writer.getRawData().begin() + 11, writer.getRawData().begin() + 11 + 193, bytes.begin() + 11);
        }

        ProgramMessage program (bytes.data(), (int) bytes.size());
        REQUIRE (program.getProgramName() == "Old Preset");
    }

    SECTION ("the empty-slot placeholder is left alone")
    {
        const auto& placeholder = Pro800DataMessage::EMPTY_MESSAGE;
        ProgramMessage program (placeholder.data(), (int) placeholder.size());
        REQUIRE (program.getRawDataSize() == placeholder.size());
        REQUIRE_FALSE (program.isValid());
    }

    SECTION ("a short message that already claims the current version is not resized")
    {
        auto bytes = programDump (5, ProgramMessage::SUPPORTED_PRESET_VERSION, 20);
        ProgramMessage program (bytes.data(), (int) bytes.size());
        REQUIRE (program.getRawDataSize() == bytes.size());
        REQUIRE_FALSE (program.isValid());
    }

    SECTION ("garbage without the Pro-800 header is not resized")
    {
        std::vector<uint8_t> garbage (100, 0x11);
        garbage.front() = 0xF0;
        garbage.back() = 0xF7;
        ProgramMessage program (garbage.data(), (int) garbage.size());
        REQUIRE (program.getRawDataSize() == garbage.size());
        REQUIRE_FALSE (program.isValid());
    }
}

TEST_CASE ("ProgramMessage: toString lists every field, including the 4-byte tuning values", "[midi][program]")
{
    ProgramMessage program = validProgram (42);
    program.setProgramName ("Listed");
    program.setValue (Pro800ProgramField::TUNING_C, 0x7FFFFFFF); // used to overflow the display calculation

    const juce::String text = program.toString();

    REQUIRE (text.contains ("A42"));
    REQUIRE (text.contains ("Listed"));
    for (const auto& [field, param] : PRO800_PROGRAM_FIELDS)
    {
        INFO ("field: " << param.name);
        REQUIRE (text.contains (juce::String (param.name) + ":"));
    }
}
