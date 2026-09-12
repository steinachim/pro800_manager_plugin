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

#include "MidiDeviceComboBox.h"

void MidiDeviceComboBox::setDevices (const juce::Array<juce::MidiDeviceInfo>& newDevices)
{
    const juce::String previouslySelected = getSelectedDeviceIdentifier();

    this->devices = newDevices;

    clear (juce::dontSendNotification);
    addItem ("None", NONE_ITEM_ID);

    int selectedItemId = NONE_ITEM_ID;
    for (int i = 0; i < this->devices.size(); i++)
    {
        const int itemId = FIRST_DEVICE_ITEM_ID + i;
        addItem (this->devices[i].name, itemId);

        if (this->devices[i].identifier == previouslySelected)
        {
            selectedItemId = itemId;
        }
    }

    setSelectedId (selectedItemId, juce::dontSendNotification);
}

juce::String MidiDeviceComboBox::getSelectedDeviceIdentifier() const
{
    const int index = getSelectedId() - FIRST_DEVICE_ITEM_ID;
    if (index < 0 || index >= this->devices.size())
    {
        return {}; // "None" or nothing selected
    }

    return this->devices[index].identifier;
}

bool MidiDeviceComboBox::selectDeviceContaining (const juce::String& nameFragment)
{
    for (int i = 0; i < this->devices.size(); i++)
    {
        if (this->devices[i].name.containsIgnoreCase (nameFragment))
        {
            setSelectedId (FIRST_DEVICE_ITEM_ID + i, juce::dontSendNotification);
            return true;
        }
    }

    return false;
}
