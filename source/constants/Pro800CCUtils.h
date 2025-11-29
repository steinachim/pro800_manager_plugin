#pragma once

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