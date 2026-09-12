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

#include "midi/LiveParameterMessage.h"
#include "midi/Pro800MessageFactory.h"
#include "midi/SysExMatchers.h"
#include "tailoring/Pro800PanelState.h"

using namespace TestMessages;

//==============================================================================
TEST_CASE ("LiveParameterMessage: a 0x73 reply carries the index and the 7-bit value", "[midi][live]")
{
    LiveParameterMessage cutoff (toMidi (liveReply (0x19, 93)));
    REQUIRE (cutoff.isValid());
    REQUIRE (cutoff.getIndex() == 0x19);
    REQUIRE (cutoff.getValue() == 93);
    REQUIRE (cutoff.getMessageType() == MessageType::PRO800_LIVE_PARAMETER);
    REQUIRE (cutoff.getIndexName() == "Filter Cutoff");
    REQUIRE (cutoff.toString().contains ("Filter Cutoff = 93"));

    REQUIRE (bytesOf (LiveParameterMessage::request (Pro800LiveIndex::FILTER_CUTOFF)) == sysEx ({ 0x72, 0x19 }));
    REQUIRE (bytesOf (LiveParameterMessage::writeRequest (Pro800LiveIndex::AMP_RELEASE, 40)) == sysEx ({ 0x73, 0x00, 40 }));
    REQUIRE (bytesOf (LiveParameterMessage::writeRequest (Pro800LiveIndex::AMP_RELEASE, 200)) == sysEx ({ 0x73, 0x00, 0x48 })); // kept 7-bit

    LiveParameterMessage truncated (toMidi (sysEx ({ LiveParameterMessage::RESPONSE_ID, 0x19 })));
    REQUIRE_FALSE (truncated.isValid());
    REQUIRE (truncated.getValue() == 0);

    REQUIRE (Pro800MessageFactory::createMidiMessage (toMidi (liveReply (0x00, 1)))->getMessageType() == MessageType::PRO800_LIVE_PARAMETER);

    REQUIRE (SysExMatchers::isLiveReplyFor (toMidi (liveReply (0x05, 7)), 0x05));
    REQUIRE (SysExMatchers::isLiveReplyFor (toMidi (statusReply (0x01)), 0x1A)); // refused index
    REQUIRE_FALSE (SysExMatchers::isLiveReplyFor (toMidi (liveReply (0x06, 7)), 0x05));
    REQUIRE_FALSE (SysExMatchers::isLiveReplyFor (toMidi (panelReply (0x05, 7)), 0x05));

    for (uint8_t index = 0; index < (uint8_t) Pro800LiveIndex::NUM_INDICES; index++)
    {
        REQUIRE (PRO800_LIVE_INDEX_NAMES.count (static_cast<Pro800LiveIndex> (index)) == 1);
    }
}

//==============================================================================
TEST_CASE ("Pro800PanelConversion: the two Filter Keyboard bits are one 3-position switch", "[panel]")
{
    REQUIRE (Pro800PanelConversion::filterKeyboardFromBits (0, 0) == PROGRAM_FILTER_KEYBOARD_TRACKING_OFF);
    REQUIRE (Pro800PanelConversion::filterKeyboardFromBits (0, 1) == PROGRAM_FILTER_KEYBOARD_TRACKING_HALF);
    REQUIRE (Pro800PanelConversion::filterKeyboardFromBits (1, 0) == PROGRAM_FILTER_KEYBOARD_TRACKING_FULL);
    REQUIRE_FALSE (Pro800PanelConversion::filterKeyboardFromBits (1, 1).has_value());
}

