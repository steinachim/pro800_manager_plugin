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

#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * A combo box listing MIDI devices ("None" plus one entry per device). It keeps the device infos,
 * so the selection can be mapped back to a device identifier.
 */
class MidiDeviceComboBox : public juce::ComboBox
{
public:
    MidiDeviceComboBox() = default;

    /** Replaces the list. The selection is kept if that device is still available, otherwise "None". */
    void setDevices (const juce::Array<juce::MidiDeviceInfo>& newDevices);

    /** The identifier of the selected device, or an empty string for "None". */
    juce::String getSelectedDeviceIdentifier() const;

    /** Selects the first device whose name contains the text (case-insensitive). Returns false if there is none. */
    bool selectDeviceContaining (const juce::String& nameFragment);

private:
    // combo box item ids must be non-zero: 1 = "None", devices[i] = FIRST_DEVICE_ITEM_ID + i
    static constexpr int NONE_ITEM_ID = 1;
    static constexpr int FIRST_DEVICE_ITEM_ID = 2;

    juce::Array<juce::MidiDeviceInfo> devices;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiDeviceComboBox)
};
