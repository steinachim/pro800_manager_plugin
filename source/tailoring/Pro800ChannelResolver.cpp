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

#include "Pro800ChannelResolver.h"
#include "Pro800SettingsConstants.h"

std::optional<int> Pro800MidiChannel::sendChannel() const
{
    switch (kind)
    {
        case Kind::CHANNEL:
            return channel;
        case Kind::ALL:
            return Pro800ChannelResolver::CHANNEL_FOR_ALL;
        case Kind::DIP:
            return channel > 0 ? std::optional<int> (channel) : std::nullopt;
        case Kind::THRU:
        case Kind::OFF:
        case Kind::INVALID:
        default:
            return std::nullopt;
    }
}

std::string Pro800MidiChannel::toString() const
{
    switch (kind)
    {
        case Kind::CHANNEL:
            return "channel " + std::to_string (channel);
        case Kind::ALL:
            return "any channel (ALL)";
        case Kind::THRU:
            return "THRU";
        case Kind::DIP:
            return channel > 0 ? "DIP switches (channel " + std::to_string (channel) + ")" : "DIP switches (not read)";
        case Kind::OFF:
            return "OFF";
        case Kind::INVALID:
        default:
            return "invalid (" + std::to_string (raw) + ")";
    }
}

std::optional<int> Pro800ChannelResolver::dipChannelFromPanel (const std::map<Pro800PanelIndex, int>& panelValues)
{
    int sum = 0;
    for (const auto& [index, weight] : PRO800_PANEL_DIP_WEIGHTS)
    {
        const auto reading = panelValues.find (index);
        if (reading == panelValues.end())
        {
            return std::nullopt;
        }

        // each switch reports its own weight when on (measured), so the sum is the channel; a plain 1 is tolerated too
        sum += (reading->second != 0) ? weight : 0;
    }

    return sum;
}

static Pro800MidiChannel resolve (int raw, std::optional<int> dipSum, Pro800MidiChannel::Kind kindForZero, bool offIsValid)
{
    Pro800MidiChannel result;
    result.raw = raw;

    if (raw == SETTINGS_MIDI_RX_ALL)
    {
        result.kind = kindForZero;
    }
    else if (raw == SETTINGS_MIDI_RX_DIPS)
    {
        result.kind = Pro800MidiChannel::Kind::DIP;
        result.channel = dipSum.has_value() ? *dipSum + 1 : 0;
    }
    else if (raw >= SETTINGS_MIDI_RX_1 && raw <= SETTINGS_MIDI_RX_16)
    {
        result.kind = Pro800MidiChannel::Kind::CHANNEL;
        result.channel = raw - SETTINGS_MIDI_RX_1 + 1;
    }
    else if (offIsValid && raw == SETTINGS_MIDI_RX_OFF)
    {
        result.kind = Pro800MidiChannel::Kind::OFF;
    }
    else
    {
        result.kind = Pro800MidiChannel::Kind::INVALID;
    }

    return result;
}

Pro800MidiChannel Pro800ChannelResolver::resolveRx (int raw, std::optional<int> dipSum)
{
    return resolve (raw, dipSum, Pro800MidiChannel::Kind::ALL, true);
}

Pro800MidiChannel Pro800ChannelResolver::resolveTx (int raw, std::optional<int> dipSum)
{
    return resolve (raw, dipSum, Pro800MidiChannel::Kind::THRU, false);
}
