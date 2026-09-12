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

#include "Pro800Constants.h"
#include <map>

enum class Pro800ProgramField {
    NONE = -1, // placeholder for no field

    STORAGE_CODE = 0,
    PRESET_VERSION, // preset layout version, not the plugin version (VERSION is a compile definition)
    OSC_A_FREQ,
    OSC_A_SHAPE_SAW,
    OSC_A_SHAPE_TRI,
    OSC_A_SHAPE_RECT,
    OSC_A_SYNC,
    OSC_A_PULSE_WIDTH,
    OSC_A_LEVEL,

    OSC_B_FREQ,
    OSC_B_FINE_FREQ,
    OSC_B_SHAPE_SAW,
    OSC_B_SHAPE_TRI,
    OSC_B_SHAPE_RECT,
    OSC_B_PULSE_WIDTH,
    OSC_B_LEVEL,

    POLYMOD_SOURCE_FILTER_ENV,
    POLYMOD_SOURCE_OSC_B,
    POLYMOD_DEST_FREQ_A,
    POLYMOD_DEST_FILTER,
    POLYMOD_UNISON_TRACK,

    NOISE,

    LFO_FREQ,
    LFO_SHAPE,
    LFO_AMOUNT,
    LFO_DEST,
    LFO_SPEED,
    LFO_AFTERTOUCH_AMOUNT,
    LFO_MODULATION_DELAY,
    LFO_VIBRATO_FREQ,
    LFO_VIBRATO_AMOUNT,

    GLIDE_AMOUNT,
    GLIDE_MODE,

    FILTER_CUTOFF,
    FILTER_RESONANCE,
    FILTER_ENV_AMOUNT,
    FILTER_RELEASE,
    FILTER_SUSTAIN,
    FILTER_DECAY,
    FILTER_ATTACK,
    FILTER_KEY_TRACKING,
    FILTER_ENV_SHAPE,
    FILTER_ENV_SPEED,
    FILTER_VELOCITY,
    FILTER_AFTERTOUCH_AMOUNT,

    AMP_RELEASE,
    AMP_SUSTAIN,
    AMP_DECAY,
    AMP_ATTACK,
    AMP_ENV_SHAPE,
    AMP_ENV_SPEED,
    AMP_VELOCITY,
    AMP_AFTERTOUCH_AMOUNT,

    PITCHBEND_TARGET,
    PITCHBEND_RANGE,

    MODWHEEL_RANGE,
    MODWHEEL_TARGET,

    OSC_A_FREQ_POT_MODE,
    OSC_B_FREQ_POT_MODE,

    UNISON_DETUNE,

    ARP_MODE,

    VOICE_SPREAD_ENABLE,
    KEY_TRACKING_REF_NOTE,

    NAME_FIRST_CHAR,
    NAME_LAST_CHAR,

    VOICE1_OFFSET,
    VOICE2_OFFSET,
    VOICE3_OFFSET,
    VOICE4_OFFSET,
    VOICE5_OFFSET,
    VOICE6_OFFSET,
    VOICE7_OFFSET,
    VOICE8_OFFSET,

    TUNING_C,
    TUNING_C_SHARP,
    TUNING_D,
    TUNING_D_SHARP,
    TUNING_E,
    TUNING_F,
    TUNING_F_SHARP,
    TUNING_G,
    TUNING_G_SHARP,
    TUNING_A,
    TUNING_A_SHARP,
    TUNING_B

};

// clang-format off
enum Pro800ProgramLfoDestinationBitMask
{
    PROGRAM_LFO_DEST_FREQ_AB = 1,       // bit 0 --> Pro800CCMessages::LFO_MOD_DEST_FREQ_AB (on/off)
    PROGRAM_LFO_DEST_FILTER  = 2,       // bit 1 --> Pro800CCMessages::LFO_MOD_DEST_FILTER (on/off)
    PROGRAM_LFO_DEST_PW_AB   = 4,       // bit 2 --> Pro800CCMessages::LFO_MOD_DEST_PW_AB (on/off)
    PROGRAM_LFO_DEST_FREQ_A  = 8,       // bit 3 --> Pro800CCMessages::LFO_TARGET (0=A+B,33=A,66=B,99=A+B+VCA)
    PROGRAM_LFO_DEST_FREQ_B  = 16,      // bit 4 --> Pro800CCMessages::LFO_TARGET (0=A+B,33=A,66=B,99=A+B+VCA)
    PROGRAM_LFO_DEST_FREQ_AB_VCA  = 32, // bit 5 --> Pro800CCMessages::LFO_TARGET (0=A+B,33=A,66=B,99=A+B+VCA)
};
// clang-format on

