#pragma once

enum MessageType {
    MIDI_CC_MESSAGE,
    MIDI_LOG_MESSAGE,
    PRO800_UNKNOWN_MESSAGE,
    PRO800_SETTINGS_MESSAGE,
    PRO800_VERSION_MESSAGE,
    PRO800_STATUS_MESSAGE,
    PRO800_PROGRAM_MESSAGE
};

enum Pro800FilterKeyboardTracking
{
    TRACKING_OFF = 0,
    TRACKING_HALF = 43,
    TRACKING_FULL = 86
};

enum Pro800LFOShape
{
    LFO_SHAPE_TRIANGLE = 22,
    LFO_SHAPE_SINE = 66,
    LFO_SHAPE_SAW = 110,

    LFO_SHAPE_PULSE = 0,
    LFO_SHAPE_RANDOM = 44,
    LFO_SHAPE_NOISE = 88,
};

enum Pro800LFOSpeed
{
    LFO_SPEED_SLOW = 0,
    LFO_SPEED_FAST = 64
};

enum Pro800EnvelopeSpeed
{
    ENV_SPEED_FAST = 0,
    ENV_SPEED_SLOW = 64,
};

enum Pro800EnvelopeShape
{
    ENV_SHAPE_LINEAR = 0,
    ENV_SHAPE_EXPONENTIAL = 64
};

enum Pro800LFOTarget
{
    LFO_TARGET_OSC_AB = 0,
    LFO_TARGET_OSC_A = 33,
    LFO_TARGET_OSC_B = 66,
    LFO_TARGET_VCA = 99
};

enum Pro800ModWheelAmount
{
    MOD_WHEEL_AMOUNT_MIN = 0,
    MOD_WHEEL_AMOUNT_LOW = 32,
    MOD_WHEEL_AMOUNT_HIGH = 64,
    MOD_WHEEL_AMOUNT_FULL = 96
};

enum Pro800ModWheelTarget
{
    MOD_WHEEL_TARGET_LFO = 0,
    MOD_WHEEL_TARGET_VIBRATO = 64
};

enum Pro800PitchBendTarget
{
    PITCH_BEND_TARGET_OFF = 0,
    PITCH_BEND_TARGET_VCO = 32,
    PITCH_BEND_TARGET_VCF = 64,
    PITCH_BEND_TARGET_VOLUME = 96
};

enum Pro800FreqPotMode
{
    FREQ_POT_MODE_FREE = 0,
    FREQ_POT_MODE_SEMI = 32,
    FREQ_POT_MODE_OCT = 64,
    FREQ_POT_MODE_FIXED = 96
};

enum Pro800KeyboardTracking
{
    KEYBOARD_TRACKING_C1 = 0,
    KEYBOARD_TRACKING_C2 = 32,
    KEYBOARD_TRACKING_C3 = 64,
    KEYBOARD_TRACKING_C4 = 96
};

enum Pro800OnOff
{
    OFF = 0,
    ON = 64
};

enum Pro800GlideMode
{
    GLIDE_MODE_TIME = 0,
    GLIDE_MODE_SPEED = 64
};

enum Pro800CCMessages
{
    // Front Panel
    OSC_A_FREQ = 8,
    OSC_A_LEVEL = 9,
    OSC_A_PULSE_WIDTH = 10,
    OSC_A_SYNC = 54,
    OSC_A_SHAPE_SAW = 48,
    OSC_A_SHAPE_TRI = 49,
    OSC_A_SHAPE_RECT = 50,

    OSC_B_FREQ = 11,
    OSC_B_FINE = 14,
    OSC_B_PULSE_WIDTH = 13,
    OSC_B_LEVEL = 12,
    OSC_B_SHAPE_SAW = 51,
    OSC_B_SHAPE_TRI = 52,
    OSC_B_SHAPE_RECT = 53,

