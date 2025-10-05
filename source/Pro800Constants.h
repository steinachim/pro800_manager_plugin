#pragma once

struct Pro800Parameter
{    
    uint16_t firstByte;
    uint8_t numBytes;
    std::string name;
    bool isSigned = false;
};

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
    {SETTINGS_PRESET_NUM,              {8, 2, "Preset Number"}},

    {SETTINGS_PRESET_MODE,             {11, 1, "Preset Mode"}}, // see: Pro800PresetMode
    {SETTINGS_MIDI_RX_CHANNEL,         {12, 1, "MIDI RX Channel"}}, // see: Pro800MidiReceiveChannel
    {SETTINGS_VOICE_KILL,              {13, 1, "Voice Kill"}}, // voices 1-7, bitwise // 14
    {SETTINGS_MIDI_TX_CHANNEL,         {14, 1, "MIDI TC Channel"}}, // see: Pro800MidiTransmitChannel
    {SETTINGS_SYNC_SOURCE,             {15, 1, "Sync Source"}}, // see: Pro800SyncSource
    // 16 = unknown
    {SETTINGS_SYNC_CLOCK_BPM,          {17, 2, "Sync Clock BPM"}}, 
    // 18 = overflow
    // 20 = unknown
    {SETTINGS_BRIGHTNESS,              {21, 1, "Display Brightness"}}, // 0-16
    {SETTINGS_DISPLAY_PARAMETER_TIME,  {22, 1, "Display Parameter Time"}}, // 0-100
    {SETTINGS_MIDI_CC_MODE,            {23, 1, "MIDI CC Mode"}}, // see: Pro800MidiMode
    {SETTINGS_MIDI_PC_MODE,            {24, 1, "MIDI PC Mode"}}, // see: Pro800MidiMode
    // 25 = unknown
    // 26 = overflow
    {SETTINGS_SYNC_IN_FORWARD,         {27, 1, "Sync In Forward Enable"}}, // see: Pro800SettingsOnOff
    {SETTINGS_EXTERNAL_CV_AMOUNT,      {28, 2, "External CV Amount"}}, 
    // 30 = unknown
    {SETTINGS_SYNC_CLOCK_SUBDIVISION,  {31, 1, "Sync Clock Subdivision"}}, // see: Pro800SyncSubdivision
    {SETTINGS_VOICE_PRIORITY,          {32, 1, "Voice Priority"}}, //see: Pro800VoicePriority
    {SETTINGS_SHOW_PRESET_NAME,        {33, 1, "Show Preset Name"}}, // see: Pro800SettingsOnOff
    // 34 = overflow
    {SETTINGS_SYNC_IN_POLARITY,        {35, 1, "Sync In Polarity"}}, // see: Pro800SettingsPolarity
    // 36 = unknown
    {SETTINGS_TUNER_PRECISION,         {37, 1, "Tuner Precision"}}, // see: Pro800TunerPrecision
    {SETTINGS_SYNC_IN_START_STOP,      {38, 1, "Sync In Start-Stop"}}, // see: Pro800SettingsOnOff
    {SETTINGS_SYNC_IN_PPQN,            {39, 1, "Sync In PPQN"}}, // see: Pro800SettingsSyncInPPQN
    {SETTINGS_SYNC_CLOCK_NOTE_LENGTH,  {40, 1, "Sync Clock Note Length"}}, // 5-100
    {SETTINGS_SYNC_CLOCK_SWING,        {41, 1, "Sync Clock Swing"}}, // 5-95
    // 42 = overflow
    {SETTINGS_AFTERTOUCH_VCA_POLARITY, {43, 1, "Aftertouch VCA Polarity"}}, // see: Pro800SettingsPolarity
    {SETTINGS_AFTERTOUCH_VCF_POLARITY, {44, 1, "Aftertouch VCF Polarity"}}, // see: Pro800SettingsPolarity
    {SETTINGS_TRANSPOSE,               {45, 1, "Transpose", true}}, // (-12 - +12; -1 = 0x7f, +1 = 0x01)
    {SETTINGS_LOCAL_ENABLE,            {46, 1, "Local Enable"}}, // see: Pro800SettingsOnOff
    {SETTINGS_SOFT_THRU,               {47, 1, "Soft Thru"}}, // see: Pro800SettingsOnOff
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