enum Pro800ProgramFilterKeyboardTracking {
    PROGRAM_FILTER_KEYBOARD_TRACKING_OFF = 0,
    PROGRAM_FILTER_KEYBOARD_TRACKING_HALF = 1,
    PROGRAM_FILTER_KEYBOARD_TRACKING_FULL = 2,
    PROGRAM_FILTER_KEYBOARD_TRACKING_NUM_VALUES = 3
};

enum Pro800ProgramLfoShape {
    PROGRAM_LFO_SHAPE_PULSE = 0,
    PROGRAM_LFO_SHAPE_TRIANGLE = 1,
    PROGRAM_LFO_SHAPE_RANDOM = 2,
    PROGRAM_LFO_SHAPE_SINE = 3,
    PROGRAM_LFO_SHAPE_NOISE = 4,
    PROGRAM_LFO_SHAPE_SAW = 5,
    PROGRAM_LFO_SHAPE_NUM_VALUES = 6
};

enum Pro800ProgramLfoSpeed {
    PROGRAM_LFO_SPEED_SLOW = 0,
    PROGRAM_LFO_SPEED_FAST = 1,
    PROGRAM_LFO_SPEED_NUM_VALUES = 2
};

enum Pro800ProgramEnvelopeSpeed {
    PROGRAM_ENV_SPEED_FAST = 0,
    PROGRAM_ENV_SPEED_SLOW = 1,
    PROGRAM_ENV_SPEED_NUM_VALUES = 2
};

enum Pro800ProgramEnvelopeShape {
    PROGRAM_ENV_SHAPE_LINEAR = 0,
    PROGRAM_ENV_SHAPE_EXPONENTIAL = 1,
    PROGRAM_ENV_SHAPE_NUM_VALUES = 2
};

enum Pro800ProgramModWheelAmount {
    PROGRAM_MOD_WHEEL_AMOUNT_MIN = 0,
    PROGRAM_MOD_WHEEL_AMOUNT_LOW = 1,
    PROGRAM_MOD_WHEEL_AMOUNT_HIGH = 2,
    PROGRAM_MOD_WHEEL_AMOUNT_FULL = 3,
    PROGRAM_MOD_WHEEL_AMOUNT_NUM_VALUES = 4
};

enum Pro800ProgramModWheelTarget {
    PROGRAM_MOD_WHEEL_TARGET_LFO = 0,
    PROGRAM_MOD_WHEEL_TARGET_VIBRATO = 1,
    PROGRAM_MOD_WHEEL_TARGET_NUM_VALUES = 2
};

enum Pro800ProgramPitchBendTarget {
    PROGRAM_PITCH_BEND_TARGET_OFF = 0,
    PROGRAM_PITCH_BEND_TARGET_VCO = 1,
    PROGRAM_PITCH_BEND_TARGET_VCF = 2,
    PROGRAM_PITCH_BEND_TARGET_VOLUME = 3,
    PROGRAM_PITCH_BEND_TARGET_NUM_VALUES = 4
};

enum Pro800ProgramFreqPotMode {
    PROGRAM_FREQ_POT_MODE_FREE = 0,
    PROGRAM_FREQ_POT_MODE_SEMI = 1,
    PROGRAM_FREQ_POT_MODE_OCT = 2,
    PROGRAM_FREQ_POT_MODE_FIXED = 3,
    PROGRAM_FREQ_POT_MODE_NUM_VALUES = 4
};

enum Pro800ProgramKeyboardTrackingReference {
    PROGRAM_KEYBOARD_TRACKING_REF_C1 = 0,
    PROGRAM_KEYBOARD_TRACKING_REF_C2 = 1,
    PROGRAM_KEYBOARD_TRACKING_REF_C3 = 2,
    PROGRAM_KEYBOARD_TRACKING_REF_C4 = 3,
    PROGRAM_KEYBOARD_TRACKING_REF_NUM_VALUES = 4
};

enum Pro800ProgramOnOff {
    PROGRAM_OFF = 0,
    PROGRAM_ON = 1,
    PROGRAM_ON_OFF_NUM_VALUES = 2
};