    POLY_MOD_SOURCE_FILTER_ENV = 26,
    POLY_MOD_SOURCE_OSC_B = 27,
    POLY_MOD_DEST_FREQ_A = 55,
    POLY_MOD_DEST_FILTER = 56,
    POLY_MOD_UNISON_TRACK = 65,

    NOISE_LEVEL = 37,

    LFO_MOD_FREQ = 28,
    LFO_MOD_SHAPE = 57, // see: Pro800LfoShape
    LFO_MOD_INITIAL_AMOUNT = 29,
    LFO_MOD_DEST_FREQ_AB = 74,
    LFO_MOD_DEST_PW_AB = 76,
    LFO_MOD_DEST_FILTER = 75,

    GLIDE_TIME = 30,

    FILTER_CUTOFF = 15,
    FILTER_RESONANCE = 16,
    FILTER_ENV_AMOUNT = 17,
    FILTER_ATTACK = 21,
    FILTER_DECAY = 20,
    FILTER_SUSTAIN = 19,
    FILTER_RELEASE = 18,
    FILTER_KEYBOARD_TRACKING = 60, // see: Pro800FilterKeyboardTracking
    
    AMP_ATTACK = 25,
    AMP_DECAY = 24,
    AMP_SUSTAIN = 23,
    AMP_RELEASE = 22,

    MASTER_TUNE = 3,
    MASTER_VOLUME = 7,

    // Menu: Performance 1
    LFO_TARGET = 59, // see: Pro800LFOTarget
    LFO_SPEED = 58, // see: Pro800Speed
    
    // Menu: Performance 2
    VIBRATO_AMOUNT = 35,
    VIBRATO_SPEED = 34,

    // Menu: Performance 3
    MOD_WHEEL_AMOUNT = 67, // see: Pro800ModWheelAmount
    MOD_WHEEL_TARGET = 70, // see: Pro800ModWheelTarget
    MODULATION_DELAY = 33,

    // Menu: Performance 4
    VCA_ENV_SHAPE = 63, // see: Pro800EnvelopeShape
    VCA_ENV_SPEED = 72, // see: Pro800Speed

    VCF_ENV_SHAPE = 61, // see: Pro800EnvelopeShape
    VCF_ENV_SPEED = 62, // see: Pro800Speed

    // Menu: Performance 5
    PITCH_BEND_TARGET = 66, // see: Pro800PitchBendTarget
    PITCH_BEND_RANGE = 42,  // 0-31 semitones (0-3 = 0, 4-7 = 1, 8-11 = 2, ..., 124-127 = 31)

    // Menu: Performance 6
    OSC_A_FREQ_POT_MODE = 68, // see: Pro800FreqPotMode
    OSC_B_FREQ_POT_MODE = 69, // see: Pro800FreqPotMode
    KEYBOARD_TRACKING = 78, // see: Pro800KeyboardTracking

    // Menu: Performance 7
    VCA_VELOCITY_AMOUNT = 31,
    VCF_VELOCITY_AMOUNT = 32,

    // Menu: Performance 8
    VCA_AFTERTOUCH_AMOUNT = 39,
    VCF_AFTERTOUCH_AMOUNT = 40,
    LFO_AFTERTOUCH_AMOUNT = 41,

    // Menu: Performance 9
    UNISON_SPREAD_DETUNE = 36,
    VOICE_SPREAD_ENABLE = 77, // see: Pro800OnOff

    // Menu: Performance 0
    GLIDE_MODE = 79 // see: Pro800GlideMode
};
                                                                                                                                                                  
