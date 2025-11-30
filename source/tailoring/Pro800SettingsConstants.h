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

enum Pro800Settings
{
    SETTINGS_FIELD_NONE = -1, // placeholder for no field

    SETTINGS_PRESET_NUM,
    SETTINGS_PRESET_MODE,
    SETTINGS_MIDI_RX_CHANNEL,
    SETTINGS_VOICE_KILL,
    SETTINGS_MIDI_TX_CHANNEL,
    SETTINGS_SYNC_SOURCE,
    SETTINGS_SYNC_CLOCK_BPM,
    SETTINGS_BRIGHTNESS,
    SETTINGS_DISPLAY_PARAMETER_TIME,
    SETTINGS_MIDI_CC_MODE,
    SETTINGS_MIDI_PC_MODE,
    SETTINGS_SYNC_IN_FORWARD,
    SETTINGS_EXTERNAL_CV_AMOUNT,
    SETTINGS_SYNC_CLOCK_SUBDIVISION,
    SETTINGS_VOICE_PRIORITY,
    SETTINGS_SHOW_PRESET_NAME,
    SETTINGS_SYNC_IN_POLARITY,
    SETTINGS_TUNER_PRECISION,
    SETTINGS_SYNC_IN_START_STOP,
    SETTINGS_SYNC_IN_PPQN,
    SETTINGS_SYNC_CLOCK_NOTE_LENGTH,
    SETTINGS_SYNC_CLOCK_SWING,
    SETTINGS_AFTERTOUCH_VCA_POLARITY,
    SETTINGS_AFTERTOUCH_VCF_POLARITY,
    SETTINGS_TRANSPOSE,
    SETTINGS_LOCAL_ENABLE,
    SETTINGS_SOFT_THRU,
};

const std::map<Pro800Settings, Pro800Parameter> PRO800_SETTINGS_FIELDS =
{
    {SETTINGS_PRESET_NUM,              {6, 2, "Preset Number"}},
    // 8 = overflow
    {SETTINGS_PRESET_MODE,             {9, 1, "Preset Mode"}}, // see: Pro800SettingsPresetMode
    {SETTINGS_MIDI_RX_CHANNEL,         {10, 1, "MIDI RX Channel"}}, // see: Pro800SettingsMidiReceiveChannel
    {SETTINGS_VOICE_KILL,              {11, 1, "Voice Kill"}}, // voices 1-7, bitwise // 14
    {SETTINGS_MIDI_TX_CHANNEL,         {12, 1, "MIDI TC Channel"}}, // see: Pro800SettingsMidiTransmitChannel
    {SETTINGS_SYNC_SOURCE,             {13, 1, "Sync Source"}}, // see: Pro800SettingsSyncSource
    // 14 = unknown
    {SETTINGS_SYNC_CLOCK_BPM,          {15, 2, "Sync Clock BPM"}}, 
    // 16 = overflow
    // 18 = unknown
    {SETTINGS_BRIGHTNESS,              {19, 1, "Display Brightness"}}, // 0-16
    {SETTINGS_DISPLAY_PARAMETER_TIME,  {20, 1, "Display Parameter Time"}}, // 0-100
    {SETTINGS_MIDI_CC_MODE,            {21, 1, "MIDI CC Mode"}}, // see: Pro800SettingsMidiMode
    {SETTINGS_MIDI_PC_MODE,            {22, 1, "MIDI PC Mode"}}, // see: Pro800SettingsMidiMode
    // 23 = unknown
    // 24 = overflow
    {SETTINGS_SYNC_IN_FORWARD,         {25, 1, "Sync In Forward Enable"}}, // see: Pro800SettingsOnOff
    {SETTINGS_EXTERNAL_CV_AMOUNT,      {26, 2, "External CV Amount"}}, 
    // 28 = unknown
    {SETTINGS_SYNC_CLOCK_SUBDIVISION,  {29, 1, "Sync Clock Subdivision"}}, // see: Pro800SettingsSyncSubdivision
    {SETTINGS_VOICE_PRIORITY,          {30, 1, "Voice Priority"}}, //see: Pro800SettingsVoicePriority
    {SETTINGS_SHOW_PRESET_NAME,        {31, 1, "Show Preset Name"}}, // see: Pro800SettingsOnOff
    // 32 = overflow
    {SETTINGS_SYNC_IN_POLARITY,        {33, 1, "Sync In Polarity"}}, // see: Pro800SettingsPolarity
    // 34 = unknown
    {SETTINGS_TUNER_PRECISION,         {35, 1, "Tuner Precision"}}, // see: Pro800SettingsTunerPrecision
    {SETTINGS_SYNC_IN_START_STOP,      {36, 1, "Sync In Start-Stop"}}, // see: Pro800SettingsOnOff
    {SETTINGS_SYNC_IN_PPQN,            {37, 1, "Sync In PPQN"}}, // see: Pro800SettingsSyncInPPQN
    {SETTINGS_SYNC_CLOCK_NOTE_LENGTH,  {38, 1, "Sync Clock Note Length"}}, // 5-100
    {SETTINGS_SYNC_CLOCK_SWING,        {39, 1, "Sync Clock Swing"}}, // 5-95
    // 40 = overflow
    {SETTINGS_AFTERTOUCH_VCA_POLARITY, {41, 1, "Aftertouch VCA Polarity"}}, // see: Pro800SettingsPolarity
    {SETTINGS_AFTERTOUCH_VCF_POLARITY, {42, 1, "Aftertouch VCF Polarity"}}, // see: Pro800SettingsPolarity
    {SETTINGS_TRANSPOSE,               {43, 1, "Transpose", 0, true}}, // (-12 - +12; -1 = 0x7f, +1 = 0x01)
    {SETTINGS_LOCAL_ENABLE,            {44, 1, "Local Enable"}}, // see: Pro800SettingsOnOff
    {SETTINGS_SOFT_THRU,               {45, 1, "Soft Thru"}}, // see: Pro800SettingsOnOff
};

