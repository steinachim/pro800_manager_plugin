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
#include "midi/Pro800Hazards.h"
#include "midi/ReloadMessage.h"

using namespace TestMessages;

//==============================================================================
TEST_CASE ("Pro800Hazards: the messages that must never be sent are refused with a reason", "[midi][hazard]")
{
    SECTION ("factory reset, with or without a parameter")
    {
        REQUIRE (Pro800Hazards::hazardReason (sysEx ({ 0x7D })).has_value());
        REQUIRE (Pro800Hazards::hazardReason (bytesOf (Pro800FactoryResetMessage::request()))->contains ("factory reset"));
        REQUIRE (Pro800Hazards::isIrreversible (sysEx ({ 0x7D, 0x00 })));
    }

    SECTION ("0x03 only with parameter 0x30, which enters the bootloader")
    {
        REQUIRE (Pro800Hazards::hazardReason (sysEx ({ 0x03, 0x30 }))->contains ("bootloader"));
        REQUIRE (Pro800Hazards::isIrreversible (sysEx ({ 0x03, 0x30 })));

        REQUIRE_FALSE (Pro800Hazards::hazardReason (sysEx ({ 0x03 })).has_value());
        REQUIRE_FALSE (Pro800Hazards::hazardReason (sysEx ({ 0x03, 0x00 })).has_value());
        REQUIRE_FALSE (Pro800Hazards::hazardReason (sysEx ({ 0x03, 0x2F })).has_value());
        REQUIRE_FALSE (Pro800Hazards::hazardReason (sysEx ({ 0x03, 0x31 })).has_value());
    }

    SECTION ("0x32 only with a non-zero parameter; 0x32 00 is the reload")
    {
        REQUIRE (Pro800Hazards::hazardReason (sysEx ({ 0x32, 0x01 }))->contains ("8888"));
        REQUIRE (Pro800Hazards::hazardReason (sysEx ({ 0x32, 0x7F })).has_value());
        REQUIRE_FALSE (Pro800Hazards::isIrreversible (sysEx ({ 0x32, 0x01 })));

        REQUIRE_FALSE (Pro800Hazards::hazardReason (bytesOf (ReloadMessage::request())).has_value());
    }

    SECTION ("0x0E and 0x50 whatever the parameters")
    {
        REQUIRE (Pro800Hazards::hazardReason (sysEx ({ 0x0E }))->contains ("deaf"));
        REQUIRE (Pro800Hazards::hazardReason (sysEx ({ 0x0E, 0x00 })).has_value());
        REQUIRE (Pro800Hazards::hazardReason (sysEx ({ 0x50, 0x05, 0x00, 0x41 }))->contains ("name"));
        REQUIRE (Pro800Hazards::hazardReason (sysEx ({ 0x50 })).has_value());
    }
}

TEST_CASE ("Pro800Hazards: ordinary requests and non-Pro-800 messages are not hazards", "[midi][hazard]")
{
    REQUIRE_FALSE (Pro800Hazards::hazardReason (sysEx ({ 0x06 })).has_value()); // device name
    REQUIRE_FALSE (Pro800Hazards::hazardReason (sysEx ({ 0x08, 0x00 })).has_value()); // version
    REQUIRE_FALSE (Pro800Hazards::hazardReason (sysEx ({ 0x70, 0x28 })).has_value()); // panel read
    REQUIRE_FALSE (Pro800Hazards::hazardReason (sysEx ({ 0x72, 0x19 })).has_value()); // live read
    REQUIRE_FALSE (Pro800Hazards::hazardReason (sysEx ({ 0x77, 0x7E, 0x03 })).has_value()); // settings read
    REQUIRE_FALSE (Pro800Hazards::hazardReason (programDump (5)).has_value()); // program write

    REQUIRE_FALSE (Pro800Hazards::hazardReason ({ 0xF0, 0xF7 }).has_value());
    REQUIRE_FALSE (Pro800Hazards::hazardReason ({ 0xB0, 0x07, 0x64 }).has_value());
    REQUIRE_FALSE (Pro800Hazards::hazardReason ({}).has_value());

    // the right type byte behind a foreign header is somebody else's message
    REQUIRE_FALSE (Pro800Hazards::hazardReason ({ 0xF0, 0x00, 0x20, 0x33, 0x00, 0x01, 0x24, 0x00, 0x7D, 0xF7 }).has_value());
}
