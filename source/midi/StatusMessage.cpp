#include "StatusMessage.h"

#include <sstream>

const std::map<StatusMessage::Status, const char *> StatusMessage::STATUS_STRING = {
    {STATUS_OK, "OK"},
    {STATUS_ERROR, "Error"},
    {STATUS_INVALID, "Invalid Status"}};

StatusMessage::StatusMessage(const juce::MidiMessage &message) : Pro800MidiMessage(message)
{

}

bool StatusMessage::isValid() const
{
    return Pro800MidiMessage::isValid() && (getRawDataSize() >= POS_STATUS_BYTE);
}

juce::String StatusMessage::toString() const
{
    std::stringstream ss;
    ss << "Pro-800 Status response: ";
    ss << STATUS_STRING.at(this->getStatus());
    return ss.str();
}

StatusMessage::Status StatusMessage::getStatus() const
{
    if (isValid())
    {
        return (Status)this->getRawData()[POS_STATUS_BYTE];
    }
    else
    {
        return STATUS_INVALID;
    }
}

unsigned char StatusMessage::getResponseType() const
{
    return RESPONSE_ID;
}
