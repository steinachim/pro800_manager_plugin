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

#include <map>
#include "Pro800Constants.h"

enum Pro800ProgramField
{
    PROGRAM_FIELD_NONE = -1, // placeholder for no field

    PROGRAM_FIELD_STORAGE_CODE = 0,
    PROGRAM_FIELD_VERSION,

    PROGRAM_FIELD_OSC_A_FREQ,
    PROGRAM_FIELD_OSC_A_SHAPE_SAW,
    PROGRAM_FIELD_OSC_A_SHAPE_TRI,
    PROGRAM_FIELD_OSC_A_SHAPE_RECT,
    PROGRAM_FIELD_OSC_A_SYNC,
    PROGRAM_FIELD_OSC_A_PULSE_WIDTH,
    PROGRAM_FIELD_OSC_A_LEVEL,

    PROGRAM_FIELD_OSC_B_FREQ,
    PROGRAM_FIELD_OSC_B_FINE_FREQ,
    PROGRAM_FIELD_OSC_B_SHAPE_SAW,
    PROGRAM_FIELD_OSC_B_SHAPE_TRI,
    PROGRAM_FIELD_OSC_B_SHAPE_RECT,
    PROGRAM_FIELD_OSC_B_PULSE_WIDTH,
    PROGRAM_FIELD_OSC_B_LEVEL,

    PROGRAM_FIELD_POLYMOD_SOURCE_FILTER_ENV,
    PROGRAM_FIELD_POLYMOD_SOURCE_OSC_B,
    PROGRAM_FIELD_POLYMOD_DEST_FREQ_A,
    PROGRAM_FIELD_POLYMOD_DEST_FILTER,
    PROGRAM_FIELD_POLYMOD_UNISON_TRACK,

    PROGRAM_FIELD_NOISE,

    PROGRAM_FIELD_LFO_FREQ,
    PROGRAM_FIELD_LFO_SHAPE,
    PROGRAM_FIELD_LFO_AMOUNT,
    PROGRAM_FIELD_LFO_DEST,
    PROGRAM_FIELD_LFO_SPEED,
    PROGRAM_FIELD_LFO_AFTERTOUCH_AMOUNT,
    PROGRAM_FIELD_LFO_MODULATION_DELAY,
    PROGRAM_FIELD_LFO_VIBRATO_FREQ,
    PROGRAM_FIELD_LFO_VIBRATO_AMOUNT,

    PROGRAM_FIELD_GLIDE_AMOUNT,
    PROGRAM_FIELD_GLIDE_MODE,

    PROGRAM_FIELD_FILTER_CUTOFF,
    PROGRAM_FIELD_FILTER_RESONANCE,
    PROGRAM_FIELD_FILTER_ENV_AMOUNT,
    PROGRAM_FIELD_FILTER_RELEASE,
    PROGRAM_FIELD_FILTER_SUSTAIN,
    PROGRAM_FIELD_FILTER_DECAY,
    PROGRAM_FIELD_FILTER_ATTACK,
    PROGRAM_FIELD_FILTER_KEY_TRACKING,
    PROGRAM_FIELD_FILTER_ENV_SHAPE,
    PROGRAM_FIELD_FILTER_ENV_SPEED,
    PROGRAM_FIELD_FILTER_VELOCITY,
    PROGRAM_FIELD_FILTER_AFTERTOUCH_AMOUNT,

    PROGRAM_FIELD_AMP_RELEASE,
    PROGRAM_FIELD_AMP_SUSTAIN,
    PROGRAM_FIELD_AMP_DECAY,
    PROGRAM_FIELD_AMP_ATTACK,
    PROGRAM_FIELD_AMP_ENV_SHAPE,
    PROGRAM_FIELD_AMP_ENV_SPEED,
    PROGRAM_FIELD_AMP_VELOCITY,
    PROGRAM_FIELD_AMP_AFTERTOUCH_AMOUNT,

    PROGRAM_FIELD_PITCHBEND_TARGET,
    PROGRAM_FIELD_PITCHBEND_RANGE,

    PROGRAM_FIELD_MODWHEEL_RANGE,
    PROGRAM_FIELD_MODWHEEL_TARGET,