enum Pro800Settings
{
    SETTINGS_ADDRESS_LOW = 0, // offset from Pro800MidiMessage::POS_MESSAGE_START
    SETTINGS_ADDRESS_HIGH = 1,
    SETTINGS_PRESET_OVERFLOW = 2, // bit 5
    SETTINGS_UNKNOWN_0 = 2, // other overflow bits?
    SETTINGS_UNKNOWN_1 = 3,
    SETTINGS_UNKNOWN_2 = 4,
    SETTINGS_UNKNOWN_3 = 5,
    SETTINGS_UNKNOWN_4 = 6,
    SETTINGS_UNKNOWN_5 = 7,
    SETTINGS_PRESET_LSB = 8,
    SETTINGS_PRESET_MSB = 9,
    SETTINGS_OVERFLOW_BPM_VOICE8 = 10, // overflow for bpm and voice 8 bit 8
    SETTINGS_PRESET_MODE = 11, // see: Pro800PresetMode
    SETTINGS_MIDI_RX_CHANNEL = 12, // see: Pro800MidiReceiveChannel
    SETTINGS_VOICE_KILL = 13, // voices 1-7, bitwise // 14
    SETTINGS_MIDI_TX_CHANNEL = 14, // see: Pro800MidiTransmitChannel
    SETTINGS_SYNC_SOURCE = 15, // see: Pro800SyncSource
    SETTINGS_UNKNOWN_7 = 16,
    SETTINGS_SYNC_CLOCK_BPM_LSB = 17,
    SETTINGS_UNKNOWN_8 = 18,
    SETTINGS_SYNC_CLOCK_BPM_MSB = 19,
    SETTINGS_UNKNOWN_9 = 20,
    SETTINGS_BRIGHTNESS = 21, // 0-16
    SETTINGS_DISPLAY_PARAMETER_TIME = 22, // 0-100
    SETTINGS_MIDI_CC_MODE = 23, // see: Pro800MidiMode
    SETTINGS_MIDI_PC_MODE = 24, // see: Pro800MidiMode
    SETTINGS_UNKNOWN_11 = 25,
    SETTINGS_EXTERNAL_CV_AMOUNT_OVERFLOW = 26, // 0-65535
    SETTINGS_SYNC_IN_FORWARD = 27, // see: Pro800SettingsOnOff
    SETTINGS_EXTERNAL_CV_AMOUNT_LSB = 28,
    SETTINGS_EXTERNAL_CV_AMOUNT_MSB = 29,
    SETTINGS_UNKNOWN_12 = 30,
    SETTINGS_SYNC_CLOCK_SUBDIVISION = 31, // see: Pro800SyncSubdivision
    SETTINGS_VOICE_PRIORITY = 32, //see: Pro800VoicePriority
    SETTINGS_SHOW_PRESET_NAME = 33, // see: Pro800SettingsOnOff
    SETTINGS_UNKNOWN_13 = 34,
    SETTINGS_SYNC_IN_POLARITY = 35, // see: Pro800SettingsPolarity
    SETTINGS_UNKNOWN_14 = 36,
    SETTINGS_TUNER_PRECISION = 37, // see: Pro800TunerPrecision
    SETTINGS_SYNC_IN_START_STOP = 38, // see: Pro800SettingsOnOff
    SETTINGS_SYNC_IN_PPQN = 39, // see: Pro800SettingsSyncInPPQN
    SETTINGS_SYNC_CLOCK_NOTE_LENGTH = 40, // 5-100
    SETTINGS_SYNC_CLOCK_SWING = 41, // 5-95
    SETTINGS_TRANSPOSE_OVERFLOW = 42, // 0 = positive, 4 = negative
    SETTINGS_AFTERTOUCH_VCA_POLARITY = 43, // see: Pro800SettingsPolarity
    SETTINGS_AFTERTOUCH_VCF_POLARITY = 44, // see: Pro800SettingsPolarity
    SETTINGS_TRANSPOSE = 45, // (-12 - +12; -1 = 0x7f, +1 = 0x01)
    SETTINGS_LOCAL_ENABLE = 46, // see: Pro800SettingsOnOff
    SETTINGS_SOFT_THRU = 47, // see: Pro800SettingsOnOff
    SETTINGS_LENGTH=48,

