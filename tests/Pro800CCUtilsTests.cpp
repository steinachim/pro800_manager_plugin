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

#include <cstdint>

#include "tailoring/Pro800CCUtils.h"
#include "tailoring/Pro800ProgramConstants.h"

TEST_CASE("Pro800CCUtils: enum values survive the trip through a 7-bit CC value", "[cc]")
{
    // every enum size used in the program field table
    for ( const int numValues : { (int) PROGRAM_ON_OFF_NUM_VALUES, (int) PROGRAM_FILTER_KEYBOARD_TRACKING_NUM_VALUES, (int) PROGRAM_MOD_WHEEL_AMOUNT_NUM_VALUES,
                                  (int) PROGRAM_LFO_SHAPE_NUM_VALUES, (int) PROGRAM_ARP_MODE_NUM_VALUES } )
    {
        for ( int enumValue = 0; enumValue < numValues; enumValue++ )
        {
            const int cc = Pro800CCUtils::ccFromProgramEnumValue(enumValue, numValues);
            INFO("numValues " << numValues << ", enum " << enumValue << " -> CC " << cc);

            REQUIRE(cc >= 0);
            REQUIRE(cc <= 127);
            REQUIRE(Pro800CCUtils::programEnumValueFromCC(cc, numValues) == enumValue);
        }
    }
}

TEST_CASE("Pro800CCUtils: continuous values pass through unchanged", "[cc]")
{
    for ( int value = 0; value <= 127; value++ )
    {
        REQUIRE(Pro800CCUtils::ccFromProgramEnumValue(value, 0) == value);
        REQUIRE(Pro800CCUtils::programEnumValueFromCC(value, 0) == value);
    }
}

TEST_CASE("Pro800CCUtils: the top of the CC range maps to the last enum value", "[cc]")
{
    // the synth may send any value within a step, 127 included
    REQUIRE(Pro800CCUtils::programEnumValueFromCC(127, PROGRAM_ON_OFF_NUM_VALUES) == PROGRAM_ON);
    REQUIRE(Pro800CCUtils::programEnumValueFromCC(127, PROGRAM_LFO_SHAPE_NUM_VALUES) == PROGRAM_LFO_SHAPE_SAW);
    REQUIRE(Pro800CCUtils::programEnumValueFromCC(127, PROGRAM_ARP_MODE_NUM_VALUES) == PROGRAM_ARP_ASSIGN);
}
