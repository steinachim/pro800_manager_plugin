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

#include "helpers/FakePro800.h"
#include "helpers/MessageThread.h"
#include "helpers/TestMessages.h"

#include "midi/MidiHandler.h"
#include "session/SynthSession.h"

using namespace TestMessages;

namespace
{
    /** A handler, a fake synth behind it and a session on top; the synth starts on B43 "Strings" with channel 3 in and out. */
    struct Bench
    {
        MessageThread messageThread;
        MidiHandler midiHandler;
        FakePro800 synth { midiHandler };
        SynthSession session { midiHandler };

        Bench()
        {
            synth.setSetting (Pro800Settings::MIDI_RX_CHANNEL, SETTINGS_MIDI_RX_3);
            synth.setSetting (Pro800Settings::MIDI_TX_CHANNEL, SETTINGS_MIDI_TX_3);
            synth.setSetting (Pro800Settings::BRIGHTNESS, 5);
            synth.setSetting (Pro800Settings::PRESET_NUM, 143);
            synth.setSetting (Pro800Settings::CURRENT_BANK, 1);
            synth.storeProgram (143, "Strings");
            synth.storeProgram (105, "Brass");
            synth.storeProgram (7, "Flute");
        }

        void connect (int settleMs = 300)
        {
            session.connect();
            messageThread.runFor (settleMs);
        }
    };
}

//==============================================================================
TEST_CASE ("SynthSession: connecting reads the version, the settings and the current preset", "[session]")
{
    Bench bench;
    bench.connect();

    REQUIRE (bench.session.isConnected());
    REQUIRE (bench.session.getFirmwareVersion() == "1.4.6");
    REQUIRE (bench.session.isFirmwareSupported());

    const auto& channels = bench.session.getChannels();
    REQUIRE (channels.readFromSynth);
    REQUIRE (channels.rx.kind == Pro800MidiChannel::Kind::CHANNEL);
    REQUIRE (channels.sendChannel() == 3);
    REQUIRE (channels.receiveChannel() == 3);
    REQUIRE (bench.midiHandler.getMidiChannel() == 3);

    const auto& pointer = bench.session.getPointer();
    REQUIRE (pointer.program == 143);
    REQUIRE (pointer.name == "Strings");
    REQUIRE (pointer.freshness == SynthSession::PointerFreshness::CONFIRMED);

    // the controls are not claimed to show the sound: nothing was loaded
    REQUIRE (bench.session.getProvenance().basis == SynthSession::Provenance::Basis::UNKNOWN);
    REQUIRE_FALSE (bench.session.isBusy());
    REQUIRE (bench.session.getLastError().isEmpty());
}

TEST_CASE ("SynthSession: the DIP switches are read when a channel setting says so", "[session][channel]")
{
    Bench bench;
    bench.synth.setSetting (Pro800Settings::MIDI_RX_CHANNEL, SETTINGS_MIDI_RX_DIPS);
    bench.synth.setSetting (Pro800Settings::MIDI_TX_CHANNEL, SETTINGS_MIDI_TX_6);
    bench.synth.panel[(uint8_t) Pro800PanelIndex::DIP_1] = 1;
    bench.synth.panel[(uint8_t) Pro800PanelIndex::DIP_2] = 2; // sum 3 -> channel 4

    bench.connect();

    const auto& channels = bench.session.getChannels();
    REQUIRE (channels.rx.kind == Pro800MidiChannel::Kind::DIP);
    REQUIRE (channels.rx.channel == 4);
    REQUIRE (channels.sendChannel() == 4);
    REQUIRE (channels.receiveChannel() == 6);
    REQUIRE (bench.synth.indexOfSent (PanelMessage::REQUEST_ID, { (uint8_t) Pro800PanelIndex::DIP_4 }) >= 0);
}

TEST_CASE ("SynthSession: a synth that ignores MIDI is reported, and a manual channel overrides it", "[session][channel]")
{
    Bench bench;
    bench.synth.setSetting (Pro800Settings::MIDI_RX_CHANNEL, SETTINGS_MIDI_RX_OFF);
    bench.connect();

    REQUIRE (bench.session.getChannels().rx.kind == Pro800MidiChannel::Kind::OFF);
    REQUIRE_FALSE (bench.session.getChannels().sendChannel().has_value());
    REQUIRE (bench.session.getChannels().describe() == "Auto (OFF!)");

    bench.session.setManualChannel (9);
    REQUIRE (bench.session.getChannels().sendChannel() == 9);
    REQUIRE (bench.midiHandler.getMidiChannel() == 9);
    REQUIRE (bench.session.getChannels().describe().startsWith ("9"));
}

TEST_CASE ("SynthSession: a lost first request is sent again", "[session][exchange]")
{
    Bench bench;
    bench.synth.dropNextRequests = 2; // an idle port loses two in a row
    bench.connect (1500);

    REQUIRE (bench.session.isConnected());
    REQUIRE (bench.session.getFirmwareVersion() == "1.4.6");
}