enum Pro800ProgramGlideMode {
    PROGRAM_GLIDE_MODE_TIME = 0,
    PROGRAM_GLIDE_MODE_SPEED = 1,
    PROGRAM_GLIDE_MODE_NUM_VALUES = 2
};

enum Pro800ArpMode {
    PROGRAM_ARP_MODE_OFF = 0,
    PROGRAM_ARP_MODE_UP = 1,
    PROGRAM_ARP_MODE_DOWN = 2,
    PROGRAM_ARP_MODE_UP_DOWN = 3,
    PROGRAM_ARP_MODE_UP_AND_DOWN = 4,
    PROGRAM_ARP_MODE_RANDOM = 5,
    PROGRAM_ARP_ASSIGN = 6,
    PROGRAM_ARP_MODE_NUM_VALUES = 7
};

// inline: one instance for the whole program instead of one per translation unit
// clang-format off
inline const std::map<Pro800ProgramField, Pro800Parameter> PRO800_PROGRAM_FIELDS =
{
    // 0 = overflow
    {Pro800ProgramField::STORAGE_CODE,              {1, 4, "Storage Code"}},
    {Pro800ProgramField::PRESET_VERSION,                   {5, 1, "Version"}},

    {Pro800ProgramField::OSC_A_FREQ,                {6, 2, "Osc A Frequency"}},
    // 8 = overflow
    {Pro800ProgramField::OSC_A_LEVEL,               {9, 2, "Osc A Level"}},
    {Pro800ProgramField::OSC_A_PULSE_WIDTH,         {11, 2, "Osc A Pulse Width"}},

    {Pro800ProgramField::OSC_B_FREQ,                {13, 2, "Osc B Frequency"}},
    {Pro800ProgramField::OSC_B_LEVEL,               {15, 2, "Osc B Level"}},
    // 16 = overflow
    {Pro800ProgramField::OSC_B_PULSE_WIDTH,         {18, 2, "Osc B Pulse Width"}},
    {Pro800ProgramField::OSC_B_FINE_FREQ,           {20, 2, "Osc B Fine Frequency"}},

    {Pro800ProgramField::FILTER_CUTOFF,             {22, 2, "Filter Cutoff"}},
    // 24 = overflow
    {Pro800ProgramField::FILTER_RESONANCE,          {25, 2, "Filter Resonance"}},
    {Pro800ProgramField::FILTER_ENV_AMOUNT,         {27, 2, "Filter Envelope Amount"}},
    {Pro800ProgramField::FILTER_RELEASE,            {29, 2, "Filter Release"}},
    {Pro800ProgramField::FILTER_SUSTAIN,            {31, 2, "Filter Sustain"}},
    // 32 = overflow
    {Pro800ProgramField::FILTER_DECAY,              {34, 2, "Filter Decay"}},
    {Pro800ProgramField::FILTER_ATTACK,             {36, 2, "Filter Attack"}},

    {Pro800ProgramField::AMP_RELEASE,               {38, 2, "Amp Release"}},
    // 40 = overflow
    {Pro800ProgramField::AMP_SUSTAIN,               {41, 2, "Amp Sustain"}},
    {Pro800ProgramField::AMP_DECAY,                 {43, 2, "Amp Decay"}},
    {Pro800ProgramField::AMP_ATTACK,                {45, 2, "Amp Attack"}},

    {Pro800ProgramField::POLYMOD_SOURCE_FILTER_ENV, {47, 2, "Poly-Mod Source Filter Env"}},
    // 48 = overflow
    {Pro800ProgramField::POLYMOD_SOURCE_OSC_B,      {50, 2, "Poly-Mod Source Osc B"}},

    {Pro800ProgramField::LFO_FREQ,                  {52, 2, "LFO Frequency"}},
    {Pro800ProgramField::LFO_AMOUNT,                {54, 2, "LFO Amount"}},

    // 56 = overflow
    {Pro800ProgramField::GLIDE_AMOUNT,              {57, 2, "Glide Amount"}},

    {Pro800ProgramField::AMP_VELOCITY,              {59, 2, "Amp Velocity"}},
    {Pro800ProgramField::FILTER_VELOCITY,           {61, 2, "Filter Velocity"}},

    {Pro800ProgramField::OSC_A_SHAPE_SAW,           {63, 1, "Osc A Shape Saw", PROGRAM_ON_OFF_NUM_VALUES}},
    // 64 = overflow
    {Pro800ProgramField::OSC_A_SHAPE_TRI,           {65, 1, "Osc A Shape Tri", PROGRAM_ON_OFF_NUM_VALUES}},
    {Pro800ProgramField::OSC_A_SHAPE_RECT,          {66, 1, "Osc A Shape Rect", PROGRAM_ON_OFF_NUM_VALUES}},

    {Pro800ProgramField::OSC_B_SHAPE_SAW,           {67, 1, "Osc B Shape Saw", PROGRAM_ON_OFF_NUM_VALUES}},
    {Pro800ProgramField::OSC_B_SHAPE_TRI,           {68, 1, "Osc B Shape Tri", PROGRAM_ON_OFF_NUM_VALUES}},
    {Pro800ProgramField::OSC_B_SHAPE_RECT,          {69, 1, "Osc B Shape Rect", PROGRAM_ON_OFF_NUM_VALUES}},

    {Pro800ProgramField::OSC_A_SYNC,                {70, 1, "Osc A Sync", PROGRAM_ON_OFF_NUM_VALUES}},

    {Pro800ProgramField::POLYMOD_DEST_FREQ_A,       {71, 1, "Poly-Mod Dest Freq A", PROGRAM_ON_OFF_NUM_VALUES}},
    // 72 = overflow
    {Pro800ProgramField::POLYMOD_DEST_FILTER,       {73, 1, "Poly-Mod Dest Filter", PROGRAM_ON_OFF_NUM_VALUES}},

    {Pro800ProgramField::LFO_SHAPE,                 {74, 1, "LFO Shape", PROGRAM_LFO_SHAPE_NUM_VALUES}}, 
    {Pro800ProgramField::LFO_SPEED,                 {75, 1, "LFO Speed", PROGRAM_LFO_SPEED_NUM_VALUES}}, 

    {Pro800ProgramField::LFO_DEST,                  {76, 1, "LFO Destination"}}, // see: Pro800ProgramLfoDestinationBitMask

    {Pro800ProgramField::FILTER_KEY_TRACKING,       {77, 1, "Filter Keyboard Tracking", PROGRAM_FILTER_KEYBOARD_TRACKING_NUM_VALUES}},
    {Pro800ProgramField::FILTER_ENV_SHAPE,          {78, 1, "Filter Envelope Shape", PROGRAM_ENV_SHAPE_NUM_VALUES}},
    {Pro800ProgramField::FILTER_ENV_SPEED,          {79, 1, "Filter Envelope Speed", PROGRAM_ENV_SPEED_NUM_VALUES}},

    // 80 = overflow
    {Pro800ProgramField::AMP_ENV_SHAPE,             {81, 1, "Amp Envelope Shape", PROGRAM_ENV_SHAPE_NUM_VALUES}},
    {Pro800ProgramField::POLYMOD_UNISON_TRACK,      {82, 1, "Poly-Mod Unison Track", PROGRAM_ON_OFF_NUM_VALUES}},
    {Pro800ProgramField::PITCHBEND_TARGET,          {83, 1, "Pitchbend Target", PROGRAM_PITCH_BEND_TARGET_NUM_VALUES}},
    {Pro800ProgramField::MODWHEEL_RANGE,            {84, 1, "Mod Wheel Range", PROGRAM_MOD_WHEEL_AMOUNT_NUM_VALUES}},

    {Pro800ProgramField::OSC_A_FREQ_POT_MODE,       {85, 1, "Osc A Freq Pot Mode", PROGRAM_FREQ_POT_MODE_NUM_VALUES}},
    {Pro800ProgramField::OSC_B_FREQ_POT_MODE,       {86, 1, "Osc B Freq Pot Mode", PROGRAM_FREQ_POT_MODE_NUM_VALUES}},

    {Pro800ProgramField::LFO_MODULATION_DELAY,      {87, 2, "Modulation Delay"}},
    // 88 = overflow

    {Pro800ProgramField::LFO_VIBRATO_FREQ,          {90, 2, "Vibrato Freq"}},
    {Pro800ProgramField::LFO_VIBRATO_AMOUNT,        {92, 2, "Vibrato Amount"}},

    {Pro800ProgramField::UNISON_DETUNE,             {94, 2, "Unison Detune"}},

    // 96 = overflow
    {Pro800ProgramField::MODWHEEL_TARGET,           {97, 1, "Mod Wheel Target", PROGRAM_MOD_WHEEL_TARGET_NUM_VALUES}},

    // 98 = reserved
    {Pro800ProgramField::VOICE1_OFFSET,             {99, 1, "Voice 1 Offset"}},
    {Pro800ProgramField::VOICE2_OFFSET,             {100, 1, "Voice 2 Offset"}},
    {Pro800ProgramField::VOICE3_OFFSET,             {101, 1, "Voice 3 Offset"}},
    {Pro800ProgramField::VOICE4_OFFSET,             {102, 1, "Voice 4 Offset"}},
    {Pro800ProgramField::VOICE5_OFFSET,             {103, 1, "Voice 5 Offset"}},
    // 104 = overflow
    {Pro800ProgramField::VOICE6_OFFSET,             {105, 1, "Voice 6 Offset"}},
    {Pro800ProgramField::VOICE7_OFFSET,             {106, 1, "Voice 7 Offset"}},
    {Pro800ProgramField::VOICE8_OFFSET,             {107, 1, "Voice 8 Offset"}},

    {Pro800ProgramField::TUNING_C,                  {108, 4, "Tune Per Note - C"}},
    // 112 = overflow
    {Pro800ProgramField::TUNING_C_SHARP,            {113, 4, "Tune Per Note - C#"}},
    {Pro800ProgramField::TUNING_D,                  {117, 4, "Tune Per Note - D"}},
    // 120 = overflow
    {Pro800ProgramField::TUNING_D_SHARP,            {122, 4, "Tune Per Note - D#"}},
    {Pro800ProgramField::TUNING_E,                  {126, 4, "Tune Per Note - E"}},
    // 128 = overflow
    {Pro800ProgramField::TUNING_F,                  {131, 4, "Tune Per Note - F"}},
    {Pro800ProgramField::TUNING_F_SHARP,            {135, 4, "Tune Per Note - F#"}},
    // 136 = overflow
    {Pro800ProgramField::TUNING_G,                  {140, 4, "Tune Per Note - G"}},
    // 144 = overflow
    {Pro800ProgramField::TUNING_G_SHARP,            {145, 4, "Tune Per Note - G#"}},
    {Pro800ProgramField::TUNING_A,                  {149, 4, "Tune Per Note - A"}},
    // 152 = overflow
    {Pro800ProgramField::TUNING_A_SHARP,            {154, 4, "Tune Per Note - A#"}},
    {Pro800ProgramField::TUNING_B,                  {158, 4, "Tune Per Note - B"}},
    // 160 = overflow

    {Pro800ProgramField::NOISE,                     {163, 2, "Noise Amount"}},
    {Pro800ProgramField::AMP_AFTERTOUCH_AMOUNT,     {165, 2, "Amp Aftertouch Amount"}},
    {Pro800ProgramField::FILTER_AFTERTOUCH_AMOUNT,  {167, 2, "Filter Aftertouch Amount"}},
    // 168 = overflow

    {Pro800ProgramField::AMP_ENV_SPEED,             {170, 1, "Amp Envelope Speed", PROGRAM_ENV_SPEED_NUM_VALUES}},
    {Pro800ProgramField::ARP_MODE,                  {171, 1, "ARP Mode", PROGRAM_ARP_MODE_NUM_VALUES}}, // TODO: Add UI element

    {Pro800ProgramField::NAME_FIRST_CHAR,           {172, 1, "Preset Name (first char)"}},

    // 176 = overflow byte, not used by name
    // 184 = overflow byte, not used by name

    {Pro800ProgramField::NAME_LAST_CHAR,            {189, 1, "Preset Name (last char)"}},

    // only in preset version 110 and newer:
    {Pro800ProgramField::LFO_AFTERTOUCH_AMOUNT,     {190, 2, "LFO Aftertouch Amount"}},
    // 192 = overflow

    // only in preset version 111 and newer:
    {Pro800ProgramField::VOICE_SPREAD_ENABLE,       {193, 1, "Voice Spread Enable", PROGRAM_ON_OFF_NUM_VALUES}},
    {Pro800ProgramField::KEY_TRACKING_REF_NOTE,     {194, 1, "Key Tracking Ref Note", PROGRAM_KEYBOARD_TRACKING_REF_NUM_VALUES}},
    {Pro800ProgramField::GLIDE_MODE,                {195, 1, "Glide Mode", PROGRAM_GLIDE_MODE_NUM_VALUES}},

    {Pro800ProgramField::PITCHBEND_RANGE,           {196, 2, "Pitchbend Range"}},
   
};
// clang-format on