    PROGRAM_FIELD_OSC_A_FREQ_POT_MODE,
    PROGRAM_FIELD_OSC_B_FREQ_POT_MODE,

    PROGRAM_FIELD_UNISON_DETUNE,

    PROGRAM_FIELD_ARP_MODE,

    PROGRAM_FIELD_VOICE_SPREAD_ENABLE,
    PROGRAM_FIELD_KEY_TRACKING_REF_NOTE,

    PROGRAM_FIELD_NAME,
    PROGRAM_FIELD_NAME_FIRST_CHAR,
    PROGRAM_FIELD_NAME_LAST_CHAR,

    PROGRAM_FIELD_VOICE1_OFFSET,
    PROGRAM_FIELD_VOICE2_OFFSET,
    PROGRAM_FIELD_VOICE3_OFFSET,
    PROGRAM_FIELD_VOICE4_OFFSET,
    PROGRAM_FIELD_VOICE5_OFFSET,
    PROGRAM_FIELD_VOICE6_OFFSET,
    PROGRAM_FIELD_VOICE7_OFFSET,
    PROGRAM_FIELD_VOICE8_OFFSET,

    PROGRAM_FIELD_TUNING_C,
    PROGRAM_FIELD_TUNING_C_SHARP,
    PROGRAM_FIELD_TUNING_D,
    PROGRAM_FIELD_TUNING_D_SHARP,
    PROGRAM_FIELD_TUNING_E,
    PROGRAM_FIELD_TUNING_F,
    PROGRAM_FIELD_TUNING_F_SHARP,
    PROGRAM_FIELD_TUNING_G,
    PROGRAM_FIELD_TUNING_G_SHARP,
    PROGRAM_FIELD_TUNING_A,
    PROGRAM_FIELD_TUNING_A_SHARP,
    PROGRAM_FIELD_TUNING_B
    
};

enum Pro800ProgramLfoDestinationBitMask
{
    PROGRAM_LFO_DEST_FREQ_AB = 1,       // bit 0 --> CC_LFO_MOD_DEST_FREQ_AB (on/off)
    PROGRAM_LFO_DEST_FILTER  = 2,       // bit 1 --> CC_LFO_MOD_DEST_FILTER (on/off)
    PROGRAM_LFO_DEST_PW_AB   = 4,       // bit 2 --> CC_LFO_MOD_DEST_PW_AB (on/off)
    PROGRAM_LFO_DEST_FREQ_A  = 8,       // bit 3 --> CC_LFO_TARGET (0=A+B,33=A,66=B,99=A+B+VCA)
    PROGRAM_LFO_DEST_FREQ_B  = 16,      // bit 4 --> CC_LFO_TARGET (0=A+B,33=A,66=B,99=A+B+VCA)
    PROGRAM_LFO_DEST_FREQ_AB_VCA  = 32, // bit 5 --> CC_LFO_TARGET (0=A+B,33=A,66=B,99=A+B+VCA)
};

enum Pro800ProgramFilterKeyboardTracking
{
    PROGRAM_FILTER_KEYBOARD_TRACKING_OFF = 0,
    PROGRAM_FILTER_KEYBOARD_TRACKING_HALF = 1,
    PROGRAM_FILTER_KEYBOARD_TRACKING_FULL = 2,
    PROGRAM_FILTER_KEYBOARD_TRACKING_NUM_VALUES = 3
};

enum Pro800ProgramLfoShape
{
    PROGRAM_LFO_SHAPE_PULSE = 0,
    PROGRAM_LFO_SHAPE_TRIANGLE = 1,
    PROGRAM_LFO_SHAPE_RANDOM = 2,
    PROGRAM_LFO_SHAPE_SINE = 3,
    PROGRAM_LFO_SHAPE_NOISE = 4,
    PROGRAM_LFO_SHAPE_SAW = 5,
    PROGRAM_LFO_SHAPE_NUM_VALUES = 6
};

enum Pro800ProgramLfoSpeed
{
    PROGRAM_LFO_SPEED_SLOW = 0,
    PROGRAM_LFO_SPEED_FAST = 1,
    PROGRAM_LFO_SPEED_NUM_VALUES = 2
};