enum Pro800SettingsMidiReceiveChannel
{
    SETTINGS_MIDI_RX_ALL = 0,
    SETTINGS_MIDI_RX_DIPS = 1,
    SETTINGS_MIDI_RX_1 = 2,
    SETTINGS_MIDI_RX_2 = 3,
    SETTINGS_MIDI_RX_3 = 4,
    SETTINGS_MIDI_RX_4 = 5,
    SETTINGS_MIDI_RX_5 = 6,
    SETTINGS_MIDI_RX_6 = 7,
    SETTINGS_MIDI_RX_7 = 8,
    SETTINGS_MIDI_RX_8 = 9,
    SETTINGS_MIDI_RX_9 = 10,
    SETTINGS_MIDI_RX_10 = 11,
    SETTINGS_MIDI_RX_11 = 12,
    SETTINGS_MIDI_RX_12 = 13,
    SETTINGS_MIDI_RX_13 = 14,
    SETTINGS_MIDI_RX_14 = 15,
    SETTINGS_MIDI_RX_15 = 16,
    SETTINGS_MIDI_RX_16 = 17,
    SETTINGS_MIDI_RX_OFF = 18
};

enum Pro800SettingsMidiTransmitChannel
{
    SETTINGS_MIDI_TX_THRU = 0,
    SETTINGS_MIDI_TX_DIPS = 1,
    SETTINGS_MIDI_TX_1 = 2,
    SETTINGS_MIDI_TX_2 = 3,
    SETTINGS_MIDI_TX_3 = 4,
    SETTINGS_MIDI_TX_4 = 5,
    SETTINGS_MIDI_TX_5 = 6,
    SETTINGS_MIDI_TX_6 = 7,
    SETTINGS_MIDI_TX_7 = 8,
    SETTINGS_MIDI_TX_8 = 9,
    SETTINGS_MIDI_TX_9 = 10,
    SETTINGS_MIDI_TX_10 = 11,
    SETTINGS_MIDI_TX_11 = 12,
    SETTINGS_MIDI_TX_12 = 13,
    SETTINGS_MIDI_TX_13 = 14,
    SETTINGS_MIDI_TX_14 = 15,
    SETTINGS_MIDI_TX_15 = 16,
    SETTINGS_MIDI_TX_16 = 17
};

enum Pro800SettingsMidiMode
{
    SETTINGS_MIDI_MODE_OFF = 0,
    SETTINGS_MIDI_MODE_TX = 1,
    SETTINGS_MIDI_MODE_RX = 2,
    SETTINGS_MIDI_MODE_TX_RX = 3
};

enum Pro800SettingsSyncSource
{
    SETTINGS_SYNC_SOURCE_INTERNAL = 0,
    SETTINGS_SYNC_SOURCE_MIDI = 1,
    SETTINGS_SYNC_SOURCE_USB = 2,
    SETTINGS_SYNC_SOURCE_EXTERNAL = 3
};

enum Pro800SettingsOnOff
{
    SETTINGS_OFF = 0,
    SETTINGS_ON = 1
};

enum Pro800SettingsPolarity
{
    SETTINGS_POLARITY_RISE = 0,
    SETTINGS_POLARITY_FALL = 1,
    SETTINGS_POLARITY_BOTH = 2
};

enum Pro800SettingsSyncInPPQN
{
    SETTINGS_SYNC_IN_1PPS = 0,
    SETTINGS_SYNC_IN_1PPQN = 1,
    SETTINGS_SYNC_IN_2PPQN = 2,
    SETTINGS_SYNC_IN_4PPQN = 3,
    SETTINGS_SYNC_IN_24PPQN = 4,
    SETTINGS_SYNC_IN_48PPQN = 5
};

enum Pro800SettingsSyncClockSubdivision
{
    SETTINGS_SYNC_CLOCK_SUBDIVISION_1_4 = 0,
    SETTINGS_SYNC_CLOCK_SUBDIVISION_1_4T = 1,
    SETTINGS_SYNC_CLOCK_SUBDIVISION_1_8 = 2,
    SETTINGS_SYNC_CLOCK_SUBDIVISION_1_8T = 3,
    SETTINGS_SYNC_CLOCK_SUBDIVISION_1_16 = 4,
    SETTINGS_SYNC_CLOCK_SUBDIVISION_1_16T = 5,
    SETTINGS_SYNC_CLOCK_SUBDIVISION_1_32 = 6,
    SETTINGS_SYNC_CLOCK_SUBDIVISION_1_32T = 7
};

enum Pro800SettingsTunerPrecision
{
    SETTINGS_TUNER_PRECISION_0_5CT = 0,
    SETTINGS_TUNER_PRECISION_1CT = 1,
    SETTINGS_TUNER_PRECISION_1_5CT = 2,
    SETTINGS_TUNER_PRECISION_2CT = 3
};

enum Pro800SettingsVoicePriority
{
    SETTINGS_VOICE_PRIORITY_LAST = 0,
    SETTINGS_VOICE_PRIORITY_LOW = 1,
    SETTINGS_VOICE_PRIORITY_HIGH = 2
};

enum Pro800SettingsPresetMode
{
    SETTINGS_PRESET_MODE_MANUAL = 0,
    SETTINGS_PRESET_MODE_LOADED = 1,
    SETTINGS_PRESET_MODE_EDITED = 2
};