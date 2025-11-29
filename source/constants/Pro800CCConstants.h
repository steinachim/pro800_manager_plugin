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

// Note:
// Values for enum parameters can be mapped to CC values using Pro800CCUtils::ccFromProgramEnumValue()
// and Pro800CCUtils::programEnumValueFromCC().

enum Pro800CCMessages
{
    CC_NONE = -1, // placeholder for no CC

    CC_BANK_SELECT = 0,

    // Front Panel
    CC_OSC_A_FREQ = 8,
    CC_OSC_A_LEVEL = 9,
    CC_OSC_A_PULSE_WIDTH = 10,
    CC_OSC_A_SYNC = 54,
    CC_OSC_A_SHAPE_SAW = 48,
    CC_OSC_A_SHAPE_TRI = 49,
    CC_OSC_A_SHAPE_RECT = 50,

    CC_OSC_B_FREQ = 11,
    CC_OSC_B_FINE = 14,
    CC_OSC_B_PULSE_WIDTH = 13,
    CC_OSC_B_LEVEL = 12,
    CC_OSC_B_SHAPE_SAW = 51,
    CC_OSC_B_SHAPE_TRI = 52,
    CC_OSC_B_SHAPE_RECT = 53,

    CC_POLY_MOD_SOURCE_FILTER_ENV = 26,
    CC_POLY_MOD_SOURCE_OSC_B = 27,
    CC_POLY_MOD_DEST_FREQ_A = 55,
    CC_POLY_MOD_DEST_FILTER = 56,
    CC_POLY_MOD_UNISON_TRACK = 65,

    CC_NOISE_LEVEL = 37,

    CC_LFO_MOD_FREQ = 28,
    CC_LFO_MOD_SHAPE = 57, // see: Pro800ProgramLfoShape
    CC_LFO_MOD_INITIAL_AMOUNT = 29,
    CC_LFO_MOD_DEST_FREQ_AB = 74, // see: Pro800ProgramLfoDestinationBitMask
    CC_LFO_MOD_DEST_PW_AB = 76,   // see: Pro800ProgramLfoDestinationBitMask
    CC_LFO_MOD_DEST_FILTER = 75,  // see: Pro800ProgramLfoDestinationBitMask

    CC_GLIDE_TIME = 30,

    CC_FILTER_CUTOFF = 15,
    CC_FILTER_RESONANCE = 16,
    CC_FILTER_ENV_AMOUNT = 17,
    CC_FILTER_ATTACK = 21,
    CC_FILTER_DECAY = 20,
    CC_FILTER_SUSTAIN = 19,
    CC_FILTER_RELEASE = 18,
    CC_FILTER_KEYBOARD_TRACKING = 60, // see: Pro800ProgramFilterKeyboardTracking
    
    CC_AMP_ATTACK = 25,
    CC_AMP_DECAY = 24,
    CC_AMP_SUSTAIN = 23,
    CC_AMP_RELEASE = 22,

    CC_MASTER_TUNE = 3,
    CC_MASTER_VOLUME = 7,

    // Menu: Performance 1
    CC_LFO_TARGET = 59, // see: Pro800ProgramLfoDestinationBitMask
    CC_LFO_SPEED = 58, // see: Pro800ProgramLfoSpeed
    
    // Menu: Performance 2
    CC_VIBRATO_AMOUNT = 35,
    CC_VIBRATO_SPEED = 34,

    // Menu: Performance 3
    CC_MOD_WHEEL_AMOUNT = 67, // see: Pro800ProgramModWheelAmount
    CC_MOD_WHEEL_TARGET = 70, // see: Pro800ProgramModWheelTarget
    CC_MODULATION_DELAY = 33,

    // Menu: Performance 4
    CC_VCA_ENV_SHAPE = 63, // see: Pro800ProgramEnvelopeShape
    CC_VCA_ENV_SPEED = 72, // see: Pro800ProgramEnvelopeSpeed

    CC_VCF_ENV_SHAPE = 61, // see: Pro800ProgramEnvelopeShape
    CC_VCF_ENV_SPEED = 62, // see: Pro800ProgramEnvelopeSpeed

    // Menu: Performance 5
    CC_PITCH_BEND_TARGET = 66, // see: Pro800ProgramPitchBendTarget
    CC_PITCH_BEND_RANGE = 42,  // 0-31 semitones (0-3 = 0, 4-7 = 1, 8-11 = 2, ..., 124-127 = 31)

    // Menu: Performance 6
    CC_OSC_A_FREQ_POT_MODE = 68, // see: Pro800ProgramFreqPotMode
    CC_OSC_B_FREQ_POT_MODE = 69, // see: Pro800ProgramFreqPotMode
    CC_KEYBOARD_TRACKING_REF = 78, // see: Pro800ProgramKeyboardTrackingReference

    // Menu: Performance 7
    CC_VCA_VELOCITY_AMOUNT = 31,
    CC_VCF_VELOCITY_AMOUNT = 32,

    // Menu: Performance 8
    CC_VCA_AFTERTOUCH_AMOUNT = 39,
    CC_VCF_AFTERTOUCH_AMOUNT = 40,
    CC_LFO_AFTERTOUCH_AMOUNT = 41,

    // Menu: Performance 9
    CC_UNISON_SPREAD_DETUNE = 36,
    CC_VOICE_SPREAD_ENABLE = 77, // see: Pro800ProgramOnOff

    // Menu: Performance 0
    CC_GLIDE_MODE = 79 // see: Pro800ProgramGlideMode
};

enum Pro800CcLfoTarget
{
    CC_LFO_TARGET_OSC_AB = 0, 
    CC_LFO_TARGET_OSC_A = 33, 
    CC_LFO_TARGET_OSC_B = 66, 
    CC_LFO_TARGET_VCA = 99    
};

enum Pro800CcOnOff
{
    CC_OFF = 0,
    CC_ON = 64
};
