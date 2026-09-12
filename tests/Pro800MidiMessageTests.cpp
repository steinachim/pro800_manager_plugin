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

#include "midi/Pro800FactoryResetMessage.h"
#include "midi/Pro800MessageFactory.h"
#include "midi/Pro800MidiMessage.h"
#include "midi/ProgramMessage.h"
#include "midi/SettingsMessage.h"
#include "midi/StatusMessage.h"
#include "midi/VersionMessage.h"

using namespace TestMessages;

TEST_CASE ("Pro800MidiMessage: validity of the SysEx frame", "[midi]")
{
    SECTION ("a message needs at least a message type byte")
    {
        // header only: 8 bytes up to index 7, no type at index 8
        const std::vector<uint8_t> headerOnly = { 0xF0, 0x00, 0x20, 0x32, 0x00, 0x01, 0x24, 0xF7 };
        REQUIRE_FALSE (Pro800MidiMessage (headerOnly.data(), (int) headerOnly.size()).isValid());

        const auto shortest = sysEx ({ 0x42 }); // 10 bytes: type at index 8, F7 at 9
        REQUIRE (Pro800MidiMessage (shortest.data(), (int) shortest.size()).isValid());
    }

    SECTION ("both SysEx delimiters are required")
    {
        auto noStart = sysEx ({ 0x42 });
        noStart.front() = 0x00;
        REQUIRE_FALSE (Pro800MidiMessage (noStart.data(), (int) noStart.size()).isValid());

        auto noEnd = sysEx ({ 0x42 });
        noEnd.back() = 0x00;
        REQUIRE_FALSE (Pro800MidiMessage (noEnd.data(), (int) noEnd.size()).isValid());
    }

    SECTION ("the Behringer / Pro-800 header is required")
    {
        auto wrongProduct = sysEx ({ 0x42 });
        wrongProduct[6] = 0x25;
        REQUIRE_FALSE (Pro800MidiMessage (wrongProduct.data(), (int) wrongProduct.size()).isValid());
    }

    SECTION ("empty and null input do not crash and are invalid")
    {
        REQUIRE_FALSE (Pro800MidiMessage (nullptr, 0).isValid());
        REQUIRE (Pro800MidiMessage (nullptr, 0).getRawDataSize() == 0);
    }

    SECTION ("the bare F0 F7 that an empty program slot answers with is invalid")
    {
        const std::vector<uint8_t> empty = { 0xF0, 0xF7 };
        REQUIRE_FALSE (Pro800MidiMessage (empty.data(), (int) empty.size()).isValid());
    }
}

TEST_CASE ("Pro800MidiMessage: requests are built with the Pro-800 header", "[midi]")
{
    const auto request = VersionMessage::request();
    const std::vector<uint8_t> expected = sysEx ({ VersionMessage::REQUEST_ID, 0x00 });

    REQUIRE (request.isSysEx());
    REQUIRE (std::vector<uint8_t> (request.getRawData(), request.getRawData() + request.getRawDataSize()) == expected);

    REQUIRE (Pro800FactoryResetMessage::request().getRawDataSize() == (int) sysEx ({ 0x7D, 0x00 }).size());

    // program requests carry the number as 7-bit LSB / MSB
    const auto programRequest = ProgramMessage::request (300); // 300 = 0x12C -> LSB 0x2C, MSB 0x02
    const std::vector<uint8_t> expectedProgramRequest = sysEx ({ Pro800DataMessage::REQUEST_ID, 0x2C, 0x02 });
    REQUIRE (std::vector<uint8_t> (programRequest.getRawData(), programRequest.getRawData() + programRequest.getRawDataSize()) == expectedProgramRequest);
}

TEST_CASE ("Pro800MidiMessage: copies are independent", "[midi]")
{
    const auto bytes = programDump (7);
    ProgramMessage original (bytes.data(), (int) bytes.size());
    original.setProgramName ("Original");

    ProgramMessage copy (original);
    copy.setProgramName ("Copy");

    ProgramMessage assigned (bytes.data(), (int) bytes.size());
    assigned = original;
    assigned.setProgramName ("Assigned");

    REQUIRE (original.getProgramName() == "Original");
    REQUIRE (copy.getProgramName() == "Copy");
    REQUIRE (assigned.getProgramName() == "Assigned");
}

TEST_CASE ("Pro800MessageFactory: dispatches on the message type", "[midi][factory]")
{
    SECTION ("version, status, settings and program responses")
    {
        REQUIRE (Pro800MessageFactory::createMidiMessage (toMidi (sysEx ({ VersionMessage::RESPONSE_ID, 0x00, 1, 4, 6 })))->getMessageType() == MessageType::PRO800_VERSION);
        REQUIRE (Pro800MessageFactory::createMidiMessage (toMidi (sysEx ({ StatusMessage::RESPONSE_ID, 0x00, 0x00 })))->getMessageType() == MessageType::PRO800_STATUS);
        REQUIRE (Pro800MessageFactory::createMidiMessage (toMidi (settingsDump()))->getMessageType() == MessageType::PRO800_SETTINGS);
        REQUIRE (Pro800MessageFactory::createMidiMessage (toMidi (programDump (12)))->getMessageType() == MessageType::PRO800_PROGRAM);
    }

    SECTION ("unknown types come back as a generic message")
    {
        const auto message = Pro800MessageFactory::createMidiMessage (toMidi (sysEx ({ 0x42, 0x00 })));
        REQUIRE (message != nullptr);
        REQUIRE (message->getMessageType() == MessageType::PRO800_UNKNOWN);
    }

    SECTION ("a 0x78 without address bytes must not be read past its end")
    {
        // F0 <header> 78 F7: type present, address missing
        const auto message = Pro800MessageFactory::createMidiMessage (toMidi (sysEx ({ Pro800DataMessage::RESPONSE_ID })));
        REQUIRE (message != nullptr);
        REQUIRE (message->getMessageType() == MessageType::PRO800_UNKNOWN);

        // one address byte only
        const auto oneAddressByte = Pro800MessageFactory::createMidiMessage (toMidi (sysEx ({ Pro800DataMessage::RESPONSE_ID, 0x05 })));
        REQUIRE (oneAddressByte != nullptr);
        REQUIRE (oneAddressByte->getMessageType() == MessageType::PRO800_UNKNOWN);
    }

    SECTION ("non-Pro-800 input yields no message")
    {
        REQUIRE (Pro800MessageFactory::createMidiMessage (juce::MidiMessage::noteOn (1, 60, 1.0f)) == nullptr);
        REQUIRE (Pro800MessageFactory::createMidiMessage (juce::MidiMessage::controllerEvent (1, 7, 100)) == nullptr);

        const std::vector<uint8_t> emptySlotReply = { 0xF0, 0xF7 };
        REQUIRE (Pro800MessageFactory::createMidiMessage (toMidi (emptySlotReply)) == nullptr);
    }
}
