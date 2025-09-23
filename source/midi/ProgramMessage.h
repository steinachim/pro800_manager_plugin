#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Pro800MidiMessage.h"

class ProgramMessage : public Pro800MidiMessage
{
    enum PresetStructurePositions // does not match midi dump message
    {
        PRESET_PROGRAM_LSB            = POS_MESSAGE_START,
        PRESET_PROGRAM_MSB            = POS_MESSAGE_START+1,
        PRESET_STORAGE_CODE           = POS_MESSAGE_START+2,
        PRESET_VERSION                = POS_MESSAGE_START+6,
        PRESET_OSC_A_FREQ             = POS_MESSAGE_START+7,
        PRESET_OSC_A_VOLUME           = POS_MESSAGE_START+9,
        PRESET_OSC_A_PULSE_WIDTH      = POS_MESSAGE_START+11,
        PRESET_OSC_B_FREQ             = POS_MESSAGE_START+13,
        PRESET_OSC_B_VOLUME           = POS_MESSAGE_START+15,
        PRESET_OSC_B_PULSE_WIDTH      = POS_MESSAGE_START+17,
        PRESET_OSC_B_FINE_FREQ        = POS_MESSAGE_START+19,
        PRESET_VCF_CUTOFF             = POS_MESSAGE_START+21,
        PRESET_VCF_RESONANCE          = POS_MESSAGE_START+23,
        PRESET_VCF_ENVELOPE_AMT       = POS_MESSAGE_START+25,
        PRESET_VCF_RELEASE            = POS_MESSAGE_START+27,
        PRESET_VCF_SUSTAIN            = POS_MESSAGE_START+29,
        PRESET_VCF_DECAY              = POS_MESSAGE_START+31,
        PRESET_VCF_ATTACK             = POS_MESSAGE_START+33,
        PRESET_VCA_RELEASE            = POS_MESSAGE_START+35,
        PRESET_VCA_SUSTAIN            = POS_MESSAGE_START+37,
        PRESET_VCA_DECAY              = POS_MESSAGE_START+39,
        PRESET_VCA_ATTACK             = POS_MESSAGE_START+41,
        PRESET_POLYMOD_FILTER_ENV     = POS_MESSAGE_START+43,
        PRESET_POLYMOD_OSC_B          = POS_MESSAGE_START+45,
        PRESET_LFO_FREQ               = POS_MESSAGE_START+47,
        PRESET_LFO_AMOUNT             = POS_MESSAGE_START+49,
        PRESET_GLIDE                  = POS_MESSAGE_START+51,
        PRESET_VCA_VELOCITY           = POS_MESSAGE_START+53,
        PRESET_VCF_VELOCITY           = POS_MESSAGE_START+55,
        PRESET_OSC_A_SAW              = POS_MESSAGE_START+57,
        PRESET_OSC_A_TRIANGLE         = POS_MESSAGE_START+58,
        PRESET_OSC_A_SQUARE           = POS_MESSAGE_START+59,
        PRESET_OSC_B_SAW              = POS_MESSAGE_START+60,
        PRESET_OSC_B_TRIANGLE         = POS_MESSAGE_START+61,
        PRESET_OSC_B_SQUARE           = POS_MESSAGE_START+62,
        PRESET_OSC_B_SYNC             = POS_MESSAGE_START+63,
        PRESET_POLYMOD_FREQ_A         = POS_MESSAGE_START+64,
        PRESET_POLYMOD_FILTER_CUTOFF  = POS_MESSAGE_START+65,
        PRESET_POLYMOD_LFO_SHAPE      = POS_MESSAGE_START+66,
        PRESET_POLYMOD_LFO_SPEED      = POS_MESSAGE_START+67,
        PRESET_POLYMOD_LFO_TARGETS    = POS_MESSAGE_START+68,
        PRESET_VCF_KEYBOARD_MOD       = POS_MESSAGE_START+69,
        PRESET_VCF_ENVELOPE_LIN_EXP   = POS_MESSAGE_START+70,
        PRESET_VCF_ENVELOPE_FAST_SLOW = POS_MESSAGE_START+71,
        PRESET_VCA_ENVELOPE_LIN_EXP   = POS_MESSAGE_START+72,
        PRESET_UNISON                 = POS_MESSAGE_START+73,
        PRESET_PITCHBEND_TARGET       = POS_MESSAGE_START+74,
        PRESET_MODWHEEL_RANGE         = POS_MESSAGE_START+75,
        PRESET_OSC_A_FREQ_POT_MODE    = POS_MESSAGE_START+76,
        PRESET_OSC_B_FREQ_POT_MODE    = POS_MESSAGE_START+77,
        PRESET_LFO_MODULATION_DELAY   = POS_MESSAGE_START+78,
        PRESET_LFO_VIBRATO_FREQ       = POS_MESSAGE_START+80,
        PRESET_LFO_VIBRATO_AMOUNT     = POS_MESSAGE_START+82,
        PRESET_UNISON_DETUNE          = POS_MESSAGE_START+84,
        PRESET_MODWHEEL_TARGET        = POS_MESSAGE_START+86,
        PRESET_RESERVED               = POS_MESSAGE_START+87,
        PRESET_CHORD_VOICE_1          = POS_MESSAGE_START+88,
        PRESET_CHORD_VOICE_2          = POS_MESSAGE_START+89,
        PRESET_CHORD_VOICE_3          = POS_MESSAGE_START+90,
        PRESET_CHORD_VOICE_4          = POS_MESSAGE_START+91,
        PRESET_CHORD_VOICE_5          = POS_MESSAGE_START+92,
        PRESET_CHORD_VOICE_6          = POS_MESSAGE_START+93,
        PRESET_CHORD_VOICE_7          = POS_MESSAGE_START+94,
        PRESET_CHORD_VOICE_8          = POS_MESSAGE_START+95,
        PRESET_TUNING_C               = POS_MESSAGE_START+96,
        PRESET_TUNING_CIS             = POS_MESSAGE_START+100,
        PRESET_TUNING_D               = POS_MESSAGE_START+104,
        PRESET_TUNING_DIS             = POS_MESSAGE_START+108,
        PRESET_TUNING_E               = POS_MESSAGE_START+112,
        PRESET_TUNING_F               = POS_MESSAGE_START+116,
        PRESET_TUNING_FIS             = POS_MESSAGE_START+120,
        PRESET_TUNING_G               = POS_MESSAGE_START+124,
        PRESET_TUNING_GIS             = POS_MESSAGE_START+128,
        PRESET_TUNING_A               = POS_MESSAGE_START+132,
        PRESET_TUNING_AIS             = POS_MESSAGE_START+136,
        PRESET_TUNING_B               = POS_MESSAGE_START+140,
        PRESET_NOISE                  = POS_MESSAGE_START+144,
        PRESET_VCA_AFTERTOUCH_AMOUNT  = POS_MESSAGE_START+146,
        PRESET_VCF_AFTERTOUCH_AMOUNT  = POS_MESSAGE_START+148,
        PRESET_VCA_ENVELOPE_FAST_SLOW = POS_MESSAGE_START+150,
        PRESET_ARP_MODE               = POS_MESSAGE_START+151,
        PRESET_PROGRAM_NAME_START     = POS_MESSAGE_START+152,
        PRESET_PROGRAM_NAME_END       = POS_MESSAGE_START+167, // NULL
        PRESET_LFO_AFTERTOUCH_AMOUNT  = POS_MESSAGE_START+168,
        PRESET_VOICE_SPREAD           = POS_MESSAGE_START+170,
        PRESET_KEYB_TRACKING_REF_NOTE = POS_MESSAGE_START+171,
        PRESET_GLIDE_MODE             = POS_MESSAGE_START+172,
        PRESET_PITCHBEND_RANGE        = POS_MESSAGE_START+173
    };