enum Pro800ProgramEnvelopeSpeed
{
    PROGRAM_ENV_SPEED_FAST = 0,
    PROGRAM_ENV_SPEED_SLOW = 1,
    PROGRAM_ENV_SPEED_NUM_VALUES = 2
};

enum Pro800ProgramEnvelopeShape
{
    PROGRAM_ENV_SHAPE_LINEAR = 0,
    PROGRAM_ENV_SHAPE_EXPONENTIAL = 1,
    PROGRAM_ENV_SHAPE_NUM_VALUES = 2
};

enum Pro800ProgramModWheelAmount
{
    PROGRAM_MOD_WHEEL_AMOUNT_MIN = 0,
    PROGRAM_MOD_WHEEL_AMOUNT_LOW = 1,
    PROGRAM_MOD_WHEEL_AMOUNT_HIGH = 2,
    PROGRAM_MOD_WHEEL_AMOUNT_FULL = 3,
    PROGRAM_MOD_WHEEL_AMOUNT_NUM_VALUES = 4
};

enum Pro800ProgramModWheelTarget
{
    PROGRAM_MOD_WHEEL_TARGET_LFO = 0,
    PROGRAM_MOD_WHEEL_TARGET_VIBRATO = 1,
    PROGRAM_MOD_WHEEL_TARGET_NUM_VALUES = 2
};

enum Pro800ProgramPitchBendTarget
{
    PROGRAM_PITCH_BEND_TARGET_OFF = 0,
    PROGRAM_PITCH_BEND_TARGET_VCO = 1,
    PROGRAM_PITCH_BEND_TARGET_VCF = 2,
    PROGRAM_PITCH_BEND_TARGET_VOLUME = 3,
    PROGRAM_PITCH_BEND_TARGET_NUM_VALUES = 4
};

enum Pro800ProgramFreqPotMode
{
    PROGRAM_FREQ_POT_MODE_FREE = 0,
    PROGRAM_FREQ_POT_MODE_SEMI = 1,
    PROGRAM_FREQ_POT_MODE_OCT = 2,
    PROGRAM_FREQ_POT_MODE_FIXED = 3,
    PROGRAM_FREQ_POT_MODE_NUM_VALUES = 4
};

enum Pro800ProgramKeyboardTrackingReference
{
    PROGRAM_KEYBOARD_TRACKING_REF_C1 = 0,
    PROGRAM_KEYBOARD_TRACKING_REF_C2 = 1,
    PROGRAM_KEYBOARD_TRACKING_REF_C3 = 2,
    PROGRAM_KEYBOARD_TRACKING_REF_C4 = 3,
    PROGRAM_KEYBOARD_TRACKING_REF_NUM_VALUES = 4
};

enum Pro800ProgramOnOff
{
    PROGRAM_OFF = 0,
    PROGRAM_ON = 1,
    PROGRAM_ON_OFF_NUM_VALUES = 2
};

enum Pro800ProgramGlideMode
{
    PROGRAM_GLIDE_MODE_TIME = 0,
    PROGRAM_GLIDE_MODE_SPEED = 1,
    PROGRAM_GLIDE_MODE_NUM_VALUES = 2
};

enum Pro800ArpMode
{
    PROGRAM_ARP_MODE_OFF = 0,
    PROGRAM_ARP_MODE_UP = 1,
    PROGRAM_ARP_MODE_DOWN = 2,
    PROGRAM_ARP_MODE_UP_DOWN = 3,
    PROGRAM_ARP_MODE_UP_AND_DOWN = 4,
    PROGRAM_ARP_MODE_RANDOM = 5,
    PROGRAM_ARP_ASSIGN = 6,
    PROGRAM_ARP_MODE_NUM_VALUES = 7
};

