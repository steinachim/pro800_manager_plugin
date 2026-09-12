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

#include "helpers/TestMessages.h"

#include "midi/SettingsMessage.h"
#include "midi/SysExMatchers.h"
#include "midi/VersionMessage.h"

using namespace TestMessages;

//==============================================================================
TEST_CASE ("SysExMatchers: a program read is answered by its own dump, a bare F0 F7 or a status", "[midi][exchange]")
{
    REQUIRE (SysExMatchers::isDumpReplyFor (toMidi (programDump (300)), 300));
    REQUIRE (SysExMatchers::isDumpReplyFor (toMidi (emptySlotReply()), 300));
    REQUIRE (SysExMatchers::isDumpReplyFor (toMidi (statusReply (0x01)), 300));
    REQUIRE (SysExMatchers::isDumpReplyFor (toMidi (settingsDump()), SettingsMessage::ADDRESS));

    SECTION ("the address must be the one asked for")
    {
        REQUIRE_FALSE (SysExMatchers::isDumpReplyFor (toMidi (programDump (301)), 300));
        REQUIRE_FALSE (SysExMatchers::isDumpReplyFor (toMidi (programDump (5)), 300));
        REQUIRE_FALSE (SysExMatchers::isDumpReplyFor (toMidi (settingsDump()), 300));
    }

    SECTION ("other message types are not accepted")
    {
        REQUIRE_FALSE (SysExMatchers::isDumpReplyFor (juce::MidiMessage::controllerEvent (1, 7, 100), 300));
        REQUIRE_FALSE (SysExMatchers::isDumpReplyFor (toMidi (panelReply (0x22, 1)), 300));
        REQUIRE_FALSE (SysExMatchers::isDumpReplyFor (toMidi (sysEx ({ VersionMessage::RESPONSE_ID, 0x00, 1, 4, 6 })), 300));
        REQUIRE_FALSE (SysExMatchers::isDumpReplyFor (toMidi (sysEx ({ Pro800DataMessage::RESPONSE_ID, 0x2C })), 300)); // address cut off
    }
}

TEST_CASE ("SysExMatchers: a dump longer than its preset version allows is not the synth's reply", "[midi][exchange]")
{
    // a format-109 record ends with the name and can have at most 190 payload bytes
    REQUIRE_FALSE (SysExMatchers::outrunsDeclaredVersion (toMidi (programDump (20, 109, 190))));
    REQUIRE (SysExMatchers::outrunsDeclaredVersion (toMidi (programDump (20, 109, 191))));
    REQUIRE_FALSE (SysExMatchers::isDumpReplyFor (toMidi (programDump (20, 109, 231)), 20));

    REQUIRE_FALSE (SysExMatchers::outrunsDeclaredVersion (toMidi (programDump (20, 110, 192))));
    REQUIRE (SysExMatchers::outrunsDeclaredVersion (toMidi (programDump (20, 110, 193))));

    REQUIRE_FALSE (SysExMatchers::outrunsDeclaredVersion (toMidi (programDump (20, 111, 198))));
    REQUIRE (SysExMatchers::outrunsDeclaredVersion (toMidi (programDump (20, 111, 199))));

    SECTION ("an unknown version byte, a short record and the settings block are not this check's business")
    {
        REQUIRE_FALSE (SysExMatchers::outrunsDeclaredVersion (toMidi (programDump (20, 112, 230))));
        REQUIRE_FALSE (SysExMatchers::outrunsDeclaredVersion (toMidi (sysEx ({ Pro800DataMessage::RESPONSE_ID, 0x14, 0x00, 0x01 }))));
        REQUIRE_FALSE (SysExMatchers::outrunsDeclaredVersion (toMidi (settingsDump())));
        REQUIRE_FALSE (SysExMatchers::outrunsDeclaredVersion (toMidi (statusReply (0x00))));
    }
}

TEST_CASE ("SysExMatchers: a panel read is answered by a 0x71 with the same index or a status", "[midi][exchange][panel]")
{
    REQUIRE (SysExMatchers::isPanelReplyFor (toMidi (panelReply (0x28, 1)), 0x28));
    REQUIRE (SysExMatchers::isPanelReplyFor (toMidi (statusReply (0x01)), 0x2C)); // an index the synth refuses

    REQUIRE_FALSE (SysExMatchers::isPanelReplyFor (toMidi (panelReply (0x29, 1)), 0x28));
    REQUIRE_FALSE (SysExMatchers::isPanelReplyFor (toMidi (sysEx ({ PanelMessage::RESPONSE_ID, 0x28 })), 0x28)); // no value
    REQUIRE_FALSE (SysExMatchers::isPanelReplyFor (toMidi (emptySlotReply()), 0x28));
    REQUIRE_FALSE (SysExMatchers::isPanelReplyFor (toMidi (programDump (3)), 0x28));
}

TEST_CASE ("SysExMatchers: status, version and empty-slot replies", "[midi][exchange]")
{
    REQUIRE (SysExMatchers::isStatusReply (toMidi (statusReply (0x00))));
    REQUIRE (SysExMatchers::isStatusReply (toMidi (sysEx ({ StatusMessage::RESPONSE_ID }))));
    REQUIRE_FALSE (SysExMatchers::isStatusReply (toMidi (emptySlotReply())));
    REQUIRE_FALSE (SysExMatchers::isStatusReply (juce::MidiMessage::noteOn (1, 60, 1.0f)));

    REQUIRE (SysExMatchers::isVersionReply (toMidi (sysEx ({ VersionMessage::RESPONSE_ID, 0x00, 1, 4, 6 }))));
    REQUIRE_FALSE (SysExMatchers::isVersionReply (toMidi (statusReply (0x00))));

    REQUIRE (SysExMatchers::isEmptySlotReply (toMidi (emptySlotReply())));
    REQUIRE_FALSE (SysExMatchers::isEmptySlotReply (toMidi (sysEx ({}))));

    REQUIRE (SysExMatchers::pro800Type (toMidi (statusReply (0x00))) == StatusMessage::RESPONSE_ID);
    REQUIRE_FALSE (SysExMatchers::pro800Type (toMidi (emptySlotReply())).has_value());
    REQUIRE_FALSE (SysExMatchers::pro800Type (toMidi ({ 0xF0, 0x00, 0x20, 0x33, 0x00, 0x01, 0x24, 0x00, 0x01, 0xF7 })).has_value());
}
