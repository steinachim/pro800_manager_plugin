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

#include <cstdint>

class Pro800CCUtils
{
public:
    static int ccFromProgramEnumValue(int enumValue, int numValues)
    {
        // warning: this mapping does *not* work for Pro800ProgramLfoDestinationBitMask because it's a bitmask
        if (numValues <= 0)
        {
            return enumValue; // continuous value
        }

        int stepSize = 127 / numValues + 1;
        return (uint8_t)(enumValue * stepSize);
    }

    static int programEnumValueFromCC(int ccValue, int numValues)
    {
        // warning: this mapping does *not* work for Pro800ProgramLfoDestinationBitMask because it's a bitmask
        if (numValues <= 0)
        {
            return ccValue; // continuous value
        }

        int stepSize = 127 / numValues + 1;
        return ccValue / stepSize;
    }
};