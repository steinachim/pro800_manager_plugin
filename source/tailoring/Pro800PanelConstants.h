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
#include <map>
#include <string>

/**
 * The control-surface indices read with 0x70 (answered by 0x71 <index> <value>, see docs/Pro800SysExMessages.md).
 * Buttons are momentary (1 only while physically held), the panel switches and the rear DIP switches latch.
 * The button indices are also the press codes of the 0x71 request.
 *
 * This is the physical panel, not the sound: loading a preset does not move the knobs and switches.
 */
// clang-format off
enum class Pro800PanelIndex : uint8_t
{
    // buttons
    BUTTON_NUMPAD_0 = 0x00,
    BUTTON_NUMPAD_1 = 0x01,
    BUTTON_NUMPAD_2 = 0x02,
    BUTTON_NUMPAD_3 = 0x03,
    BUTTON_NUMPAD_4 = 0x04,
    BUTTON_NUMPAD_5 = 0x05,
    BUTTON_NUMPAD_6 = 0x06,
    BUTTON_NUMPAD_7 = 0x07,
    BUTTON_NUMPAD_8 = 0x08,
    BUTTON_NUMPAD_9 = 0x09,
    BUTTON_ARP_UP_DOWN = 0x0A,
    BUTTON_ARP_ASSIGN = 0x0B,
    BUTTON_PRESET = 0x0C,      // as a press: also recalls the preset the settings block points at
    BUTTON_REC = 0x0D,
    BUTTON_PERF = 0x0E,
    BUTTON_SETTINGS = 0x0F,
    BUTTON_SEQ_1 = 0x10,
    BUTTON_SEQ_2 = 0x11,
    BUTTON_TUNE = 0x12,
    BUTTON_SYNC_CLOCK = 0x13,
    BUTTON_SYNC_SOURCE = 0x14,

    // press codes without a button behind them; always read 0
    UNUSED_15 = 0x15,          // accepted as a press, does nothing
    VALUE_WHEEL_NEXT = 0x16,   // as a press: steps the preset selection (preset mode) or a setting in its change state
    VALUE_WHEEL_PREV = 0x17,

    // panel switches
    SWITCH_OSC_A_SHAPE_RECT = 0x18,
    SWITCH_OSC_B_SHAPE_RECT = 0x19,
    SWITCH_FILTER_KEYBOARD_FULL = 0x1A, // the 3-position switch is two bits: Off = (0,0), Half = (0,1), Full = (1,0)
    SWITCH_FILTER_KEYBOARD_HALF = 0x1B,
    SWITCH_LFO_SHAPE = 0x1C,            // 1 = Tri/Sine/Saw, 0 = Pulse/Random/Noise
    SWITCH_LFO_DEST_FREQ_AB = 0x1D,
    SWITCH_LFO_DEST_PW_AB = 0x1E,
    SWITCH_LFO_DEST_FILTER = 0x1F,
    SWITCH_OSC_A_SHAPE_SAW = 0x20,
    SWITCH_OSC_A_SHAPE_TRI = 0x21,
    SWITCH_OSC_A_SYNC = 0x22,
    SWITCH_OSC_B_SHAPE_SAW = 0x23,
    SWITCH_OSC_B_SHAPE_TRI = 0x24,
    SWITCH_POLY_MOD_DEST_FREQ_A = 0x25,
    SWITCH_POLY_MOD_DEST_FILTER = 0x26,
    SWITCH_POLY_MOD_UNISON_TRACK = 0x27,

    // rear DIP switches; each reports its weight when on, the sum is the 0-based MIDI channel
    DIP_1 = 0x28, // weight 1
    DIP_2 = 0x29, // weight 2
    DIP_3 = 0x2A, // weight 4
    DIP_4 = 0x2B, // weight 8

    NUM_INDICES = 0x2C // this and everything above answers a failure status
};

