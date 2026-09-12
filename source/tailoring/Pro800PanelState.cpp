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

#include "Pro800PanelState.h"

#include "Pro800CCUtils.h"

namespace
{
    // clang-format off
    // the panel switches that are one program field each (the Filter Keyboard pair and the LFO destinations are combined below)
    const std::map<Pro800PanelIndex, Pro800ProgramField> SWITCH_TO_FIELD =
    {
        {Pro800PanelIndex::SWITCH_OSC_A_SHAPE_RECT,      Pro800ProgramField::OSC_A_SHAPE_RECT},
        {Pro800PanelIndex::SWITCH_OSC_B_SHAPE_RECT,      Pro800ProgramField::OSC_B_SHAPE_RECT},
        {Pro800PanelIndex::SWITCH_OSC_A_SHAPE_SAW,       Pro800ProgramField::OSC_A_SHAPE_SAW},
        {Pro800PanelIndex::SWITCH_OSC_A_SHAPE_TRI,       Pro800ProgramField::OSC_A_SHAPE_TRI},
        {Pro800PanelIndex::SWITCH_OSC_A_SYNC,            Pro800ProgramField::OSC_A_SYNC},
        {Pro800PanelIndex::SWITCH_OSC_B_SHAPE_SAW,       Pro800ProgramField::OSC_B_SHAPE_SAW},
        {Pro800PanelIndex::SWITCH_OSC_B_SHAPE_TRI,       Pro800ProgramField::OSC_B_SHAPE_TRI},
        {Pro800PanelIndex::SWITCH_POLY_MOD_DEST_FREQ_A,  Pro800ProgramField::POLYMOD_DEST_FREQ_A},
        {Pro800PanelIndex::SWITCH_POLY_MOD_DEST_FILTER,  Pro800ProgramField::POLYMOD_DEST_FILTER},
        {Pro800PanelIndex::SWITCH_POLY_MOD_UNISON_TRACK, Pro800ProgramField::POLYMOD_UNISON_TRACK},
    };

    const std::map<Pro800PanelIndex, Pro800ProgramLfoDestinationBitMask> SWITCH_TO_LFO_DEST_BIT =
    {
        {Pro800PanelIndex::SWITCH_LFO_DEST_FREQ_AB, PROGRAM_LFO_DEST_FREQ_AB},
        {Pro800PanelIndex::SWITCH_LFO_DEST_PW_AB,   PROGRAM_LFO_DEST_PW_AB},
        {Pro800PanelIndex::SWITCH_LFO_DEST_FILTER,  PROGRAM_LFO_DEST_FILTER},
    };

    const std::map<Pro800LiveIndex, Pro800ProgramField> LIVE_TO_FIELD =
    {
        {Pro800LiveIndex::AMP_RELEASE,                Pro800ProgramField::AMP_RELEASE},
        {Pro800LiveIndex::AMP_SUSTAIN,                Pro800ProgramField::AMP_SUSTAIN},
        {Pro800LiveIndex::AMP_DECAY,                  Pro800ProgramField::AMP_DECAY},
        {Pro800LiveIndex::AMP_ATTACK,                 Pro800ProgramField::AMP_ATTACK},
        {Pro800LiveIndex::FILTER_RELEASE,             Pro800ProgramField::FILTER_RELEASE},
        {Pro800LiveIndex::FILTER_SUSTAIN,             Pro800ProgramField::FILTER_SUSTAIN},
        {Pro800LiveIndex::FILTER_DECAY,               Pro800ProgramField::FILTER_DECAY},
        {Pro800LiveIndex::FILTER_ATTACK,              Pro800ProgramField::FILTER_ATTACK},
        {Pro800LiveIndex::FILTER_RESONANCE,           Pro800ProgramField::FILTER_RESONANCE},
        {Pro800LiveIndex::FILTER_ENV_AMOUNT,          Pro800ProgramField::FILTER_ENV_AMOUNT},
        {Pro800LiveIndex::OSC_A_FREQ,                 Pro800ProgramField::OSC_A_FREQ},
        {Pro800LiveIndex::OSC_B_FREQ,                 Pro800ProgramField::OSC_B_FREQ},
        {Pro800LiveIndex::OSC_B_FINE,                 Pro800ProgramField::OSC_B_FINE_FREQ},
        {Pro800LiveIndex::OSC_A_PULSE_WIDTH,          Pro800ProgramField::OSC_A_PULSE_WIDTH},
        {Pro800LiveIndex::OSC_B_PULSE_WIDTH,          Pro800ProgramField::OSC_B_PULSE_WIDTH},
        {Pro800LiveIndex::OSC_A_LEVEL,                Pro800ProgramField::OSC_A_LEVEL},
        {Pro800LiveIndex::GLIDE_TIME,                 Pro800ProgramField::GLIDE_AMOUNT},
        {Pro800LiveIndex::LFO_FREQ,                   Pro800ProgramField::LFO_FREQ},
        {Pro800LiveIndex::LFO_INITIAL_AMOUNT,         Pro800ProgramField::LFO_AMOUNT},
        {Pro800LiveIndex::OSC_B_LEVEL,                Pro800ProgramField::OSC_B_LEVEL},
        {Pro800LiveIndex::POLY_MOD_SOURCE_OSC_B,      Pro800ProgramField::POLYMOD_SOURCE_OSC_B},
        {Pro800LiveIndex::POLY_MOD_SOURCE_FILTER_ENV, Pro800ProgramField::POLYMOD_SOURCE_FILTER_ENV},
        {Pro800LiveIndex::NOISE_LEVEL,                Pro800ProgramField::NOISE},
        {Pro800LiveIndex::FILTER_CUTOFF,              Pro800ProgramField::FILTER_CUTOFF},
    };

