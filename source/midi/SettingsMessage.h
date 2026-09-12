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

#include "../tailoring/Pro800Constants.h"
#include "../tailoring/Pro800SettingsConstants.h"
#include "Pro800DataMessage.h"

#include <optional>
#include <vector>

class SettingsMessage : public Pro800DataMessage
{
public:
    static constexpr size_t SETTINGS_MESSAGE_SIZE = 58;

    static constexpr uint8_t ADDRESS_LOW = 0x7E;
    static constexpr uint8_t ADDRESS_HIGH = 0x03;
    static constexpr int ADDRESS = 510; // the settings block shares the program address space

    static constexpr int NUM_BANKS = 4; // A-D
    static constexpr int PROGRAMS_PER_BANK = 100;

    static juce::MidiMessage request();

    explicit SettingsMessage (const juce::MidiMessage& message);
    virtual MessageType getMessageType() const override { return MessageType::PRO800_SETTINGS; }

    virtual bool isValid() const override;

    virtual juce::String toString() const override;

    void setValue (Pro800Settings setting, int value);
    int getValue (Pro800Settings setting) const;

    /**
     * The program (0-399) the synth's selection pointer names: the bank from CURRENT_BANK, the slot from
     * PRESET_NUM % 100 (the synth stores the full number but only uses that part). nullopt if the message is invalid.
     */
    std::optional<int> getCurrentProgram() const;

    /**
     * Moves the selection pointer to the program (0-399): PRESET_NUM = program, CURRENT_BANK = program / 100, both
     * in one block so the pair is never inconsistent. Writing the block moves the display, not the sound - the synth
     * recalls the pointer on a ReloadMessage (0x32 00).
     */
    void setCurrentProgram (int program);
};
