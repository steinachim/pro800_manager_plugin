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

#include "PanelMessage.h"

juce::MidiMessage PanelMessage::request (Pro800PanelIndex index)
{
    return request (static_cast<uint8_t> (index));
}

juce::MidiMessage PanelMessage::request (uint8_t index)
{
    return makeRequest ({ REQUEST_ID, index });
}

PanelMessage::PanelMessage (const juce::MidiMessage& message) : Pro800MidiMessage (message)
{
}

bool PanelMessage::isValid() const
{
    return Pro800MidiMessage::isValid() && isDataPosition (POS_VALUE);
}

juce::String PanelMessage::toString() const
{
    if (!isValid())
    {
        return "Pro-800 panel/button message without a value (a button press, or truncated)";
    }

    return "Pro-800 panel state: " + getIndexName() + " = " + juce::String (getValue());
}

uint8_t PanelMessage::getIndex() const
{
    return getUint8Value (POS_INDEX);
}

int PanelMessage::getValue() const
{
    return isValid() ? getUint8Value (POS_VALUE) : 0;
}

juce::String PanelMessage::getIndexName() const
{
    const auto entry = PRO800_PANEL_INDEX_NAMES.find (static_cast<Pro800PanelIndex> (getIndex()));
    if (entry != PRO800_PANEL_INDEX_NAMES.end())
    {
        return entry->second;
    }

    return "index 0x" + juce::String::toHexString ((int) getIndex());
}

uint8_t PanelMessage::getResponseType() const
{
    return RESPONSE_ID;
}
