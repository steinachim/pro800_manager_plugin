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
    return Pro800MidiMessage::isValid() && (rawData->size() == SETTINGS_MESSAGE_SIZE);
}

juce::String SettingsMessage::toString() const
{
    std::stringstream ss;
    ss << "Pro800 Settings Dump:\n";

    for ( auto param : PRO800_SETTINGS_FIELDS )
    {
        int value = getValue(param.first);
        int maxValue = (1 << param.second.numBytes*8) - 1;

        ss << param.second.name << ": " << getValue(param.first) << " (display: " << value * 999 / maxValue << ")\n";
    }
       
    return ss.str();
}

unsigned char SettingsMessage::getResponseType() const
{
    return RESPONSE_ID;
}

void SettingsMessage::setValue(Pro800Settings setting, int value)
{
    if ( PRO800_SETTINGS_FIELDS.contains(setting) )
    {
        Pro800Parameter param = PRO800_SETTINGS_FIELDS.at(setting);
        Pro800MidiMessage::setValue(param.firstByte, param.numBytes, value);
    }
    else
    {
        std::cerr << "SettingsMessage::setValue(): No setter for field defined: " << setting << std::endl;
    }    
}

int SettingsMessage::getValue(Pro800Settings setting) const
{
    if ( PRO800_SETTINGS_FIELDS.contains(setting) )
    {
        Pro800Parameter param = PRO800_SETTINGS_FIELDS.at(setting);
        return Pro800MidiMessage::getValue(param.firstByte, param.numBytes, param.isSigned);
    }
    else
    {
        std::cerr << "SettingsMessage::getValue(): No getter for field defined: " << setting << std::endl;
    }

    return 0;
}