TEST_CASE ("SynthSession: nothing answering leaves the session disconnected with an error", "[session]")
{
    Bench bench;
    bench.synth.dropNextRequests = 100;
    bench.connect (2500);

    REQUIRE_FALSE (bench.session.isConnected());
    REQUIRE (bench.session.getLastError().contains ("No Pro-800"));
    REQUIRE_FALSE (bench.session.isBusy());
}

//==============================================================================
TEST_CASE ("SynthSession: selecting a program writes the pointer, waits for it to show, then reloads", "[session][select]")
{
    Bench bench;
    bench.connect();
    bench.synth.settingsCommitLagMs = 700; // the pointer shows on the third read-back, not the first

    bench.session.selectProgram (105);
    REQUIRE (bench.session.isBusy());

    bench.messageThread.runFor (100); // the block is read, patched and written; the pointer is not readable yet
    REQUIRE (bench.session.getPointer().freshness == SynthSession::PointerFreshness::PENDING);
    REQUIRE (bench.synth.reloads == 0);

    bench.messageThread.runFor (1500);

    REQUIRE_FALSE (bench.session.isBusy());
    REQUIRE (bench.synth.reloads == 1);
    REQUIRE (bench.synth.getSetting (Pro800Settings::PRESET_NUM) == 105);
    REQUIRE (bench.synth.getSetting (Pro800Settings::CURRENT_BANK) == 1);

    // the reload went out only after the write, and after the pointer had become readable
    const int write = bench.synth.indexOfSent (Pro800DataMessage::RESPONSE_ID, { SettingsMessage::ADDRESS_LOW, SettingsMessage::ADDRESS_HIGH });
    const int reload = bench.synth.indexOfSent (ReloadMessage::REQUEST_ID);
    REQUIRE (write >= 0);
    REQUIRE (reload > write);

    const auto& pointer = bench.session.getPointer();
    REQUIRE (pointer.program == 105);
    REQUIRE (pointer.name == "Brass");
    REQUIRE (pointer.freshness == SynthSession::PointerFreshness::CONFIRMED);

    const auto& provenance = bench.session.getProvenance();
    REQUIRE (provenance.basis == SynthSession::Provenance::Basis::STORED);
    REQUIRE (provenance.program == 105);
    REQUIRE_FALSE (provenance.isEdited());
    REQUIRE (provenance.describe() == "B05 as stored");
}

TEST_CASE ("SynthSession: a selection the synth never shows is reported and not reloaded", "[session][select]")
{
    Bench bench;
    bench.connect();
    bench.synth.ignoreSettingsWrites = true;

    bench.session.selectProgram (105);
    bench.messageThread.runFor (3500);

    REQUIRE_FALSE (bench.session.isBusy());
    REQUIRE (bench.synth.reloads == 0);
    REQUIRE (bench.session.getPointer().freshness == SynthSession::PointerFreshness::UNCONFIRMED);
    REQUIRE (bench.session.getLastError().contains ("B05"));
}

TEST_CASE ("SynthSession: reverting reloads and shows the stored record", "[session]")
{
    Bench bench;
    bench.connect();

    bench.session.reloadStoredProgram();
    bench.messageThread.runFor (300);

    REQUIRE (bench.synth.reloads == 1);
    REQUIRE (bench.session.getProvenance().basis == SynthSession::Provenance::Basis::STORED);
    REQUIRE (bench.session.getProvenance().program == 143);
}

TEST_CASE ("SynthSession: a preset selected on the synth is followed, and counts as the stored sound", "[session]")
{
    Bench bench;
    bench.connect();

    bench.synth.setSetting (Pro800Settings::PRESET_NUM, 7);
    bench.synth.setSetting (Pro800Settings::CURRENT_BANK, 0);
    bench.messageThread.runFor (SynthSession::POLL_INTERVAL_MS + 400);

    REQUIRE (bench.session.getPointer().program == 7);
    REQUIRE (bench.session.getPointer().name == "Flute");
    REQUIRE (bench.session.getProvenance().basis == SynthSession::Provenance::Basis::STORED);
    REQUIRE (bench.session.getProvenance().program == 7);
}

TEST_CASE ("SynthSession: edits are noted from what the plugin sends and what the synth reports", "[session]")
{
    Bench bench;
    bench.connect();
    bench.session.reloadStoredProgram();
    bench.messageThread.runFor (300);
    REQUIRE_FALSE (bench.session.getProvenance().isEdited());

    bench.midiHandler.sendMidiCCMessage (Pro800CCMessages::FILTER_CUTOFF, 100);
    REQUIRE (bench.session.getProvenance().editedInApp);
    REQUIRE (bench.session.getProvenance().describe() == "B43 + edits (in app)");

    // the synth's knob movement arrives on its TX channel; the echo of our own CC is not an edit
    bench.midiHandler.handleIncomingMidiMessage (nullptr, juce::MidiMessage::controllerEvent (3, 15, 100));
    bench.midiHandler.handleIncomingMidiMessage (nullptr, juce::MidiMessage::controllerEvent (3, 16, 40));
    bench.messageThread.runFor (50);
    REQUIRE (bench.session.getProvenance().editedOnSynth);
    REQUIRE (bench.session.getProvenance().describe() == "B43 + edits (in app and on synth)");
}

