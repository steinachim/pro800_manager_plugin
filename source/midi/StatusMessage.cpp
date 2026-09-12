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

#include "StatusMessage.h"

const std::map<StatusMessage::Status, const char *> StatusMessage::STATUS_STRING = {
    {STATUS_OK, "OK"},
    {STATUS_ERROR, "Error"},
    {STATUS_INVALID, "Invalid Status"}};

StatusMessage::StatusMessage(const juce::MidiMessage &message) : Pro800MidiMessage(message)
{

}

bool StatusMessage::isValid() const
{
    return Pro800MidiMessage::isValid() && isDataPosition(POS_STATUS_BYTE);
}

juce::String StatusMessage::toString() const
{
    const Status status = this->getStatus();
    const auto statusString = STATUS_STRING.find(status);

    juce::String result = "Pro-800 Status response: ";
    if ( statusString != STATUS_STRING.end() )
    {
        result += statusString->second;
    }
    else
    {
        // the device answered with a status byte we don't know: report it instead of throwing
        result += "Unknown status " + juce::String::toHexString((int)status);
    }

    return result;
}

StatusMessage::Status StatusMessage::getStatus() const
{
    if (isValid())
    {
        return (Status)this->getUint8Value(POS_STATUS_BYTE);
    }
    else
    {
        return STATUS_INVALID;
    }
}

uint8_t StatusMessage::getResponseType() const
{
    return RESPONSE_ID;
}