    SETTINGS_EXTERNAL_CV_AMOUNT = 100, // special case for handling EXTERNAL_CV_AMOUNT_LSB, EXTERNAL_CV_AMOUNT_MSB and EXTERNAL_CV_AMOUNT_OVERFLOW together
    SETTINGS_SYNC_CLOCK_BPM = 101,     // special case for handling SYNC_CLOCK_BPM_LSB and SYNC_CLOCK_BPM_MSB together
    SETTINGS_PRESET_NUM = 102,         // special case for handling PRESET_LSB, PRESET_MSB and PRESET_OVERFLOW together
    SETTINGS_NONE = -1
};

enum Pro800MidiReceiveChannel
{
    MIDI_RX_ALL = 0,
    MIDI_RX_DIPS = 1,
    MIDI_RX_1 = 2,
    MIDI_RX_2 = 3,
    MIDI_RX_3 = 4,
    MIDI_RX_4 = 5,
    MIDI_RX_5 = 6,
    MIDI_RX_6 = 7,
    MIDI_RX_7 = 8,
    MIDI_RX_8 = 9,
    MIDI_RX_9 = 10,
    MIDI_RX_10 = 11,
    MIDI_RX_11 = 12,
    MIDI_RX_12 = 13,
    MIDI_RX_13 = 14,
    MIDI_RX_14 = 15,
    MIDI_RX_15 = 16,
    MIDI_RX_16 = 17,
    MIDI_RX_OFF = 18
};

enum Pro800MidiTransmitChannel
{
    MIDI_TX_THRU = 0,
    MIDI_TX_DIPS = 1,
    MIDI_TX_1 = 2,
    MIDI_TX_2 = 3,
    MIDI_TX_3 = 4,
    MIDI_TX_4 = 5,
    MIDI_TX_5 = 6,
    MIDI_TX_6 = 7,
    MIDI_TX_7 = 8,
    MIDI_TX_8 = 9,
    MIDI_TX_9 = 10,
    MIDI_TX_10 = 11,
    MIDI_TX_11 = 12,
    MIDI_TX_12 = 13,
    MIDI_TX_13 = 14,
    MIDI_TX_14 = 15,
    MIDI_TX_15 = 16,
    MIDI_TX_16 = 17
};

enum Pro800MidiMode
{
    MIDI_MODE_OFF = 0,
    MIDI_MODE_TX = 1,
    MIDI_MODE_RX = 2,
    MIDI_MODE_TX_RX = 3
};

enum Pro800SyncSource
{
    SYNC_SOURCE_INTERNAL = 0,
    SYNC_SOURCE_MIDI = 1,
    SYNC_SOURCE_USB = 2,
    SYNC_SOURCE_EXTERNAL = 3
};

enum Pro800SettingsOnOff
{
    SETTINGS_OFF = 0,
    SETTINGS_ON = 1
};

enum Pro800SettingsPolarity
{
    POLARITY_RISE = 0,
    POLARITY_FALL = 1,
    POLARITY_BOTH = 2
};

enum Pro800SettingsSyncInPPQN
{
    SYNC_IN_1PPS = 0,
    SYNC_IN_1PPQN = 1,
    SYNC_IN_2PPQN = 2,
    SYNC_IN_4PPQN = 3,
    SYNC_IN_24PPQN = 4,
    SYNC_IN_48PPQN = 5
};

enum Pro800SyncClockSubdivision
{
    SYNC_CLOCK_SUBDIVISION_1_4 = 0,
    SYNC_CLOCK_SUBDIVISION_1_4T = 1,
    SYNC_CLOCK_SUBDIVISION_1_8 = 2,
    SYNC_CLOCK_SUBDIVISION_1_8T = 3,
    SYNC_CLOCK_SUBDIVISION_1_16 = 4,
    SYNC_CLOCK_SUBDIVISION_1_16T = 5,
    SYNC_CLOCK_SUBDIVISION_1_32 = 6,
    SYNC_CLOCK_SUBDIVISION_1_32T = 7
};

