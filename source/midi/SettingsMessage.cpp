#include "SettingsMessage.h"

#include <sstream>
#include <list>
#include <bitset>
#include <iostream>

juce::MidiMessage SettingsMessage::request()
{
    std::vector<uint8_t> request;
    request.insert(request.end(), std::begin(PRO800_HEADER), std::end(PRO800_HEADER));
    request.insert(request.end(), {REQUEST_ID, ADDRESS_LOW, ADDRESS_HIGH});
    return juce::MidiMessage::createSysExMessage(request.data(), (int)request.size());
}

SettingsMessage::SettingsMessage(const juce::MidiMessage &message) : Pro800MidiMessage(message)
{

}

bool SettingsMessage::isValid() const
{
    return Pro800MidiMessage::isValid() && (getRawDataSize() >= POS_MESSAGE_START + SETTINGS_LENGTH);
}

juce::String SettingsMessage::toString() const
{
    uint16_t extCvAmount = getExternalCVAmount();
    unsigned short extCvAmountDisplayValue = (extCvAmount * 999)/65535;

    uint8_t presetMode = getUint8Value(SETTINGS_PRESET_MODE);
    std::string presetModeStr = presetMode == PRESET_MODE_MANUAL ? "MANUAL" : presetMode == PRESET_MODE_LOADED ? "LOADED" : "EDITED";

    uint16_t currentPreset = getCurrentPreset();
    juce::String currentPresetBytes = juce::String::formatted(" (LSB: 0x%02x, MSB: 0x%02x)", currentPreset & 0x7F, (currentPreset >> 7));

    std::stringstream ss;
    ss << "Pro800 Settings Message\n"
       << "Brightness: " << (unsigned int)getUint8Value(SETTINGS_BRIGHTNESS) << "\n"
       << "External CV Amount: " << extCvAmount << " (display: " << extCvAmountDisplayValue << ")\n"
       << "Edited: " << presetModeStr << " (" << (int)presetMode << ")\n" 
       << "Preset: " << getCurrentPreset() << currentPresetBytes << "\n"
       << "raw: " << juce::String::toHexString(getRawData(), getRawDataSize());

    return ss.str();
}

unsigned char SettingsMessage::getResponseType() const
{
    return RESPONSE_ID;
}

uint16_t SettingsMessage::getExternalCVAmount() const
{
    return getUint16Value(SETTINGS_EXTERNAL_CV_AMOUNT_MSB, SETTINGS_EXTERNAL_CV_AMOUNT_LSB, {SETTINGS_EXTERNAL_CV_AMOUNT_OVERFLOW, SETTINGS_EXTERNAL_CV_AMOUNT_OVERFLOW}, {OVERFLOW_EXTERNAL_CV_AMOUNT_BIT8, OVERFLOW_EXTERNAL_CV_AMOUNT_BIT16});
}

void SettingsMessage::setExternalCVAmount(uint16_t value)
{
    setUint16Value(SETTINGS_EXTERNAL_CV_AMOUNT_MSB, SETTINGS_EXTERNAL_CV_AMOUNT_LSB, {SETTINGS_EXTERNAL_CV_AMOUNT_OVERFLOW, SETTINGS_EXTERNAL_CV_AMOUNT_OVERFLOW}, {OVERFLOW_EXTERNAL_CV_AMOUNT_BIT8, OVERFLOW_EXTERNAL_CV_AMOUNT_BIT16}, value);
}

uint16_t SettingsMessage::getClockBPM() const
{
    return getUint16Value(SETTINGS_SYNC_CLOCK_BPM_MSB, SETTINGS_SYNC_CLOCK_BPM_LSB, {SETTINGS_OVERFLOW_BPM_VOICE8}, {OVERFLOW_BPM_BIT8});
}

void SettingsMessage::setClockBPM(uint16_t value)
{
    setUint16Value(SETTINGS_SYNC_CLOCK_BPM_MSB, SETTINGS_SYNC_CLOCK_BPM_LSB, {SETTINGS_OVERFLOW_BPM_VOICE8}, {OVERFLOW_BPM_BIT8}, value);
}

uint8_t SettingsMessage::getVoiceStatus() const
{
    return getUint8Value(SETTINGS_VOICE_KILL, SETTINGS_OVERFLOW_BPM_VOICE8, OVERFLOW_VOICE8_BIT8);
}

void SettingsMessage::setVoiceStatus(uint8_t value)
{
    setUint8Value(SETTINGS_VOICE_KILL, SETTINGS_OVERFLOW_BPM_VOICE8, OVERFLOW_VOICE8_BIT8, value);
}   

int8_t SettingsMessage::getTranspose() const
{
    uint8_t value = getUint8Value(SETTINGS_TRANSPOSE, SETTINGS_TRANSPOSE_OVERFLOW, OVERFLOW_TRANSPOSE_BIT8);
    return (int8_t)value;
}

void SettingsMessage::setTranspose(int8_t value)
{
    setUint8Value(SETTINGS_TRANSPOSE, SETTINGS_TRANSPOSE_OVERFLOW, OVERFLOW_TRANSPOSE_BIT8, (uint8_t)value);
}

uint16_t SettingsMessage::getCurrentPreset() const
{
    return getUint16Value(SETTINGS_PRESET_MSB, SETTINGS_PRESET_LSB, {SETTINGS_PRESET_OVERFLOW}, {OVERFLOW_PRESET_BIT8});
}

void SettingsMessage::setCurrentPreset(uint16_t value)
{    
    setUint16Value(SETTINGS_PRESET_MSB, SETTINGS_PRESET_LSB, {SETTINGS_PRESET_OVERFLOW}, {OVERFLOW_PRESET_BIT8}, value);
}

void SettingsMessage::setValue(Pro800Settings setting, int value)
{
    switch(setting)
    {
        case SETTINGS_TRANSPOSE:
            setTranspose((int8_t)value);
            break;

        case SETTINGS_EXTERNAL_CV_AMOUNT:
            setExternalCVAmount((uint16_t)value);
            break;

        case SETTINGS_SYNC_CLOCK_BPM:
            setClockBPM((uint16_t)value);
            break;

        case SETTINGS_VOICE_KILL:
            setVoiceStatus((uint8_t)value);            
            break;

        case SETTINGS_PRESET_NUM:
            setCurrentPreset((uint16_t)value);
            break;

        default:
            setUint8Value(setting, (uint8_t)value);
            break;
    }
}

int SettingsMessage::getValue(Pro800Settings setting) const
{
    switch(setting)
    {
        case SETTINGS_TRANSPOSE:
            return getTranspose();

        case SETTINGS_EXTERNAL_CV_AMOUNT:
            return getExternalCVAmount();

        case SETTINGS_SYNC_CLOCK_BPM:
            return getClockBPM();

        case SETTINGS_VOICE_KILL:
            return getVoiceStatus();

        case SETTINGS_PRESET_NUM:
            return getCurrentPreset();

        default:
            return (int)getUint8Value(setting);
    }
}