    // every knob also has a CC, which is what the synth itself sends when it is turned
    const std::map<Pro800LiveIndex, Pro800CCMessages> LIVE_TO_CC =
    {
        {Pro800LiveIndex::AMP_RELEASE,                Pro800CCMessages::AMP_RELEASE},
        {Pro800LiveIndex::AMP_SUSTAIN,                Pro800CCMessages::AMP_SUSTAIN},
        {Pro800LiveIndex::AMP_DECAY,                  Pro800CCMessages::AMP_DECAY},
        {Pro800LiveIndex::AMP_ATTACK,                 Pro800CCMessages::AMP_ATTACK},
        {Pro800LiveIndex::FILTER_RELEASE,             Pro800CCMessages::FILTER_RELEASE},
        {Pro800LiveIndex::FILTER_SUSTAIN,             Pro800CCMessages::FILTER_SUSTAIN},
        {Pro800LiveIndex::FILTER_DECAY,               Pro800CCMessages::FILTER_DECAY},
        {Pro800LiveIndex::FILTER_ATTACK,              Pro800CCMessages::FILTER_ATTACK},
        {Pro800LiveIndex::FILTER_RESONANCE,           Pro800CCMessages::FILTER_RESONANCE},
        {Pro800LiveIndex::FILTER_ENV_AMOUNT,          Pro800CCMessages::FILTER_ENV_AMOUNT},
        {Pro800LiveIndex::OSC_A_FREQ,                 Pro800CCMessages::OSC_A_FREQ},
        {Pro800LiveIndex::OSC_B_FREQ,                 Pro800CCMessages::OSC_B_FREQ},
        {Pro800LiveIndex::OSC_B_FINE,                 Pro800CCMessages::OSC_B_FINE},
        {Pro800LiveIndex::OSC_A_PULSE_WIDTH,          Pro800CCMessages::OSC_A_PULSE_WIDTH},
        {Pro800LiveIndex::OSC_B_PULSE_WIDTH,          Pro800CCMessages::OSC_B_PULSE_WIDTH},
        {Pro800LiveIndex::OSC_A_LEVEL,                Pro800CCMessages::OSC_A_LEVEL},
        {Pro800LiveIndex::GLIDE_TIME,                 Pro800CCMessages::GLIDE_TIME},
        {Pro800LiveIndex::LFO_FREQ,                   Pro800CCMessages::LFO_MOD_FREQ},
        {Pro800LiveIndex::LFO_INITIAL_AMOUNT,         Pro800CCMessages::LFO_MOD_INITIAL_AMOUNT},
        {Pro800LiveIndex::MASTER_TUNE,                Pro800CCMessages::MASTER_TUNE},
        {Pro800LiveIndex::MASTER_VOLUME,              Pro800CCMessages::MASTER_VOLUME},
        {Pro800LiveIndex::OSC_B_LEVEL,                Pro800CCMessages::OSC_B_LEVEL},
        {Pro800LiveIndex::POLY_MOD_SOURCE_OSC_B,      Pro800CCMessages::POLY_MOD_SOURCE_OSC_B},
        {Pro800LiveIndex::POLY_MOD_SOURCE_FILTER_ENV, Pro800CCMessages::POLY_MOD_SOURCE_FILTER_ENV},
        {Pro800LiveIndex::NOISE_LEVEL,                Pro800CCMessages::NOISE_LEVEL},
        {Pro800LiveIndex::FILTER_CUTOFF,              Pro800CCMessages::FILTER_CUTOFF},
    };