const std::map<Pro800ProgramField, Pro800Parameter> PRO800_PROGRAM_FIELDS =
{
    // 0 = overflow
    {PROGRAM_FIELD_STORAGE_CODE,              {1, 4, "Storage Code"}},
    {PROGRAM_FIELD_VERSION,                   {5, 1, "Version"}},

    {PROGRAM_FIELD_OSC_A_FREQ,                {6, 2, "Osc A Frequency"}},
    // 8 = overflow
    {PROGRAM_FIELD_OSC_A_LEVEL,               {9, 2, "Osc A Level"}},
    {PROGRAM_FIELD_OSC_A_PULSE_WIDTH,         {11, 2, "Osc A Pulse Width"}},

    {PROGRAM_FIELD_OSC_B_FREQ,                {13, 2, "Osc B Frequency"}},
    {PROGRAM_FIELD_OSC_B_LEVEL,               {15, 2, "Osc B Level"}},
    // 16 = overflow
    {PROGRAM_FIELD_OSC_B_PULSE_WIDTH,         {18, 2, "Osc B Pulse Width"}},
    {PROGRAM_FIELD_OSC_B_FINE_FREQ,           {20, 2, "Osc B Fine Frequency"}},

    {PROGRAM_FIELD_FILTER_CUTOFF,             {22, 2, "Filter Cutoff"}},
    // 24 = overflow
    {PROGRAM_FIELD_FILTER_RESONANCE,          {25, 2, "Filter Resonance"}},
    {PROGRAM_FIELD_FILTER_ENV_AMOUNT,         {27, 2, "Filter Envelope Amount"}},
    {PROGRAM_FIELD_FILTER_RELEASE,            {29, 2, "Filter Release"}},
    {PROGRAM_FIELD_FILTER_SUSTAIN,            {31, 2, "Filter Sustain"}},
    // 32 = overflow
    {PROGRAM_FIELD_FILTER_DECAY,              {34, 2, "Filter Decay"}},
    {PROGRAM_FIELD_FILTER_ATTACK,             {36, 2, "Filter Attack"}},

    {PROGRAM_FIELD_AMP_RELEASE,               {38, 2, "Amp Release"}},
    // 40 = overflow
    {PROGRAM_FIELD_AMP_SUSTAIN,               {41, 2, "Amp Sustain"}},
    {PROGRAM_FIELD_AMP_DECAY,                 {43, 2, "Amp Decay"}},
    {PROGRAM_FIELD_AMP_ATTACK,                {45, 2, "Amp Attack"}},

    {PROGRAM_FIELD_POLYMOD_SOURCE_FILTER_ENV, {47, 2, "Poly-Mod Source Filter Env"}},
    // 48 = overflow
    {PROGRAM_FIELD_POLYMOD_SOURCE_OSC_B,      {50, 2, "Poly-Mod Source Osc B"}},

    {PROGRAM_FIELD_LFO_FREQ,                  {52, 2, "LFO Frequency"}},
    {PROGRAM_FIELD_LFO_AMOUNT,                {54, 2, "LFO Amount"}},

    // 56 = overflow
    {PROGRAM_FIELD_GLIDE_AMOUNT,              {57, 2, "Glide Amount"}},

    {PROGRAM_FIELD_AMP_VELOCITY,              {59, 2, "Amp Velocity"}},
    {PROGRAM_FIELD_FILTER_VELOCITY,           {61, 2, "Filter Velocity"}},

    {PROGRAM_FIELD_OSC_A_SHAPE_SAW,           {63, 1, "Osc A Shape Saw", PROGRAM_ON_OFF_NUM_VALUES}},
    // 64 = overflow
    {PROGRAM_FIELD_OSC_A_SHAPE_TRI,           {65, 1, "Osc A Shape Tri", PROGRAM_ON_OFF_NUM_VALUES}},
    {PROGRAM_FIELD_OSC_A_SHAPE_RECT,          {66, 1, "Osc A Shape Rect", PROGRAM_ON_OFF_NUM_VALUES}},

    {PROGRAM_FIELD_OSC_B_SHAPE_SAW,           {67, 1, "Osc B Shape Saw", PROGRAM_ON_OFF_NUM_VALUES}},
    {PROGRAM_FIELD_OSC_B_SHAPE_TRI,           {68, 1, "Osc B Shape Tri", PROGRAM_ON_OFF_NUM_VALUES}},
    {PROGRAM_FIELD_OSC_B_SHAPE_RECT,          {69, 1, "Osc B Shape Rect", PROGRAM_ON_OFF_NUM_VALUES}},

    {PROGRAM_FIELD_OSC_A_SYNC,                {70, 1, "Osc A Sync", PROGRAM_ON_OFF_NUM_VALUES}},

    {PROGRAM_FIELD_POLYMOD_DEST_FREQ_A,       {71, 1, "Poly-Mod Dest Freq A", PROGRAM_ON_OFF_NUM_VALUES}},
    // 72 = overflow
    {PROGRAM_FIELD_POLYMOD_DEST_FILTER,       {73, 1, "Poly-Mod Dest Filter", PROGRAM_ON_OFF_NUM_VALUES}},

    {PROGRAM_FIELD_LFO_SHAPE,                 {74, 1, "LFO Shape", PROGRAM_LFO_SHAPE_NUM_VALUES}}, 
    {PROGRAM_FIELD_LFO_SPEED,                 {75, 1, "LFO Speed", PROGRAM_LFO_SPEED_NUM_VALUES}}, 

    {PROGRAM_FIELD_LFO_DEST,                  {76, 1, "LFO Destination"}}, // see: Pro800ProgramLfoDestinationBitMask

    {PROGRAM_FIELD_FILTER_KEY_TRACKING,       {77, 1, "Filter Keyboard Tracking", PROGRAM_FILTER_KEYBOARD_TRACKING_NUM_VALUES}},
    {PROGRAM_FIELD_FILTER_ENV_SHAPE,          {78, 1, "Filter Envelope Shape", PROGRAM_ENV_SHAPE_NUM_VALUES}},
    {PROGRAM_FIELD_FILTER_ENV_SPEED,          {79, 1, "Filter Envelope Speed", PROGRAM_ENV_SPEED_NUM_VALUES}},

    // 80 = overflow
    {PROGRAM_FIELD_AMP_ENV_SHAPE,             {81, 1, "Amp Envelope Shape", PROGRAM_ENV_SHAPE_NUM_VALUES}},
    {PROGRAM_FIELD_POLYMOD_UNISON_TRACK,      {82, 1, "Poly-Mod Unison Track", PROGRAM_ON_OFF_NUM_VALUES}},
    {PROGRAM_FIELD_PITCHBEND_TARGET,          {83, 1, "Pitchbend Target", PROGRAM_PITCH_BEND_TARGET_NUM_VALUES}},
    {PROGRAM_FIELD_MODWHEEL_RANGE,            {84, 1, "Mod Wheel Range", PROGRAM_MOD_WHEEL_AMOUNT_NUM_VALUES}},

    {PROGRAM_FIELD_OSC_A_FREQ_POT_MODE,       {85, 1, "Osc A Freq Pot Mode", PROGRAM_FREQ_POT_MODE_NUM_VALUES}},
    {PROGRAM_FIELD_OSC_B_FREQ_POT_MODE,       {86, 1, "Osc B Freq Pot Mode", PROGRAM_FREQ_POT_MODE_NUM_VALUES}},

    {PROGRAM_FIELD_LFO_MODULATION_DELAY,      {87, 2, "Modulation Delay"}},
    // 88 = overflow

    {PROGRAM_FIELD_LFO_VIBRATO_FREQ,          {90, 2, "Vibrato Freq"}},
    {PROGRAM_FIELD_LFO_VIBRATO_AMOUNT,        {92, 2, "Vibrato Amount"}},

    {PROGRAM_FIELD_UNISON_DETUNE,             {94, 2, "Unison Detune"}},

    // 96 = overflow
    {PROGRAM_FIELD_MODWHEEL_TARGET,           {97, 1, "Mod Wheel Target", PROGRAM_MOD_WHEEL_TARGET_NUM_VALUES}},

    // 98 = reserved
    {PROGRAM_FIELD_VOICE1_OFFSET,             {99, 1, "Voice 1 Offset"}},
    {PROGRAM_FIELD_VOICE2_OFFSET,             {100, 1, "Voice 2 Offset"}},
    {PROGRAM_FIELD_VOICE3_OFFSET,             {101, 1, "Voice 3 Offset"}},
    {PROGRAM_FIELD_VOICE4_OFFSET,             {102, 1, "Voice 4 Offset"}},
    {PROGRAM_FIELD_VOICE5_OFFSET,             {103, 1, "Voice 5 Offset"}},
    // 104 = overflow
    {PROGRAM_FIELD_VOICE6_OFFSET,             {105, 1, "Voice 6 Offset"}},
    {PROGRAM_FIELD_VOICE7_OFFSET,             {106, 1, "Voice 7 Offset"}},
    {PROGRAM_FIELD_VOICE8_OFFSET,             {107, 1, "Voice 8 Offset"}},

    {PROGRAM_FIELD_TUNING_C,                  {108, 4, "Tune Per Note - C"}},
    // 112 = overflow
    {PROGRAM_FIELD_TUNING_C_SHARP,            {113, 4, "Tune Per Note - C#"}},
    {PROGRAM_FIELD_TUNING_D,                  {117, 4, "Tune Per Note - D"}},
    // 120 = overflow
    {PROGRAM_FIELD_TUNING_D_SHARP,            {122, 4, "Tune Per Note - D#"}},
    {PROGRAM_FIELD_TUNING_E,                  {126, 4, "Tune Per Note - E"}},
    // 128 = overflow
    {PROGRAM_FIELD_TUNING_F,                  {131, 4, "Tune Per Note - F"}},
    {PROGRAM_FIELD_TUNING_F_SHARP,            {135, 4, "Tune Per Note - F#"}},
    // 136 = overflow
    {PROGRAM_FIELD_TUNING_G,                  {140, 4, "Tune Per Note - G"}},
    // 144 = overflow
    {PROGRAM_FIELD_TUNING_G_SHARP,            {145, 4, "Tune Per Note - G#"}},
    {PROGRAM_FIELD_TUNING_A,                  {149, 4, "Tune Per Note - A"}},
    // 152 = overflow
    {PROGRAM_FIELD_TUNING_A_SHARP,            {154, 4, "Tune Per Note - A#"}},
    {PROGRAM_FIELD_TUNING_B,                  {158, 4, "Tune Per Note - B"}},
    // 160 = overflow

    {PROGRAM_FIELD_NOISE,                     {163, 2, "Noise Amount"}},
    {PROGRAM_FIELD_AMP_AFTERTOUCH_AMOUNT,     {165, 2, "Amp Aftertouch Amount"}},
    {PROGRAM_FIELD_FILTER_AFTERTOUCH_AMOUNT,  {167, 2, "Filter Aftertouch Amount"}},
    // 168 = overflow

    {PROGRAM_FIELD_AMP_ENV_SPEED,             {170, 1, "Amp Envelope Speed", PROGRAM_ENV_SPEED_NUM_VALUES}},
    {PROGRAM_FIELD_ARP_MODE,                  {171, 1, "ARP Mode", PROGRAM_ARP_MODE_NUM_VALUES}}, // TODO: Add UI element

    {PROGRAM_FIELD_NAME_FIRST_CHAR,           {172, 1, "Preset Name (first char)"}},

    // 176 = overflow byte, not used by name
    // 184 = overflow byte, not used by name

    {PROGRAM_FIELD_NAME_LAST_CHAR,            {189, 1, "Preset Name (last char)"}},

    // only in preset version 110 and newer:
    {PROGRAM_FIELD_LFO_AFTERTOUCH_AMOUNT,     {190, 2, "LFO Aftertouch Amount"}},
    // 192 = overflow

    // only in preset version 111 and newer:
    {PROGRAM_FIELD_VOICE_SPREAD_ENABLE,       {193, 1, "Voice Spread Enable", PROGRAM_ON_OFF_NUM_VALUES}},
    {PROGRAM_FIELD_KEY_TRACKING_REF_NOTE,     {194, 1, "Key Tracking Ref Note", PROGRAM_KEYBOARD_TRACKING_REF_NUM_VALUES}},
    {PROGRAM_FIELD_GLIDE_MODE,                {195, 1, "Glide Mode", PROGRAM_GLIDE_MODE_NUM_VALUES}},

    {PROGRAM_FIELD_PITCHBEND_RANGE,           {196, 2, "Pitchbend Range"}},
   
};