TEST_CASE ("Pro800PanelConversion: the shape switch picks within the pair the previous shape names", "[panel]")
{
    // the switch has two sides - Tri/Sine/Saw and Pulse/Random/Noise - and the Performance menu picks which pair
    // it toggles: Tri/Pulse, Sine/Random, Saw/Noise. Pro800ProgramLfoShape is ordered in exactly those pairs.
    constexpr int TRI_SINE_SAW = 1;
    constexpr int PULSE_RANDOM_NOISE = 0;

    SECTION ("a shape already on the switch's side is kept")
    {
        REQUIRE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_TRIANGLE, TRI_SINE_SAW) == PROGRAM_LFO_SHAPE_TRIANGLE);
        REQUIRE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_SINE, TRI_SINE_SAW) == PROGRAM_LFO_SHAPE_SINE);
        REQUIRE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_SAW, TRI_SINE_SAW) == PROGRAM_LFO_SHAPE_SAW);
        REQUIRE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_PULSE, PULSE_RANDOM_NOISE) == PROGRAM_LFO_SHAPE_PULSE);
        REQUIRE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_RANDOM, PULSE_RANDOM_NOISE) == PROGRAM_LFO_SHAPE_RANDOM);
        REQUIRE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_NOISE, PULSE_RANDOM_NOISE) == PROGRAM_LFO_SHAPE_NOISE);
    }

    SECTION ("a shape on the other side flips to its partner, and back again")
    {
        REQUIRE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_TRIANGLE, PULSE_RANDOM_NOISE) == PROGRAM_LFO_SHAPE_PULSE);
        REQUIRE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_PULSE, TRI_SINE_SAW) == PROGRAM_LFO_SHAPE_TRIANGLE);
        REQUIRE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_SINE, PULSE_RANDOM_NOISE) == PROGRAM_LFO_SHAPE_RANDOM);
        REQUIRE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_RANDOM, TRI_SINE_SAW) == PROGRAM_LFO_SHAPE_SINE);
        REQUIRE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_SAW, PULSE_RANDOM_NOISE) == PROGRAM_LFO_SHAPE_NOISE);
        REQUIRE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_NOISE, TRI_SINE_SAW) == PROGRAM_LFO_SHAPE_SAW);
    }

    SECTION ("every shape stays within its own pair, whatever the switch says")
    {
        for (int shape = 0; shape < PROGRAM_LFO_SHAPE_NUM_VALUES; shape++)
        {
            for (int position : { 0, 1 })
            {
                const auto resolved = Pro800PanelConversion::lfoShapeFromSwitch (shape, position);
                REQUIRE (resolved.has_value());
                REQUIRE ((*resolved / 2) == (shape / 2)); // the pair
                REQUIRE ((*resolved % 2) == position); // the side the switch is on
            }
        }
    }

    SECTION ("without a previous shape the pair is unknown, so nothing is resolved")
    {
        REQUIRE_FALSE (Pro800PanelConversion::lfoShapeFromSwitch (std::nullopt, TRI_SINE_SAW).has_value());
        REQUIRE_FALSE (Pro800PanelConversion::lfoShapeFromSwitch (-1, TRI_SINE_SAW).has_value());
        REQUIRE_FALSE (Pro800PanelConversion::lfoShapeFromSwitch (PROGRAM_LFO_SHAPE_NUM_VALUES, TRI_SINE_SAW).has_value());
    }
}

TEST_CASE ("Pro800PanelConversion: a 7-bit knob reading lands on the stored record's 16-bit scale", "[panel]")
{
    REQUIRE (Pro800PanelConversion::liveValueToProgramRange (0) == 0);
    REQUIRE (Pro800PanelConversion::liveValueToProgramRange (127) == 65535);
    REQUIRE (Pro800PanelConversion::liveValueToProgramRange (64) == 33026); // 64 * 65535 / 127 = 33025.5, rounded
    REQUIRE (Pro800PanelConversion::liveValueToProgramRange (200) == 65535); // clamped
    REQUIRE (Pro800PanelConversion::liveValueToProgramRange (-3) == 0);
}

TEST_CASE ("Pro800PanelConversion: every knob has a CC, and all but the two master knobs a program field", "[panel]")
{
    int fields = 0;
    for (uint8_t i = 0; i < (uint8_t) Pro800LiveIndex::NUM_INDICES; i++)
    {
        const auto index = static_cast<Pro800LiveIndex> (i);
        REQUIRE (Pro800PanelConversion::liveIndexToCC (index) != Pro800CCMessages::NONE); // every knob has a CC
        fields += Pro800PanelConversion::liveIndexToProgramField (index) != Pro800ProgramField::NONE ? 1 : 0;
    }
    REQUIRE (fields == 24); // all but Master Tune and Master Volume, which no preset field holds
    REQUIRE (Pro800PanelConversion::liveIndexToCC (Pro800LiveIndex::MASTER_TUNE) == Pro800CCMessages::MASTER_TUNE);
    REQUIRE (Pro800PanelConversion::liveIndexToProgramField (Pro800LiveIndex::FILTER_CUTOFF) == Pro800ProgramField::FILTER_CUTOFF);
    REQUIRE (Pro800PanelConversion::liveIndexToProgramField (Pro800LiveIndex::GLIDE_TIME) == Pro800ProgramField::GLIDE_AMOUNT);
}