enum Pro800TunerPrecision
{
    TUNER_PRECISION_0_5CT = 0,
    TUNER_PRECISION_1CT = 1,
    TUNER_PRECISION_1_5CT = 2,
    TUNER_PRECISION_2CT = 3
};

enum Pro800VoicePriority
{
    VOICE_PRIORITY_LAST = 0,
    VOICE_PRIORITY_LOW = 1,
    VOICE_PRIORITY_HIGH = 2
};

enum Pro800TransposeSign
{
    TRANSPOSE_POSITIVE = 0,
    TRANSPOSE_NEGATIVE = 4
};

enum Pro800OverflowBitPosition
{
    OVERFLOW_BPM_BIT8 = 6,
    OVERFLOW_VOICE8_BIT8 = 2,
    OVERFLOW_EXTERNAL_CV_AMOUNT_BIT8 = 1,
    OVERFLOW_EXTERNAL_CV_AMOUNT_BIT16 = 2,
    OVERFLOW_TRANSPOSE_BIT8 = 2,
    OVERFLOW_PRESET_BIT8 = 5,
    OVERFLOW_NONE = -1
};

enum Pro800PresetMode
{
    PRESET_MODE_MANUAL = 0,
    PRESET_MODE_LOADED = 1,
    PRESET_MODE_EDITED = 2
};



enum Pro800Program
{
    PROGRAM_NUM_LSB = 0, // offset from Pro800MidiMessage::POS_MESSAGE_START
    PROGRAM_NUM_MSB = 1,
    
    PROGRAM_NAME_FIRST_CHAR = 174,
    PROGRAM_NAME_LAST_CHAR   = PROGRAM_NAME_FIRST_CHAR + 16
};

enum Pro800ProgramField
{
    PROGRAM_FIELD_NUM = 0,
    PROGRAM_FIELD_VERSION,

    PROGRAM_FIELD_OSC_A_FREQ,
    PROGRAM_FIELD_OSC_A_LEVEL,
    PROGRAM_FIELD_OSC_A_PULSE_WIDTH,

    PROGRAM_FIELD_OSC_B_FREQ,
    PROGRAM_FIELD_OSC_B_LEVEL,
    PROGRAM_FIELD_OSC_B_PULSE_WIDTH,
    PROGRAM_FIELD_OSC_B_FINE_FREQ,

    PROGRAM_FIELD_FILTER_CUTOFF,
    PROGRAM_FIELD_FILTER_RESONANCE,
    PROGRAM_FIELD_FILTER_ENV_AMOUNT,
    PROGRAM_FIELD_FILTER_RELEASE,
    PROGRAM_FIELD_FILTER_SUSTAIN,
    PROGRAM_FIELD_FILTER_DECAY,
    PROGRAM_FIELD_FILTER_ATTACK,

    PROGRAM_FIELD_AMP_RELEASE,
    PROGRAM_FIELD_AMP_SUSTAIN,
    PROGRAM_FIELD_AMP_DECAY,
    PROGRAM_FIELD_AMP_ATTACK,

    PROGRAM_FIELD_POLYMOD_FILTER_ENV,
    PROGRAM_FIELD_POLYMOD_OSC_B,

    PROGRAM_FIELD_LFO_FREQ,
    PROGRAM_FIELD_LFO_AMOUNT,

    PROGRAM_FIELD_GLIDE,

    PROGRAM_FIELD_NOISE,

    PROGRAM_FIELD_NAME
};

struct Pro800Parameter
{    
    std::vector<uint16_t> dataBytes; // lsb, msb
    std::vector<uint16_t> overflowBytes;
    std::vector<uint8_t> overflowBits;
    std::string name;
};