    enum RawDumpPosition
    {
        DUMP_POS_PROGRAM_LSB = 9,
        DUMP_POS_PROGRAM_MSB = 10,
        DUMP_POS_PROGRAM_NAME_START = 183,
        DUMP_POS_PROGRAM_NAME_END   = DUMP_POS_PROGRAM_NAME_START + 15
    };


public:
    static const unsigned char MAX_PROGRAM_NAME_LENGTH = 15;
    static const unsigned short NUM_PROGRAMS = 400;
    static const unsigned char REQUEST_ID = 0x77;
    static const unsigned char RESPONSE_ID = 0x78;

    static juce::MidiMessage request(int programNumber);

    ProgramMessage(const juce::MidiMessage &message);
    virtual Pro800MessageType getMessageType() const override { return Pro800MessageType::PRO800_PROGRAM_MESSAGE;}

    virtual bool isValid() const override;

    short getProgramNumber() const;
    std::string getProgramBankNumber() const;
    void setProgramNumber(short programNumber);

    std::string getProgramName() const;
    void setProgramName(const std::string &newName);

    virtual juce::String toString() const override;

    static juce::MidiMessage getRequestDumpMessage(unsigned short programNumber);

protected:
    unsigned char getResponseType() const override;

    uint32_t getValue(RawDumpPosition position, size_t width);
    uint8_t  getValue8(RawDumpPosition position);
    uint16_t getValue16(RawDumpPosition position);
    uint32_t getValue32(RawDumpPosition position);

    std::string getValueString(RawDumpPosition start, RawDumpPosition end);

private:
    void extractDumpData();

    short programNumber;
    std::string programName;



};