inline const std::map<Pro800PanelIndex, std::string> PRO800_PANEL_INDEX_NAMES =
{
    {Pro800PanelIndex::BUTTON_NUMPAD_0,              "Numpad 0"},
    {Pro800PanelIndex::BUTTON_NUMPAD_1,              "Numpad 1"},
    {Pro800PanelIndex::BUTTON_NUMPAD_2,              "Numpad 2"},
    {Pro800PanelIndex::BUTTON_NUMPAD_3,              "Numpad 3"},
    {Pro800PanelIndex::BUTTON_NUMPAD_4,              "Numpad 4"},
    {Pro800PanelIndex::BUTTON_NUMPAD_5,              "Numpad 5"},
    {Pro800PanelIndex::BUTTON_NUMPAD_6,              "Numpad 6"},
    {Pro800PanelIndex::BUTTON_NUMPAD_7,              "Numpad 7"},
    {Pro800PanelIndex::BUTTON_NUMPAD_8,              "Numpad 8"},
    {Pro800PanelIndex::BUTTON_NUMPAD_9,              "Numpad 9"},
    {Pro800PanelIndex::BUTTON_ARP_UP_DOWN,           "Arp Up-Dn"},
    {Pro800PanelIndex::BUTTON_ARP_ASSIGN,            "Arp Assign"},
    {Pro800PanelIndex::BUTTON_PRESET,                "Preset"},
    {Pro800PanelIndex::BUTTON_REC,                   "Rec"},
    {Pro800PanelIndex::BUTTON_PERF,                  "Perf"},
    {Pro800PanelIndex::BUTTON_SETTINGS,              "Settings"},
    {Pro800PanelIndex::BUTTON_SEQ_1,                 "Seq 1"},
    {Pro800PanelIndex::BUTTON_SEQ_2,                 "Seq 2"},
    {Pro800PanelIndex::BUTTON_TUNE,                  "Tune"},
    {Pro800PanelIndex::BUTTON_SYNC_CLOCK,            "Sync Clock"},
    {Pro800PanelIndex::BUTTON_SYNC_SOURCE,           "Sync Source"},
    {Pro800PanelIndex::UNUSED_15,                    "(unused press code 0x15)"},
    {Pro800PanelIndex::VALUE_WHEEL_NEXT,             "Value wheel next"},
    {Pro800PanelIndex::VALUE_WHEEL_PREV,             "Value wheel previous"},
    {Pro800PanelIndex::SWITCH_OSC_A_SHAPE_RECT,      "Osc A Shape Rect"},
    {Pro800PanelIndex::SWITCH_OSC_B_SHAPE_RECT,      "Osc B Shape Rect"},
    {Pro800PanelIndex::SWITCH_FILTER_KEYBOARD_FULL,  "Filter Keyboard Full"},
    {Pro800PanelIndex::SWITCH_FILTER_KEYBOARD_HALF,  "Filter Keyboard Half"},
    {Pro800PanelIndex::SWITCH_LFO_SHAPE,             "LFO Shape (1 = Tri/Sine/Saw)"},
    {Pro800PanelIndex::SWITCH_LFO_DEST_FREQ_AB,      "LFO Dest Freq A-B"},
    {Pro800PanelIndex::SWITCH_LFO_DEST_PW_AB,        "LFO Dest PW A-B"},
    {Pro800PanelIndex::SWITCH_LFO_DEST_FILTER,       "LFO Dest Filter"},
    {Pro800PanelIndex::SWITCH_OSC_A_SHAPE_SAW,       "Osc A Shape Saw"},
    {Pro800PanelIndex::SWITCH_OSC_A_SHAPE_TRI,       "Osc A Shape Tri"},
    {Pro800PanelIndex::SWITCH_OSC_A_SYNC,            "Osc A Sync"},
    {Pro800PanelIndex::SWITCH_OSC_B_SHAPE_SAW,       "Osc B Shape Saw"},
    {Pro800PanelIndex::SWITCH_OSC_B_SHAPE_TRI,       "Osc B Shape Tri"},
    {Pro800PanelIndex::SWITCH_POLY_MOD_DEST_FREQ_A,  "Poly-Mod Dest Freq A"},
    {Pro800PanelIndex::SWITCH_POLY_MOD_DEST_FILTER,  "Poly-Mod Dest Filter"},
    {Pro800PanelIndex::SWITCH_POLY_MOD_UNISON_TRACK, "Poly-Mod Unison Track"},
    {Pro800PanelIndex::DIP_1,                        "DIP switch 1"},
    {Pro800PanelIndex::DIP_2,                        "DIP switch 2"},
    {Pro800PanelIndex::DIP_3,                        "DIP switch 3"},
    {Pro800PanelIndex::DIP_4,                        "DIP switch 4"},
};

/**
 * The 26 live parameters read with 0x72 <index> (answered by 0x73 <index> <value>, 7-bit) and written with
 * 0x73 <index> <value>: the physical knob positions. Like the panel switches they do not follow a preset load;
 * a write reaches the edit buffer (audible at once) but not the stored record. The numbering is unrelated to
 * the knobs' CC numbers.
 */
