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

#include <juce_core/juce_core.h>

#include <cstdint>
#include <optional>
#include <vector>

/**
 * The SysEx messages that must never reach a Pro-800 (docs/Pro800SysExMessages.md, "Never send"). They are named
 * here rather than left out so that nobody rediscovers them by sending them.
 */
class Pro800Hazards
{
public:
    /** Why these raw bytes must not be sent, or nullopt if they are not a known hazard (or not a Pro-800 SysEx at all). */
    static std::optional<juce::String> hazardReason (const std::vector<uint8_t>& rawMessage);

    /** True for the hazards that cannot be undone from the host (factory reset, bootloader): worth a second confirmation. */
    static bool isIrreversible (const std::vector<uint8_t>& rawMessage);

    static constexpr uint8_t TYPE_UNKNOWN_03 = 0x03;
    static constexpr uint8_t PARAM_BOOTLOADER = 0x30;
    static constexpr uint8_t TYPE_CHANNEL_WRITE = 0x0E;
    static constexpr uint8_t TYPE_RELOAD = 0x32;
    static constexpr uint8_t TYPE_SET_NAME = 0x50;
    static constexpr uint8_t TYPE_FACTORY_RESET = 0x7D;

private:
    /** (type, first parameter) of a Pro-800 SysEx; the parameter is nullopt if the message has none. */
    static std::optional<std::pair<uint8_t, std::optional<uint8_t>>> typeAndParam (const std::vector<uint8_t>& rawMessage);
};