const std::map<Pro800ProgramField, Pro800Parameter> PRO800_PROGRAM_FIELDS =
{
    // field, {lsb, msb, {overflow 1, overflow 2}, {bit8, bit16}}}
    {PROGRAM_FIELD_VERSION,            {{ 7 },  {}, {}, "Version"}},

    {PROGRAM_FIELD_OSC_A_FREQ,         {{  8,   9},  {  2,   2}, {5, 6}, "Osc A Frequency"}},
    {PROGRAM_FIELD_OSC_A_LEVEL,        {{ 11,  12},  { 10,  10}, {0, 1}, "Osc A Level"}},
    {PROGRAM_FIELD_OSC_A_PULSE_WIDTH,  {{ 13,  14},  { 10,  10}, {2, 3}, "Osc A Pulse Width"}},

    {PROGRAM_FIELD_OSC_B_FREQ,         {{ 15,  16},  { 10,  10}, {4, 5}, "Osc B Frequency"}},
    {PROGRAM_FIELD_OSC_B_LEVEL,        {{ 17,  19},  { 18,  10}, {0, 6}, "Osc B Level"}},
    {PROGRAM_FIELD_OSC_B_PULSE_WIDTH,  {{ 20,  21},  { 18,  18}, {1, 2}, "Osc B Pulse Width"}},
    {PROGRAM_FIELD_OSC_B_FINE_FREQ,    {{ 22,  23},  { 18,  18}, {3, 4}, "Osc B Fine Frequency"}},

    {PROGRAM_FIELD_FILTER_CUTOFF,      {{ 24,  25},  { 18,  18}, {5, 6}, "Filter Cutoff"}},
    {PROGRAM_FIELD_FILTER_RESONANCE,   {{ 27,  28},  { 26,  26}, {0, 1}, "Filter Resonance"}},
    {PROGRAM_FIELD_FILTER_ENV_AMOUNT,  {{ 29,  30},  { 26,  26}, {2, 3}, "Filter Envelope Amount"}},
    {PROGRAM_FIELD_FILTER_RELEASE,     {{ 31,  32},  { 26,  26}, {4, 5}, "Filter Release"}},
    {PROGRAM_FIELD_FILTER_SUSTAIN,     {{ 33,  35},  { 34,  26}, {0, 6}, "Filter Sustain"}},
    {PROGRAM_FIELD_FILTER_DECAY,       {{ 36,  37},  { 34,  34}, {1, 2}, "Filter Decay"}},
    {PROGRAM_FIELD_FILTER_ATTACK,      {{ 38,  39},  { 34,  34}, {3, 4}, "Filter Attack"}},

    {PROGRAM_FIELD_AMP_RELEASE,        {{ 40,  41},  { 34,  34}, {5, 6}, "Amp Release"}},
    {PROGRAM_FIELD_AMP_SUSTAIN,        {{ 43,  44},  { 42,  42}, {0, 1}, "Amp Sustain"}},
    {PROGRAM_FIELD_AMP_DECAY,          {{ 45,  46},  { 42,  42}, {2, 3}, "Amp Decay"}},
    {PROGRAM_FIELD_AMP_ATTACK,         {{ 47,  48},  { 42,  42}, {4, 5}, "Amp Attack"}},

    {PROGRAM_FIELD_POLYMOD_FILTER_ENV, {{ 49,  51},  { 42,  50}, {6, 0}, "Poly-Mod Filter Env"}},
    {PROGRAM_FIELD_POLYMOD_OSC_B,      {{ 52,  53},  { 50,  50}, {1, 2}, "Poly-Mod Osc B"}},

    {PROGRAM_FIELD_LFO_FREQ,           {{ 54,  55},  { 50,  50}, {3, 4}, "LFO Frequency"}},
    {PROGRAM_FIELD_LFO_AMOUNT,         {{ 56,  57},  { 50,  50}, {5, 6}, "LFO Amount"}},

    {PROGRAM_FIELD_GLIDE,              {{ 59,  60},  { 58,  58}, {0, 1}, "Glide Amount"}},

    {PROGRAM_FIELD_NOISE,              {{165, 166},  {162, 162}, {2, 3}, "Noise Amount"}}
   
};