enum class Pro800LiveIndex : uint8_t
{
    AMP_RELEASE = 0x00,
    AMP_SUSTAIN = 0x01,
    AMP_DECAY = 0x02,
    AMP_ATTACK = 0x03,
    FILTER_RELEASE = 0x04,
    FILTER_SUSTAIN = 0x05,
    FILTER_DECAY = 0x06,
    FILTER_ATTACK = 0x07,
    FILTER_RESONANCE = 0x08,
    FILTER_ENV_AMOUNT = 0x09,
    OSC_A_FREQ = 0x0A,
    OSC_B_FREQ = 0x0B,
    OSC_B_FINE = 0x0C,
    OSC_A_PULSE_WIDTH = 0x0D,
    OSC_B_PULSE_WIDTH = 0x0E,
    OSC_A_LEVEL = 0x0F,
    GLIDE_TIME = 0x10,
    LFO_FREQ = 0x11,
    LFO_INITIAL_AMOUNT = 0x12, // the one index whose read-back does not match a write (a lossy curve)
    MASTER_TUNE = 0x13,
    MASTER_VOLUME = 0x14,
    OSC_B_LEVEL = 0x15,
    POLY_MOD_SOURCE_OSC_B = 0x16,
    POLY_MOD_SOURCE_FILTER_ENV = 0x17,
    NOISE_LEVEL = 0x18,
    FILTER_CUTOFF = 0x19,

    NUM_INDICES = 0x1A // this and everything above answers a failure status
};

inline const std::map<Pro800LiveIndex, std::string> PRO800_LIVE_INDEX_NAMES =
{
    {Pro800LiveIndex::AMP_RELEASE,                "Amp Release"},
    {Pro800LiveIndex::AMP_SUSTAIN,                "Amp Sustain"},
    {Pro800LiveIndex::AMP_DECAY,                  "Amp Decay"},
    {Pro800LiveIndex::AMP_ATTACK,                 "Amp Attack"},
    {Pro800LiveIndex::FILTER_RELEASE,             "Filter Release"},
    {Pro800LiveIndex::FILTER_SUSTAIN,             "Filter Sustain"},
    {Pro800LiveIndex::FILTER_DECAY,               "Filter Decay"},
    {Pro800LiveIndex::FILTER_ATTACK,              "Filter Attack"},
    {Pro800LiveIndex::FILTER_RESONANCE,           "Filter Resonance"},
    {Pro800LiveIndex::FILTER_ENV_AMOUNT,          "Filter Env Amount"},
    {Pro800LiveIndex::OSC_A_FREQ,                 "Osc A Frequency"},
    {Pro800LiveIndex::OSC_B_FREQ,                 "Osc B Frequency"},
    {Pro800LiveIndex::OSC_B_FINE,                 "Osc B Fine"},
    {Pro800LiveIndex::OSC_A_PULSE_WIDTH,          "Osc A Pulse Width"},
    {Pro800LiveIndex::OSC_B_PULSE_WIDTH,          "Osc B Pulse Width"},
    {Pro800LiveIndex::OSC_A_LEVEL,                "Osc A Level"},
    {Pro800LiveIndex::GLIDE_TIME,                 "Glide Time"},
    {Pro800LiveIndex::LFO_FREQ,                   "LFO Mod Freq"},
    {Pro800LiveIndex::LFO_INITIAL_AMOUNT,         "LFO Mod Initial Amount"},
    {Pro800LiveIndex::MASTER_TUNE,                "Master Tune"},
    {Pro800LiveIndex::MASTER_VOLUME,              "Master Volume"},
    {Pro800LiveIndex::OSC_B_LEVEL,                "Osc B Level"},
    {Pro800LiveIndex::POLY_MOD_SOURCE_OSC_B,      "Poly-Mod Source Osc B"},
    {Pro800LiveIndex::POLY_MOD_SOURCE_FILTER_ENV, "Poly-Mod Source Filter Env"},
    {Pro800LiveIndex::NOISE_LEVEL,                "Noise Level"},
    {Pro800LiveIndex::FILTER_CUTOFF,              "Filter Cutoff"},
};

/** The four DIP switches and the weight each one reports when it is on. */
inline const std::map<Pro800PanelIndex, int> PRO800_PANEL_DIP_WEIGHTS =
{
    {Pro800PanelIndex::DIP_1, 1},
    {Pro800PanelIndex::DIP_2, 2},
    {Pro800PanelIndex::DIP_3, 4},
    {Pro800PanelIndex::DIP_4, 8},
};
// clang-format on