    // the on/off switches, as CCs
    const std::map<Pro800PanelIndex, Pro800CCMessages> SWITCH_TO_CC =
    {
        {Pro800PanelIndex::SWITCH_OSC_A_SHAPE_RECT,      Pro800CCMessages::OSC_A_SHAPE_RECT},
        {Pro800PanelIndex::SWITCH_OSC_B_SHAPE_RECT,      Pro800CCMessages::OSC_B_SHAPE_RECT},
        {Pro800PanelIndex::SWITCH_OSC_A_SHAPE_SAW,       Pro800CCMessages::OSC_A_SHAPE_SAW},
        {Pro800PanelIndex::SWITCH_OSC_A_SHAPE_TRI,       Pro800CCMessages::OSC_A_SHAPE_TRI},
        {Pro800PanelIndex::SWITCH_OSC_A_SYNC,            Pro800CCMessages::OSC_A_SYNC},
        {Pro800PanelIndex::SWITCH_OSC_B_SHAPE_SAW,       Pro800CCMessages::OSC_B_SHAPE_SAW},
        {Pro800PanelIndex::SWITCH_OSC_B_SHAPE_TRI,       Pro800CCMessages::OSC_B_SHAPE_TRI},
        {Pro800PanelIndex::SWITCH_POLY_MOD_DEST_FREQ_A,  Pro800CCMessages::POLY_MOD_DEST_FREQ_A},
        {Pro800PanelIndex::SWITCH_POLY_MOD_DEST_FILTER,  Pro800CCMessages::POLY_MOD_DEST_FILTER},
        {Pro800PanelIndex::SWITCH_POLY_MOD_UNISON_TRACK, Pro800CCMessages::POLY_MOD_UNISON_TRACK},
        {Pro800PanelIndex::SWITCH_LFO_DEST_FREQ_AB,      Pro800CCMessages::LFO_MOD_DEST_FREQ_AB},
        {Pro800PanelIndex::SWITCH_LFO_DEST_PW_AB,        Pro800CCMessages::LFO_MOD_DEST_PW_AB},
        {Pro800PanelIndex::SWITCH_LFO_DEST_FILTER,       Pro800CCMessages::LFO_MOD_DEST_FILTER},
    };
    // clang-format on

    constexpr int MAX_LIVE_VALUE = 127;
    constexpr int MAX_PROGRAM_VALUE = 65535;
}

std::optional<Pro800ProgramFilterKeyboardTracking> Pro800PanelConversion::filterKeyboardFromBits (int full, int half)
{
    if (full != 0 && half != 0)
    {
        return std::nullopt;
    }
    if (full != 0)
    {
        return PROGRAM_FILTER_KEYBOARD_TRACKING_FULL;
    }
    if (half != 0)
    {
        return PROGRAM_FILTER_KEYBOARD_TRACKING_HALF;
    }
    return PROGRAM_FILTER_KEYBOARD_TRACKING_OFF;
}

std::optional<Pro800ProgramLfoShape> Pro800PanelConversion::lfoShapeFromSwitch (std::optional<int> previousShape, int switchValue)
{
    if (!previousShape.has_value() || *previousShape < 0 || *previousShape >= PROGRAM_LFO_SHAPE_NUM_VALUES)
    {
        return std::nullopt; // nothing to pick a pair from
    }

    const int shape = *previousShape;
    const bool switchOnTriSineSaw = (switchValue != 0);
    const bool shapeOnTriSineSaw = (shape % 2) != 0; // Triangle 1, Sine 3, Saw 5

    return static_cast<Pro800ProgramLfoShape> (shapeOnTriSineSaw == switchOnTriSineSaw ? shape : shape ^ 1);
}

int Pro800PanelConversion::liveValueToProgramRange (int value)
{
    const int clamped = value < 0 ? 0 : (value > MAX_LIVE_VALUE ? MAX_LIVE_VALUE : value);
    return (clamped * MAX_PROGRAM_VALUE + MAX_LIVE_VALUE / 2) / MAX_LIVE_VALUE;
}

Pro800ProgramField Pro800PanelConversion::liveIndexToProgramField (Pro800LiveIndex index)
{
    const auto entry = LIVE_TO_FIELD.find (index);
    return entry != LIVE_TO_FIELD.end() ? entry->second : Pro800ProgramField::NONE;
}

Pro800CCMessages Pro800PanelConversion::liveIndexToCC (Pro800LiveIndex index)
{
    const auto entry = LIVE_TO_CC.find (index);
    return entry != LIVE_TO_CC.end() ? entry->second : Pro800CCMessages::NONE;
}

