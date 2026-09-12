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

#include <catch2/catch_test_macros.hpp>

#include "tailoring/Pro800ChannelResolver.h"
#include "tailoring/Pro800SettingsConstants.h"

using Kind = Pro800MidiChannel::Kind;

//==============================================================================
TEST_CASE ("Pro800ChannelResolver: the DIP switch sum is the zero-based channel", "[channel][panel]")
{
    // measured against the synth at both endpoints and two single-bit cases
    using I = Pro800PanelIndex;
    REQUIRE (Pro800ChannelResolver::dipChannelFromPanel ({ { I::DIP_1, 0 }, { I::DIP_2, 0 }, { I::DIP_3, 0 }, { I::DIP_4, 0 } }) == 0);
    REQUIRE (Pro800ChannelResolver::dipChannelFromPanel ({ { I::DIP_1, 1 }, { I::DIP_2, 0 }, { I::DIP_3, 0 }, { I::DIP_4, 0 } }) == 1);
    REQUIRE (Pro800ChannelResolver::dipChannelFromPanel ({ { I::DIP_1, 0 }, { I::DIP_2, 0 }, { I::DIP_3, 0 }, { I::DIP_4, 8 } }) == 8);
    REQUIRE (Pro800ChannelResolver::dipChannelFromPanel ({ { I::DIP_1, 1 }, { I::DIP_2, 0 }, { I::DIP_3, 0 }, { I::DIP_4, 8 } }) == 9);
    REQUIRE (Pro800ChannelResolver::dipChannelFromPanel ({ { I::DIP_1, 1 }, { I::DIP_2, 2 }, { I::DIP_3, 4 }, { I::DIP_4, 8 } }) == 15);

    SECTION ("a switch reporting 1 instead of its weight still counts with its weight")
    {
        REQUIRE (Pro800ChannelResolver::dipChannelFromPanel ({ { I::DIP_1, 0 }, { I::DIP_2, 1 }, { I::DIP_3, 0 }, { I::DIP_4, 1 } }) == 10);
    }

    SECTION ("a missing reading is reported, not treated as zero")
    {
        REQUIRE_FALSE (Pro800ChannelResolver::dipChannelFromPanel ({ { I::DIP_1, 1 }, { I::DIP_3, 4 }, { I::DIP_4, 8 } }).has_value());
        REQUIRE_FALSE (Pro800ChannelResolver::dipChannelFromPanel ({}).has_value());
    }
}

TEST_CASE ("Pro800ChannelResolver: the raw RX byte resolves to a channel, ALL, DIP, OFF or invalid", "[channel]")
{
    SECTION ("2-17 are channels 1-16")
    {
        const auto channel1 = Pro800ChannelResolver::resolveRx (SETTINGS_MIDI_RX_1, std::nullopt);
        REQUIRE (channel1.kind == Kind::CHANNEL);
        REQUIRE (channel1.channel == 1);
        REQUIRE (channel1.sendChannel() == 1);
        REQUIRE (channel1.toString() == "channel 1");

        const auto channel3 = Pro800ChannelResolver::resolveRx (4, std::nullopt); // a synth showing channel 3 reports 4
        REQUIRE (channel3.channel == 3);

        const auto channel16 = Pro800ChannelResolver::resolveRx (SETTINGS_MIDI_RX_16, std::nullopt);
        REQUIRE (channel16.kind == Kind::CHANNEL);
        REQUIRE (channel16.channel == 16);
        REQUIRE (channel16.raw == 17);
    }

    SECTION ("ALL is heard on any channel, so channel 1 is used")
    {
        const auto all = Pro800ChannelResolver::resolveRx (SETTINGS_MIDI_RX_ALL, std::nullopt);
        REQUIRE (all.kind == Kind::ALL);
        REQUIRE (all.sendChannel() == Pro800ChannelResolver::CHANNEL_FOR_ALL);
        REQUIRE (all.toString().contains ("ALL"));
    }

    SECTION ("DIP switches resolve through the panel reading")
    {
        const auto resolved = Pro800ChannelResolver::resolveRx (SETTINGS_MIDI_RX_DIPS, 2);
        REQUIRE (resolved.kind == Kind::DIP);
        REQUIRE (resolved.channel == 3);
        REQUIRE (resolved.sendChannel() == 3);
        REQUIRE (resolved.toString() == "DIP switches (channel 3)");

        const auto unread = Pro800ChannelResolver::resolveRx (SETTINGS_MIDI_RX_DIPS, std::nullopt);
        REQUIRE (unread.kind == Kind::DIP);
        REQUIRE (unread.channel == 0);
        REQUIRE_FALSE (unread.sendChannel().has_value());
        REQUIRE (unread.toString().contains ("not read"));
    }

    SECTION ("OFF and out-of-range values leave nothing to send on")
    {
        const auto off = Pro800ChannelResolver::resolveRx (SETTINGS_MIDI_RX_OFF, std::nullopt);
        REQUIRE (off.kind == Kind::OFF);
        REQUIRE_FALSE (off.sendChannel().has_value());
        REQUIRE (off.toString() == "OFF");

        for (int raw : { 19, 34, 249, 255 })
        {
            const auto invalid = Pro800ChannelResolver::resolveRx (raw, std::nullopt);
            REQUIRE (invalid.kind == Kind::INVALID);
            REQUIRE (invalid.raw == raw);
            REQUIRE_FALSE (invalid.sendChannel().has_value());
            REQUIRE (invalid.toString() == "invalid (" + std::to_string (raw) + ")");
        }
    }
}

TEST_CASE ("Pro800ChannelResolver: the raw TX byte has THRU where RX has ALL and no OFF", "[channel]")
{
    const auto thru = Pro800ChannelResolver::resolveTx (SETTINGS_MIDI_TX_THRU, std::nullopt);
    REQUIRE (thru.kind == Kind::THRU);
    REQUIRE_FALSE (thru.sendChannel().has_value());

    const auto dip = Pro800ChannelResolver::resolveTx (SETTINGS_MIDI_TX_DIPS, 15);
    REQUIRE (dip.kind == Kind::DIP);
    REQUIRE (dip.channel == 16);

    REQUIRE (Pro800ChannelResolver::resolveTx (SETTINGS_MIDI_TX_5, std::nullopt).channel == 5);

    REQUIRE (Pro800ChannelResolver::resolveTx (18, std::nullopt).kind == Kind::INVALID); // 18 means OFF for RX only
}