//==============================================================================
TEST_CASE ("SynthSession: a run of setting changes becomes one write, kept against a stale read-back", "[session][settings]")
{
    Bench bench;
    bench.connect();
    bench.synth.settingsCommitLagMs = 1000;
    const int writesBefore = bench.synth.settingsWrites;

    bench.session.writeSetting (Pro800Settings::BRIGHTNESS, 7);
    bench.session.writeSetting (Pro800Settings::BRIGHTNESS, 8);
    bench.session.writeSetting (Pro800Settings::BRIGHTNESS, 9);
    REQUIRE (bench.session.getSettings()->getValue (Pro800Settings::BRIGHTNESS) == 9);
    REQUIRE (bench.session.getSettingsWriteStatus().state == SynthSession::SettingsWriteStatus::State::SAVING);
    REQUIRE (bench.session.getSettingsWriteStatus().message.contains ("Brightness"));

    bench.messageThread.runFor (SynthSession::WRITE_DEBOUNCE_MS + 100);
    REQUIRE (bench.synth.settingsWrites == writesBefore + 1);
    REQUIRE (SettingsMessage (toMidi (bench.synth.received.back().size() > 20 ? bench.synth.received.back() : bench.synth.settings)).isValid());

    // inside the synth's commit lag every read-back still shows 5; the block must not fall back to it
    bench.messageThread.runFor (500);
    REQUIRE (bench.synth.getSetting (Pro800Settings::BRIGHTNESS) == 5);
    REQUIRE (bench.session.getSettings()->getValue (Pro800Settings::BRIGHTNESS) == 9);
    REQUIRE (bench.session.getSettingsWriteStatus().state == SynthSession::SettingsWriteStatus::State::SAVING);

    // once it has landed, the write is confirmed
    bench.messageThread.runFor (1200);
    REQUIRE (bench.synth.getSetting (Pro800Settings::BRIGHTNESS) == 9);
    REQUIRE (bench.session.getSettings()->getValue (Pro800Settings::BRIGHTNESS) == 9);
    REQUIRE (bench.session.getSettingsWriteStatus().state == SynthSession::SettingsWriteStatus::State::IDLE);
    REQUIRE (bench.session.getSettingsWriteStatus().message.isEmpty());
}

TEST_CASE ("SynthSession: a setting the synth never takes is undone and reported", "[session][settings]")
{
    Bench bench;
    bench.connect();
    bench.synth.ignoreSettingsWrites = true;

    bench.session.writeSetting (Pro800Settings::BRIGHTNESS, 12);
    bench.messageThread.runFor (SynthSession::WRITE_DEBOUNCE_MS + SynthSession::WRITE_CONFIRM_WINDOW_MS + 800);

    const auto& status = bench.session.getSettingsWriteStatus();
    REQUIRE (status.state == SynthSession::SettingsWriteStatus::State::FAILED);
    REQUIRE (status.message.contains ("Brightness = 12"));
    REQUIRE (status.message.contains ("reads back 5"));
    REQUIRE (bench.session.getSettings()->getValue (Pro800Settings::BRIGHTNESS) == 5); // what the synth shows
}

TEST_CASE ("SynthSession: a change made on the synth's front panel is noticed", "[session][settings]")
{
    Bench bench;
    bench.connect();

    bench.synth.setSetting (Pro800Settings::TRANSPOSE, -5);
    bench.messageThread.runFor (SynthSession::POLL_INTERVAL_MS + 400);

    REQUIRE (bench.session.getSettings()->getValue (Pro800Settings::TRANSPOSE) == -5);
    const auto& changed = bench.session.getSettingsWriteStatus().changedOnSynth;
    REQUIRE (changed.size() == 1);
    REQUIRE (changed.front() == Pro800Settings::TRANSPOSE);
}

TEST_CASE ("SynthSession: a select and a settings change in flight do not undo each other", "[session][settings][select]")
{
    Bench bench;
    bench.connect();
    bench.synth.settingsCommitLagMs = 600;

    bench.session.selectProgram (105);
    bench.messageThread.runFor (100);
    bench.session.writeSetting (Pro800Settings::BRIGHTNESS, 11); // while the pointer write has not committed yet
    bench.messageThread.runFor (2500);

    REQUIRE (bench.synth.getSetting (Pro800Settings::PRESET_NUM) == 105);
    REQUIRE (bench.synth.getSetting (Pro800Settings::BRIGHTNESS) == 11);
    REQUIRE (bench.session.getPointer().program == 105);
    REQUIRE (bench.session.getSettings()->getValue (Pro800Settings::BRIGHTNESS) == 11);
}
