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

#include "SettingsMessage.h"

juce::MidiMessage SettingsMessage::request()
{
    return Pro800DataMessage::request (ADDRESS_LOW, ADDRESS_HIGH);
}

SettingsMessage::SettingsMessage (const juce::MidiMessage& message) : Pro800DataMessage (message)
{
}

bool SettingsMessage::isValid() const
{
    return Pro800MidiMessage::isValid() && (getRawDataSize() == SETTINGS_MESSAGE_SIZE);
}

juce::String SettingsMessage::toString() const
{
    return "Pro800 Settings Dump:\n" + fieldsToString (PRO800_SETTINGS_FIELDS);
}

void SettingsMessage::setValue (Pro800Settings setting, int value)
{
    setFieldValue (PRO800_SETTINGS_FIELDS, setting, value);
}

int SettingsMessage::getValue (Pro800Settings setting) const
{
    return getFieldValue (PRO800_SETTINGS_FIELDS, setting);
}