TEST_CASE ("Pro800PanelConversion: a reading becomes program fields and CC values, the LFO shape left out", "[panel]")
{
    Pro800PanelState state;
    using P = Pro800PanelIndex;
    using L = Pro800LiveIndex;
    state.panel = { { P::SWITCH_OSC_A_SHAPE_SAW, 1 }, { P::SWITCH_OSC_A_SHAPE_TRI, 0 }, { P::SWITCH_OSC_A_SYNC, 1 }, { P::SWITCH_LFO_SHAPE, 1 }, { P::SWITCH_LFO_DEST_FREQ_AB, 1 }, { P::SWITCH_LFO_DEST_PW_AB, 0 }, { P::SWITCH_LFO_DEST_FILTER, 1 }, { P::SWITCH_FILTER_KEYBOARD_FULL, 0 }, { P::SWITCH_FILTER_KEYBOARD_HALF, 1 }, { P::DIP_1, 1 }, { P::BUTTON_PRESET, 1 } };
    state.live = { { L::FILTER_CUTOFF, 127 }, { L::AMP_RELEASE, 0 }, { L::MASTER_TUNE, 64 }, { L::MASTER_VOLUME, 100 } };

    const auto values = Pro800PanelConversion::toValues (state, PROGRAM_LFO_SHAPE_PULSE);

    REQUIRE (values.fields.at (Pro800ProgramField::OSC_A_SHAPE_SAW) == 1);
    REQUIRE (values.fields.at (Pro800ProgramField::OSC_A_SHAPE_TRI) == 0);
    REQUIRE (values.fields.at (Pro800ProgramField::OSC_A_SYNC) == 1);
    REQUIRE (values.fields.at (Pro800ProgramField::LFO_DEST) == (PROGRAM_LFO_DEST_FREQ_AB | PROGRAM_LFO_DEST_FILTER));
    REQUIRE (values.fields.at (Pro800ProgramField::FILTER_KEY_TRACKING) == PROGRAM_FILTER_KEYBOARD_TRACKING_HALF);
    REQUIRE (values.fields.at (Pro800ProgramField::FILTER_CUTOFF) == 65535);
    REQUIRE (values.fields.at (Pro800ProgramField::AMP_RELEASE) == 0);
    REQUIRE (values.ccValues.at (Pro800CCMessages::MASTER_TUNE) == 64);
    REQUIRE (values.ccValues.at (Pro800CCMessages::MASTER_VOLUME) == 100);

    REQUIRE (values.fields.at (Pro800ProgramField::LFO_SHAPE) == PROGRAM_LFO_SHAPE_TRIANGLE); // Pulse, switched to the other side of its pair
    REQUIRE (values.fields.count (Pro800ProgramField::OSC_B_SHAPE_SAW) == 0); // not read
    REQUIRE (values.fields.size() == 8);
    REQUIRE (values.ccValues.size() == 2);

    SECTION ("the LFO shape is left out when no previous shape names its pair")
    {
        REQUIRE (Pro800PanelConversion::toValues (state).fields.count (Pro800ProgramField::LFO_SHAPE) == 0);
    }

    SECTION ("the LFO destination needs all three switches")
    {
        state.panel.erase (P::SWITCH_LFO_DEST_PW_AB);
        REQUIRE (Pro800PanelConversion::toValues (state, PROGRAM_LFO_SHAPE_PULSE).fields.count (Pro800ProgramField::LFO_DEST) == 0);
    }

    SECTION ("an impossible Filter Keyboard reading is left out")
    {
        state.panel[P::SWITCH_FILTER_KEYBOARD_FULL] = 1;
        REQUIRE (Pro800PanelConversion::toValues (state, PROGRAM_LFO_SHAPE_PULSE).fields.count (Pro800ProgramField::FILTER_KEY_TRACKING) == 0);
    }
}

TEST_CASE ("Pro800PanelConversion: a reading becomes the CC values that make the synth play it", "[panel]")
{
    Pro800PanelState state;
    using P = Pro800PanelIndex;
    using L = Pro800LiveIndex;
    state.panel = { { P::SWITCH_OSC_A_SYNC, 1 }, { P::SWITCH_OSC_A_SHAPE_SAW, 0 }, { P::SWITCH_LFO_DEST_FILTER, 1 }, { P::SWITCH_FILTER_KEYBOARD_FULL, 1 }, { P::SWITCH_FILTER_KEYBOARD_HALF, 0 }, { P::SWITCH_LFO_SHAPE, 0 }, { P::BUTTON_REC, 1 } };
    state.live = { { L::FILTER_CUTOFF, 127 }, { L::MASTER_VOLUME, 100 }, { L::LFO_INITIAL_AMOUNT, 0 } };

    const auto ccValues = Pro800PanelConversion::toCCValues (state, PROGRAM_LFO_SHAPE_SAW);

    // knob readings are 7-bit already, so they are the CC value unchanged
    REQUIRE (ccValues.at (Pro800CCMessages::FILTER_CUTOFF) == 127);
    REQUIRE (ccValues.at (Pro800CCMessages::MASTER_VOLUME) == 100);
    REQUIRE (ccValues.at (Pro800CCMessages::LFO_MOD_INITIAL_AMOUNT) == 0);

    REQUIRE (ccValues.at (Pro800CCMessages::OSC_A_SYNC) == CC_ON);
    REQUIRE (ccValues.at (Pro800CCMessages::OSC_A_SHAPE_SAW) == CC_OFF);
    REQUIRE (ccValues.at (Pro800CCMessages::LFO_MOD_DEST_FILTER) == CC_ON);
    REQUIRE (ccValues.at (Pro800CCMessages::FILTER_KEYBOARD_TRACKING) == 86); // Full, per docs/Pro800CCMessages.md
    REQUIRE (ccValues.at (Pro800CCMessages::LFO_MOD_SHAPE) == 88); // Saw's partner Noise, the switch being on the other side

    REQUIRE (ccValues.size() == 8); // a held button is not a control value
}
