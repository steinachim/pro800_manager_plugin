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
    }

    SECTION ("the fields an older record does not have get what the synth itself fills in")
    {
        // measured: a version 109 factory preset stored through the front panel comes back as 111 with its existing
        // fields untouched and Key Tracking Ref Note = C4, Pitchbend Range = 12 x 2048, everything else 0
        auto bytes = programDump (5, 109, 181); // a 109 record ending after a 7-character name
        ProgramMessage program (bytes.data(), (int) bytes.size());

        REQUIRE (program.isValid());
        REQUIRE (program.getValue (Pro800ProgramField::LFO_AFTERTOUCH_AMOUNT) == 0);
        REQUIRE (program.getValue (Pro800ProgramField::VOICE_SPREAD_ENABLE) == PROGRAM_OFF);
        REQUIRE (program.getValue (Pro800ProgramField::KEY_TRACKING_REF_NOTE) == PROGRAM_KEYBOARD_TRACKING_REF_C4);
        REQUIRE (program.getValue (Pro800ProgramField::GLIDE_MODE) == PROGRAM_GLIDE_MODE_TIME);
        REQUIRE (program.getValue (Pro800ProgramField::PITCHBEND_RANGE) == 12 * 2048);

        // the name characters the short record did not carry stay empty, as does everything the record did carry
        REQUIRE (program.getProgramName().empty());
        REQUIRE (program.getValue (Pro800ProgramField::FILTER_CUTOFF) == 0);
    }

    SECTION ("a version 110 record keeps its own LFO Aftertouch Amount and only gets the fields after it")
    {
        auto bytes = programDump (5, 110, 193);
        {
            auto full = programDump (5, 110);
            ProgramMessage writer (full.data(), (int) full.size());
            writer.setValue (Pro800ProgramField::LFO_AFTERTOUCH_AMOUNT, 4321);
            std::copy (writer.getRawData().begin() + 11, writer.getRawData().begin() + 11 + 193, bytes.begin() + 11);
        }

        ProgramMessage program (bytes.data(), (int) bytes.size());
        REQUIRE (program.getValue (Pro800ProgramField::LFO_AFTERTOUCH_AMOUNT) == 4321);
        REQUIRE (program.getValue (Pro800ProgramField::KEY_TRACKING_REF_NOTE) == PROGRAM_KEYBOARD_TRACKING_REF_C4);
        REQUIRE (program.getValue (Pro800ProgramField::PITCHBEND_RANGE) == 12 * 2048);
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

TEST_CASE ("ProgramMessage: the pitch bend range is the field's top five bits, whoever wrote it", "[midi][program]")
{
    // every value below was read back from a saved record on firmware 1.4.6 (reverse-engineering session 19), and
    // the synth's own menu displayed the semitone count given here for the ones an operator looked at
    SECTION ("written by the 0x11 message: semitones x 2048")
    {
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (24 * 2048) == 24);
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (12 * 2048) == 12); // also what the synth fills in on upgrade
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (0) == 0);
    }

    SECTION ("written by a received CC 42: the CC byte replicated into 16 bits, cc x 516 + 3")
    {
        // clang-format off
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (1551) == 0);   // CC 3
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (2067) == 1);   // CC 4
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (3615) == 1);   // CC 7
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (24771) == 12); // CC 48
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (49539) == 24); // CC 96
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (51087) == 24); // CC 99, displayed as 24
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (63987) == 31); // CC 124
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (65535) == 31); // CC 127
        // clang-format on
    }

    SECTION ("written by the front panel's wheel: any value inside the semitone's band")
    {
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (49531) == 24);
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (50563) == 24);
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (50715) == 24);
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (50737) == 24); // the value this doc once called "the" 24
        REQUIRE (ProgramMessage::pitchBendRangeSemitones (26470) == 12);
    }

    SECTION ("a record reads its own field the same way")
    {
        auto bytes = programDump (5);
        ProgramMessage program (bytes.data(), (int) bytes.size());
        program.setValue (Pro800ProgramField::PITCHBEND_RANGE, 49531);
        REQUIRE (program.getPitchBendRangeSemitones() == 24);
    }
}

TEST_CASE ("ProgramMessage: the upgrade matches what the synth writes when it saves an older preset", "[midi][program]")
{
    // captured on firmware 1.4.6: factory preset A00 "Organ I" (version 109, 193 bytes) stored through the front panel
    // to A99, then both dumped. The synth kept every field, re-stamped the record 111, appended its own values for the
    // new fields - and blanked the name, a known firmware issue the plugin does not copy.
    // clang-format off
    const std::vector<uint8_t> a00 = {
        0xF0, 0x00, 0x20, 0x32, 0x00, 0x01, 0x24, 0x00, 0x78, 0x00, 0x00, 0x01, 0x25, 0x16, 0x61, 0x00,
        0x6D, 0x00, 0x7A, 0x16, 0x00, 0x1D, 0x4C, 0x54, 0x00, 0x7B, 0x00, 0x5F, 0x02, 0x6F, 0x26, 0x00,
        0x00, 0x66, 0x15, 0x0F, 0x00, 0x04, 0x6F, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3B, 0x00,
        0x00, 0x00, 0x00, 0x03, 0x7F, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00,
        0x2B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
        0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x4F, 0x72, 0x67, 0x61, 0x00, 0x6E, 0x20, 0x49, 0x00,
        0xF7,
    };
    const std::vector<uint8_t> a99 = {
        0xF0, 0x00, 0x20, 0x32, 0x00, 0x01, 0x24, 0x00, 0x78, 0x63, 0x00, 0x01, 0x25, 0x16, 0x61, 0x00,
        0x6F, 0x00, 0x7A, 0x16, 0x00, 0x1D, 0x4C, 0x54, 0x00, 0x7B, 0x00, 0x5F, 0x02, 0x6F, 0x26, 0x00,
        0x00, 0x66, 0x15, 0x0F, 0x00, 0x04, 0x6F, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3B, 0x00,
        0x00, 0x00, 0x00, 0x03, 0x7F, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00,
        0x2B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
        0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x60, 0xF7,
    };
    // clang-format on
    REQUIRE (a00.size() == 193);
    REQUIRE (a99.size() == ProgramMessage::PROGRAM_MESSAGE_SIZE);

    ProgramMessage upgraded (a00.data(), (int) a00.size());
    REQUIRE (upgraded.isValid());
    REQUIRE (upgraded.getProgramName() == "Organ I");

    // put the upgraded record at the synth's address and without the name, then it must be the synth's record
    ProgramMessage expected (a99.data(), (int) a99.size());
    upgraded.setProgramNumber (expected.getProgramNumber());
    upgraded.setProgramName ("");
    REQUIRE (upgraded.getRawData() == expected.getRawData());
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