enum Pro800ProgramSpecialParameters
{
    PROGRAM_NUM_LSB = 0, // offset from Pro800MidiMessage::POS_MESSAGE_START
    PROGRAM_NUM_MSB = 1,

    PROGRAM_VERSION_POS = 7,

    PROGRAM_NAME_FIRST_CHAR = 174,
    PROGRAM_NAME_LAST_CHAR  = 191,
};

enum Pro800ProgramLfoDestination
{
    PROGRAM_LFO_DEST_FREQ_AB = 1, // bit 0
    PROGRAM_LFO_DEST_FILTER  = 2, // bit 1
    PROGRAM_LFO_DEST_PW_AB   = 4, // bit 2
    PROGRAM_LFO_DEST_FREQ_A  = 8, // bit 3
    PROGRAM_LFO_DEST_FREQ_B  = 16, // bit 4
    PROGRAM_LFO_DEST_FREQ_AB_VCA  = 32, // bit 5

};

enum Pro800ProgramField
{
    PROGRAM_FIELD_NUM = 0,
    PROGRAM_FIELD_STORAGE_CODE,
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

const std::map<Pro800ProgramField, Pro800Parameter> PRO800_PROGRAM_FIELDS =
{
    // field, {first byte, num bytes, name, isSigned}
    {PROGRAM_FIELD_STORAGE_CODE,              {3, 4, "Storage Code"}},
    {PROGRAM_FIELD_VERSION,                   {PROGRAM_VERSION_POS, 1, "Version"}},

    {PROGRAM_FIELD_OSC_A_FREQ,                {8, 2, "Osc A Frequency"}},
    // 10 = overflow
    {PROGRAM_FIELD_OSC_A_LEVEL,               {11, 2, "Osc A Level"}},
    {PROGRAM_FIELD_OSC_A_PULSE_WIDTH,         {13, 2, "Osc A Pulse Width"}},

    {PROGRAM_FIELD_OSC_B_FREQ,                {15, 2, "Osc B Frequency"}},
    {PROGRAM_FIELD_OSC_B_LEVEL,               {17, 2, "Osc B Level"}},
    // 18 = overflow
    {PROGRAM_FIELD_OSC_B_PULSE_WIDTH,         {20, 2, "Osc B Pulse Width"}},
    {PROGRAM_FIELD_OSC_B_FINE_FREQ,           {22, 2, "Osc B Fine Frequency"}},

    {PROGRAM_FIELD_FILTER_CUTOFF,             {24, 2, "Filter Cutoff"}},
    // 26 = overflow
    {PROGRAM_FIELD_FILTER_RESONANCE,          {27, 2, "Filter Resonance"}},
    {PROGRAM_FIELD_FILTER_ENV_AMOUNT,         {29, 2, "Filter Envelope Amount"}},
    {PROGRAM_FIELD_FILTER_RELEASE,            {31, 2, "Filter Release"}},
    {PROGRAM_FIELD_FILTER_SUSTAIN,            {33, 2, "Filter Sustain"}},
    // 34 = overflow
    {PROGRAM_FIELD_FILTER_DECAY,              {36, 2, "Filter Decay"}},
    {PROGRAM_FIELD_FILTER_ATTACK,             {38, 2, "Filter Attack"}},

    {PROGRAM_FIELD_AMP_RELEASE,               {40, 2, "Amp Release"}},
    // 42 = overflow
    {PROGRAM_FIELD_AMP_SUSTAIN,               {43, 2, "Amp Sustain"}},
    {PROGRAM_FIELD_AMP_DECAY,                 {45, 2, "Amp Decay"}},
    {PROGRAM_FIELD_AMP_ATTACK,                {47, 2, "Amp Attack"}},

    {PROGRAM_FIELD_POLYMOD_SOURCE_FILTER_ENV, {49, 2, "Poly-Mod Source Filter Env"}},
    // 50 = overflow
    {PROGRAM_FIELD_POLYMOD_SOURCE_OSC_B,      {52, 2, "Poly-Mod Source Osc B"}},

    {PROGRAM_FIELD_LFO_FREQ,                  {54, 2, "LFO Frequency"}},
    {PROGRAM_FIELD_LFO_AMOUNT,                {56, 2, "LFO Amount"}},

    // 58 = overflow
    {PROGRAM_FIELD_GLIDE_AMOUNT,              {59, 2, "Glide Amount"}},

    {PROGRAM_FIELD_AMP_VELOCITY,              {61, 2, "Amp Velocity"}},
    {PROGRAM_FIELD_FILTER_VELOCITY,           {63, 2, "Filter Velocity"}},

    {PROGRAM_FIELD_OSC_A_SHAPE_SAW,           {65, 1, "Osc A Shape Saw"}},
    // 66 = overflow
    {PROGRAM_FIELD_OSC_A_SHAPE_TRI,           {67, 1, "Osc A Shape Tri"}},
    {PROGRAM_FIELD_OSC_A_SHAPE_RECT,          {68, 1, "Osc A Shape Rect"}},

    {PROGRAM_FIELD_OSC_B_SHAPE_SAW,           {69, 1, "Osc B Shape Saw"}},
    {PROGRAM_FIELD_OSC_B_SHAPE_TRI,           {70, 1, "Osc B Shape Tri"}},
    {PROGRAM_FIELD_OSC_B_SHAPE_RECT,          {71, 1, "Osc B Shape Rect"}},

    {PROGRAM_FIELD_OSC_A_SYNC,                {72, 1, "Osc A Sync"}},

    {PROGRAM_FIELD_POLYMOD_DEST_FREQ_A,       {73, 1, "Poly-Mod Dest Freq A"}},
    // 74 = overflow
    {PROGRAM_FIELD_POLYMOD_DEST_FILTER,       {75, 1, "Poly-Mod Dest Filter"}},

    {PROGRAM_FIELD_LFO_SHAPE,                 {76, 1, "LFO Shape"}}, 
    {PROGRAM_FIELD_LFO_SPEED,                 {77, 1, "LFO Speed"}}, 

    {PROGRAM_FIELD_LFO_DEST,                  {78, 1, "LFO Destination"}},

    {PROGRAM_FIELD_FILTER_KEY_TRACKING,       {79, 1, "Filter Keyboard Tracking"}},
    {PROGRAM_FIELD_FILTER_ENV_SHAPE,          {80, 1, "Filter Envelope Shape"}},
    {PROGRAM_FIELD_FILTER_ENV_SPEED,          {81, 1, "Filter Envelope Speed"}},

    // 82 = overflow
    {PROGRAM_FIELD_AMP_ENV_SHAPE,             {83, 1, "Amp Envelope Shape"}},
    {PROGRAM_FIELD_POLYMOD_UNISON_TRACK,      {84, 1, "Poly-Mod Unison Track"}},
    {PROGRAM_FIELD_PITCHBEND_TARGET,          {85, 1, "Pitchbend Target"}},
    {PROGRAM_FIELD_MODWHEEL_RANGE,            {86, 1, "Mod Wheel Range"}},

    {PROGRAM_FIELD_OSC_A_FREQ_POT_MODE,       {87, 1, "Osc A Freq Pot Mode"}},
    {PROGRAM_FIELD_OSC_A_FREQ_POT_MODE,       {88, 1, "Osc A Freq Pot Mode"}},

    {PROGRAM_FIELD_LFO_MODULATION_DELAY,      {89, 2, "Modulation Delay"}},
    // 90 = overflow

    {PROGRAM_FIELD_LFO_VIBRATO_FREQ,          {92, 2, "Vibrato Freq"}},
    {PROGRAM_FIELD_LFO_VIBRATO_AMOUNT,        {94, 2, "Vibrato Amount"}},

    {PROGRAM_FIELD_UNISON_DETUNE,             {96, 2, "Unison Detune"}},

    // 98 = overflow
    {PROGRAM_FIELD_MODWHEEL_TARGET,           {99, 1, "Mod Wheel Target"}},

    // 100 = reserved
    {PROGRAM_FIELD_VOICE1_OFFSET,             {101, 1, "Voice 1 Offset"}},
    {PROGRAM_FIELD_VOICE2_OFFSET,             {102, 1, "Voice 2 Offset"}},
    {PROGRAM_FIELD_VOICE3_OFFSET,             {103, 1, "Voice 3 Offset"}},
    {PROGRAM_FIELD_VOICE4_OFFSET,             {104, 1, "Voice 4 Offset"}},
    {PROGRAM_FIELD_VOICE5_OFFSET,             {105, 1, "Voice 5 Offset"}},
    // 106 = overflow
    {PROGRAM_FIELD_VOICE6_OFFSET,             {107, 1, "Voice 6 Offset"}},
    {PROGRAM_FIELD_VOICE7_OFFSET,             {108, 1, "Voice 7 Offset"}},
    {PROGRAM_FIELD_VOICE8_OFFSET,             {109, 1, "Voice 8 Offset"}},

    {PROGRAM_FIELD_TUNING_C,                  {110, 4, "Tune Per Note - C"}},
    // 114 = overflow
    {PROGRAM_FIELD_TUNING_C_SHARP,            {115, 4, "Tune Per Note - C#"}},
    {PROGRAM_FIELD_TUNING_D,                  {119, 4, "Tune Per Note - D"}},
    // 122 = overflow
    {PROGRAM_FIELD_TUNING_D_SHARP,            {124, 4, "Tune Per Note - D#"}},
    {PROGRAM_FIELD_TUNING_E,                  {128, 4, "Tune Per Note - E"}},
    // 130 = overflow
    {PROGRAM_FIELD_TUNING_F,                  {133, 4, "Tune Per Note - F"}},
    {PROGRAM_FIELD_TUNING_F_SHARP,            {137, 4, "Tune Per Note - F#"}},
    // 138 = overflow
    {PROGRAM_FIELD_TUNING_G,                  {142, 4, "Tune Per Note - G"}},
    // 146 = overflow
    {PROGRAM_FIELD_TUNING_G_SHARP,            {147, 4, "Tune Per Note - G#"}},
    {PROGRAM_FIELD_TUNING_A,                  {151, 4, "Tune Per Note - A"}},
    // 154 = overflow
    {PROGRAM_FIELD_TUNING_A_SHARP,            {156, 4, "Tune Per Note - A#"}},
    {PROGRAM_FIELD_TUNING_B,                  {160, 4, "Tune Per Note - B"}},
    // 162 = overflow

    {PROGRAM_FIELD_NOISE,                     {165, 2, "Noise Amount"}},
    {PROGRAM_FIELD_AMP_AFTERTOUCH_AMOUNT,     {167, 2, "Amp Aftertouch Amount"}},
    {PROGRAM_FIELD_FILTER_AFTERTOUCH_AMOUNT,  {169, 2, "Filter Aftertouch Amount"}},
    // 170 = overflow

    {PROGRAM_FIELD_AMP_ENV_SPEED,             {172, 1, "Amp Envelope Speed"}},
    {PROGRAM_FIELD_ARP_MODE,                  {173, 1, "ARP Mode"}},

    // 174 = first char of preset name

    // 178 = overflow byte, not used by name
    // 186 = overflow byte, not used by name

    // 191 = last char of preset name

    // only in preset version 110 and newer:
    {PROGRAM_FIELD_LFO_AFTERTOUCH_AMOUNT,     {192, 2, "LFO Aftertouch Amount"}},
    // 194 = overflow

    // only in preset version 111 and newer:
    {PROGRAM_FIELD_VOICE_SPREAD_ENABLE,       {195, 1, "Voice Spread Enable"}},
    {PROGRAM_FIELD_KEY_TRACKING_REF_NOTE,     {196, 1, "Key Tracking Ref Note"}},
    {PROGRAM_FIELD_GLIDE_MODE,                {197, 1, "Glide Mode"}},

    {PROGRAM_FIELD_PITCHBEND_RANGE,           {198, 2, "Pitchbend Range"}},
   
};