# Behringer Pro-800 Midi CC Messages

### Selecting a preset

Not a SysEx message: a preset is loaded with plain channel-voice MIDI, a
**bank select followed by a program change**.

    CC 0 (BANK_SELECT), value 0-3      -> bank A-D
    Program Change, value 0-99         -> slot within that bank

Program Change alone only reaches 0-127 and so cannot address all 400 presets;
the bank select is what makes the other three banks reachable.

Neither message is acknowledged - nothing comes back either way - so a preset
change sent on the wrong MIDI channel fails **silently**. The channel to use is
the synth's `MIDI RX Channel` setting (see the settings message in
[Pro800SysExMessages.md](Pro800SysExMessages.md)); note that when that setting
reads 1, the channel comes from the rear DIP switches and the synth does not
report which one they select.

### Front Panel Knobs
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
     --> LFO_SHAPE_PULSE = 0, 
     --> LFO_SHAPE_TRIANGLE = 22,
     --> LFO_SHAPE_RANDOM = 44,
     --> LFO_SHAPE_SINE = 66,
     --> LFO_SHAPE_NOISE = 8,
     --> LFO_SHAPE_SAW = 110,
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
     --> TRACKING_OFF = 0,
     --> TRACKING_HALF = 43,
     --> TRACKING_FULL = 86
    
    AMP_ATTACK = 25,
    AMP_DECAY = 24,
    AMP_SUSTAIN = 23,
    AMP_RELEASE = 22,

    MASTER_TUNE = 3,
    MASTER_VOLUME = 7,

### Menu: Performance 1
    LFO_TARGET = 59, // see: Pro800LFOTarget
     --> LFO_TARGET_OSC_AB = 0,
     --> LFO_TARGET_OSC_A = 33,
     --> LFO_TARGET_OSC_B = 66,
     --> LFO_TARGET_VCA = 99
    LFO_SPEED = 58, // see: Pro800Speed
     --> LFO_SPEED_SLOW = 0,
     --> LFO_SPEED_FAST = 64


### Menu: Performance 2
    VIBRATO_AMOUNT = 35,
    VIBRATO_SPEED = 34,

### Menu: Performance 3
    MOD_WHEEL_AMOUNT = 67, // see: Pro800ModWheelAmount
     --> MOD_WHEEL_AMOUNT_MIN = 0,
     --> MOD_WHEEL_AMOUNT_LOW = 32,
     --> MOD_WHEEL_AMOUNT_HIGH = 64,
     --> MOD_WHEEL_AMOUNT_FULL = 96
    MOD_WHEEL_TARGET = 70, // see: Pro800ModWheelTarget
     --> MOD_WHEEL_TARGET_LFO = 0,
     --> MOD_WHEEL_TARGET_VIBRATO = 64
    MODULATION_DELAY = 33,

### Menu: Performance 4
    VCA_ENV_SHAPE = 63, // see: Pro800EnvelopeShape
     --> ENV_SHAPE_LINEAR = 0,
     --> ENV_SHAPE_EXPONENTIAL = 64
    VCA_ENV_SPEED = 72, // see: Pro800Speed
     --> ENV_SPEED_FAST = 0,
     --> ENV_SPEED_SLOW = 64

    VCF_ENV_SHAPE = 61, // see: Pro800EnvelopeShape
     --> ENV_SHAPE_LINEAR = 0,
     --> ENV_SHAPE_EXPONENTIAL = 64
    VCF_ENV_SPEED = 62, // see: Pro800Speed
     --> ENV_SPEED_FAST = 0,
     --> ENV_SPEED_SLOW = 64

### Menu: Performance 5
    PITCH_BEND_TARGET = 66, // see: Pro800PitchBendTarget
     --> PITCH_BEND_TARGET_OFF = 0,
     --> PITCH_BEND_TARGET_VCO = 32,
     --> PITCH_BEND_TARGET_VCF = 64,
     --> PITCH_BEND_TARGET_VOLUME = 96
    PITCH_BEND_RANGE = 42,  
     --> 0-31 semitones (0-3 = 0, 4-7 = 1, 8-11 = 2, ..., 124-127 = 31)

### Menu: Performance 6
    OSC_A_FREQ_POT_MODE = 68, // see: Pro800FreqPotMode
     --> FREQ_POT_MODE_FREE = 0,
     --> FREQ_POT_MODE_SEMI = 32,
     --> FREQ_POT_MODE_OCT = 64,
     --> FREQ_POT_MODE_FIXED = 96
  
    OSC_B_FREQ_POT_MODE = 69, // see: Pro800FreqPotMode
     --> FREQ_POT_MODE_FREE = 0,
     --> FREQ_POT_MODE_SEMI = 32,
     --> FREQ_POT_MODE_OCT = 64,
     --> FREQ_POT_MODE_FIXED = 96

    KEYBOARD_TRACKING = 78, // see: Pro800KeyboardTracking
     --> KEYBOARD_TRACKING_C1 = 0,
     --> KEYBOARD_TRACKING_C2 = 32,
     --> KEYBOARD_TRACKING_C3 = 64,
     --> KEYBOARD_TRACKING_C4 = 96

### Menu: Performance 7
    VCA_VELOCITY_AMOUNT = 31,
    VCF_VELOCITY_AMOUNT = 32,

### Menu: Performance 8
    VCA_AFTERTOUCH_AMOUNT = 39,
    VCF_AFTERTOUCH_AMOUNT = 40,
    LFO_AFTERTOUCH_AMOUNT = 41,

### Menu: Performance 9
    UNISON_SPREAD_DETUNE = 36,
    VOICE_SPREAD_ENABLE = 77, // see: Pro800OnOff
     --> OFF = 0,
     --> ON = 64

### Menu: Performance 0
    GLIDE_MODE = 79 // see: Pro800GlideMode
     --> GLIDE_MODE_TIME = 0,
     --> GLIDE_MODE_SPEED = 64
