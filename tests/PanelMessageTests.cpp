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

#include "midi/PanelMessage.h"
#include "midi/Pro800MessageFactory.h"
#include "midi/ReloadMessage.h"
#include "tailoring/Pro800PanelConstants.h"

using namespace TestMessages;

//==============================================================================
TEST_CASE ("PanelMessage: a 0x71 reply carries the index and the value", "[midi][panel]")
{
    PanelMessage oscASync (toMidi (panelReply (0x22, 1)));
    REQUIRE (oscASync.isValid());
    REQUIRE (oscASync.getIndex() == 0x22);
    REQUIRE (oscASync.getValue() == 1);
    REQUIRE (oscASync.getMessageType() == MessageType::PRO800_PANEL);
    REQUIRE (oscASync.getIndexName() == "Osc A Sync");
    REQUIRE (oscASync.toString().contains ("Osc A Sync = 1"));

    PanelMessage dip4 (toMidi (panelReply (0x2B, 8)));
    REQUIRE (dip4.getValue() == 8);
    REQUIRE (dip4.getIndexName() == "DIP switch 4");
}

TEST_CASE ("PanelMessage: the request is 0x70 with the index", "[midi][panel]")
{
    REQUIRE (bytesOf (PanelMessage::request (Pro800PanelIndex::DIP_1)) == sysEx ({ PanelMessage::REQUEST_ID, 0x28 }));
    REQUIRE (bytesOf (PanelMessage::request ((uint8_t) 0x19)) == sysEx ({ PanelMessage::REQUEST_ID, 0x19 }));
}

TEST_CASE ("PanelMessage: a reply without the value byte is invalid", "[midi][panel]")
{
    // F0 <header> 71 22 F7: index 10 (the value) is the F7 - this is also what a sent button press looks like
    PanelMessage truncated (toMidi (sysEx ({ PanelMessage::RESPONSE_ID, 0x22 })));
    REQUIRE_FALSE (truncated.isValid());
    REQUIRE (truncated.getValue() == 0);
    REQUIRE_NOTHROW (truncated.toString());

    PanelMessage wrongType (toMidi (statusReply (0x00)));
    REQUIRE_FALSE (wrongType.isValid());
}

TEST_CASE ("PanelMessage: an index without a name is reported in hex", "[midi][panel]")
{
    PanelMessage unknown (toMidi (panelReply (0x2C, 0)));
    REQUIRE (unknown.isValid());
    REQUIRE (unknown.getIndexName().contains ("2c"));
}

TEST_CASE ("PanelMessage: every index below the ceiling has a name", "[midi][panel]")
{
    for (uint8_t index = 0; index < (uint8_t) Pro800PanelIndex::NUM_INDICES; index++)
    {
        REQUIRE (PRO800_PANEL_INDEX_NAMES.count (static_cast<Pro800PanelIndex> (index)) == 1);
    }
    REQUIRE (PRO800_PANEL_INDEX_NAMES.size() == (size_t) Pro800PanelIndex::NUM_INDICES);
}

TEST_CASE ("Pro800MessageFactory: a 0x71 reply becomes a PanelMessage", "[midi][factory][panel]")
{
    const auto message = Pro800MessageFactory::createMidiMessage (toMidi (panelReply (0x1A, 1)));
    REQUIRE (message != nullptr);
    REQUIRE (message->getMessageType() == MessageType::PRO800_PANEL);
}

//==============================================================================
TEST_CASE ("ReloadMessage: the request is 0x32 with parameter 0", "[midi]")
{
    REQUIRE (bytesOf (ReloadMessage::request()) == sysEx ({ 0x32, 0x00 }));
}