Pro800PanelValues Pro800PanelConversion::toValues (const Pro800PanelState& state, std::optional<int> previousLfoShape)
{
    Pro800PanelValues values;

    for (const auto& [index, field] : SWITCH_TO_FIELD)
    {
        if (const auto reading = state.panel.find (index); reading != state.panel.end())
        {
            values.fields[field] = reading->second != 0 ? 1 : 0;
        }
    }

    // the three LFO destination switches share one bitmask field; only present if all three were read
    int lfoDestination = 0;
    bool allLfoDestinationsRead = true;
    for (const auto& [index, bit] : SWITCH_TO_LFO_DEST_BIT)
    {
        const auto reading = state.panel.find (index);
        if (reading == state.panel.end())
        {
            allLfoDestinationsRead = false;
            break;
        }
        lfoDestination |= (reading->second != 0) ? bit : 0;
    }
    if (allLfoDestinationsRead)
    {
        values.fields[Pro800ProgramField::LFO_DEST] = lfoDestination;
    }

    if (const auto shapeSwitch = state.panel.find (Pro800PanelIndex::SWITCH_LFO_SHAPE); shapeSwitch != state.panel.end())
    {
        if (const auto shape = lfoShapeFromSwitch (previousLfoShape, shapeSwitch->second))
        {
            values.fields[Pro800ProgramField::LFO_SHAPE] = *shape;
        }
    }

    const auto full = state.panel.find (Pro800PanelIndex::SWITCH_FILTER_KEYBOARD_FULL);
    const auto half = state.panel.find (Pro800PanelIndex::SWITCH_FILTER_KEYBOARD_HALF);
    if (full != state.panel.end() && half != state.panel.end())
    {
        if (const auto tracking = filterKeyboardFromBits (full->second, half->second))
        {
            values.fields[Pro800ProgramField::FILTER_KEY_TRACKING] = *tracking;
        }
    }

    for (const auto& [index, value] : state.live)
    {
        if (const auto field = liveIndexToProgramField (index); field != Pro800ProgramField::NONE)
        {
            values.fields[field] = liveValueToProgramRange (value);
        }
        else if (const auto cc = liveIndexToCC (index); cc != Pro800CCMessages::NONE)
        {
            values.ccValues[cc] = value < 0 ? 0 : (value > MAX_LIVE_VALUE ? MAX_LIVE_VALUE : value);
        }
    }

    return values;
}

std::map<Pro800CCMessages, int> Pro800PanelConversion::toCCValues (const Pro800PanelState& state, std::optional<int> previousLfoShape)
{
    std::map<Pro800CCMessages, int> ccValues;

    // the knobs: the reading is already the 7-bit value a CC carries, so nothing is lost here
    for (const auto& [index, value] : state.live)
    {
        if (const auto cc = liveIndexToCC (index); cc != Pro800CCMessages::NONE)
        {
            ccValues[cc] = value < 0 ? 0 : (value > MAX_LIVE_VALUE ? MAX_LIVE_VALUE : value);
        }
    }

    for (const auto& [index, cc] : SWITCH_TO_CC)
    {
        if (const auto reading = state.panel.find (index); reading != state.panel.end())
        {
            ccValues[cc] = reading->second != 0 ? CC_ON : CC_OFF;
        }
    }

    const auto full = state.panel.find (Pro800PanelIndex::SWITCH_FILTER_KEYBOARD_FULL);
    const auto half = state.panel.find (Pro800PanelIndex::SWITCH_FILTER_KEYBOARD_HALF);
    if (full != state.panel.end() && half != state.panel.end())
    {
        if (const auto tracking = filterKeyboardFromBits (full->second, half->second))
        {
            ccValues[Pro800CCMessages::FILTER_KEYBOARD_TRACKING] = Pro800CCUtils::ccFromProgramEnumValue (*tracking, PROGRAM_FILTER_KEYBOARD_TRACKING_NUM_VALUES);
        }
    }

    if (const auto shapeSwitch = state.panel.find (Pro800PanelIndex::SWITCH_LFO_SHAPE); shapeSwitch != state.panel.end())
    {
        if (const auto shape = lfoShapeFromSwitch (previousLfoShape, shapeSwitch->second))
        {
            ccValues[Pro800CCMessages::LFO_MOD_SHAPE] = Pro800CCUtils::ccFromProgramEnumValue (*shape, PROGRAM_LFO_SHAPE_NUM_VALUES);
        }
    }

    return ccValues;
}
