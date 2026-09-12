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
#include "tailoring/Pro800PanelState.h"
#include "ui/MidiComponent.h"

using namespace TestMessages;

namespace
{
    /** Collects what the code logs, so that a test can say what should and should not end up in it. */
    struct CapturedLog : private juce::Logger
    {
        CapturedLog() { juce::Logger::setCurrentLogger (this); }
        ~CapturedLog() override { juce::Logger::setCurrentLogger (nullptr); }

        int countContaining (const juce::String& text) const
        {
            int count = 0;
            for (const auto& line : lines)
            {
                count += line.contains (text) ? 1 : 0;
            }
            return count;
        }

        juce::StringArray lines;

    private:
        void logMessage (const juce::String& message) override { lines.add (message); }
    };

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
            synth.setSetting (Pro800Settings::MIDI_CC_MODE, SETTINGS_MIDI_MODE_TX_RX);
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

TEST_CASE ("SynthSession: a synth that stops answering ends the connection instead of being polled forever", "[session]")
{
    Bench bench;
    CapturedLog log;
    bench.connect();
    REQUIRE (bench.session.isConnected());

    // unplugged, powered off, or the port taken by something else
    bench.synth.dropNextRequests = 1000;
    const size_t sentAfterConnect = bench.synth.received.size();

    bench.messageThread.runFor (7000); // three polls, each two attempts

    REQUIRE_FALSE (bench.session.isConnected());
    REQUIRE (bench.session.getLastError().contains ("stopped answering"));
    REQUIRE (bench.session.getPointer().program == std::nullopt);
    REQUIRE_FALSE (bench.session.getChannels().readFromSynth);

    // and it stays quiet: no more polls once it has given up
    const size_t sentWhenGivenUp = bench.synth.received.size();
    REQUIRE (sentWhenGivenUp > sentAfterConnect);
    bench.messageThread.runFor (4000);
    REQUIRE (bench.synth.received.size() == sentWhenGivenUp);

    SECTION ("the polls that failed on the way there did not fill the log")
    {
        // three polls, each sent twice - and not one of them is worth a line, because they repeat on a timer
        REQUIRE (log.countContaining ("No reply to settings request") == 0);

        // what the silence means is said once, by whoever owns the poll
        REQUIRE (log.countContaining ("stopped answering") == 1);
    }

    SECTION ("a request the user asked for does say when it goes unanswered")
    {
        log.lines.clear();
        bench.session.connect(); // the identity probe is nobody's routine background traffic
        bench.messageThread.runFor (4000);

        REQUIRE (log.countContaining ("No reply to firmware version request") > 0);
    }

    SECTION ("connecting again picks it back up")
    {
        bench.synth.dropNextRequests = 0;
        bench.connect (500);
        REQUIRE (bench.session.isConnected());
        REQUIRE (bench.session.getPointer().program == 143);
    }
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

//==============================================================================
namespace
{
    /** A component with one slider per knob kind, like the Front Panel tab: what a panel reading has to fill. */
    struct PanelMirror : public MidiComponent
    {
        PanelMirror (MidiHandler& handler, SynthSession& session) : MidiComponent (&handler, session, true)
        {
            cutoff.setRange (0, 65535, 1);
            masterVolume.setRange (0, 127, 1);
            setupMidiComponent (&cutoff, Pro800CCMessages::FILTER_CUTOFF, Pro800ProgramField::FILTER_CUTOFF);
            setupMidiComponent (&masterVolume, Pro800CCMessages::MASTER_VOLUME, Pro800ProgramField::NONE);
            setupMidiComponent (&oscASync, Pro800CCMessages::OSC_A_SYNC, Pro800ProgramField::OSC_A_SYNC);
            setupMidiComponent (&lfoDestFilter, Pro800CCMessages::LFO_MOD_DEST_FILTER, Pro800ProgramField::LFO_DEST);
            setupMidiComponent (&lfoShape, Pro800CCMessages::LFO_MOD_SHAPE, Pro800ProgramField::LFO_SHAPE);
            for (int shape = 0; shape < PROGRAM_LFO_SHAPE_NUM_VALUES; shape++)
            {
                lfoShape.addItem (juce::String (shape), shape + MidiComponent::COMBO_BOX_ID_OFFSET);
            }
        }

        juce::Slider cutoff, masterVolume;
        juce::ToggleButton oscASync, lfoDestFilter;
        juce::ComboBox lfoShape;
    };
}

TEST_CASE ("SynthSession: aligning sends the panel to the synth and shows it here", "[session][panel]")
{
    Bench bench;
    PanelMirror mirror (bench.midiHandler, bench.session);
    bench.connect();

    bench.synth.knobs[(uint8_t) Pro800LiveIndex::FILTER_CUTOFF] = 127;
    bench.synth.knobs[(uint8_t) Pro800LiveIndex::MASTER_VOLUME] = 100;
    bench.synth.panel[(uint8_t) Pro800PanelIndex::SWITCH_OSC_A_SYNC] = 1;
    bench.synth.panel[(uint8_t) Pro800PanelIndex::SWITCH_LFO_DEST_FILTER] = 1;

    REQUIRE (mirror.cutoff.getAlpha() < 1.0f); // unknown until something sets it

    bench.session.alignWithPanel();
    REQUIRE (bench.session.isBusy());
    bench.messageThread.runFor (800);

    REQUIRE_FALSE (bench.session.isBusy());
    REQUIRE (bench.session.getLastPanelState().has_value());
    REQUIRE (bench.session.getLastPanelState()->live.size() == (size_t) Pro800LiveIndex::NUM_INDICES);
    REQUIRE (bench.session.getLastPanelState()->panel.size() == (size_t) Pro800PanelIndex::NUM_INDICES);

    SECTION ("the panel went to the synth as CC, on its own channel")
    {
        const auto ccs = bench.synth.receivedCCs();
        REQUIRE (ccs.at ((int) Pro800CCMessages::FILTER_CUTOFF) == 127);
        REQUIRE (ccs.at ((int) Pro800CCMessages::MASTER_VOLUME) == 100);
        REQUIRE (ccs.at ((int) Pro800CCMessages::AMP_RELEASE) == 0);
        REQUIRE (ccs.at ((int) Pro800CCMessages::OSC_A_SYNC) == CC_ON);
        REQUIRE (ccs.at ((int) Pro800CCMessages::LFO_MOD_DEST_FILTER) == CC_ON);
        REQUIRE (ccs.at ((int) Pro800CCMessages::LFO_MOD_DEST_FREQ_AB) == CC_OFF);
        REQUIRE (ccs.count ((int) Pro800CCMessages::LFO_MOD_SHAPE) == 0); // no previous shape to resolve it against

        for (const auto& bytes : bench.synth.received)
        {
            if (bytes.size() == 3 && (bytes[0] & 0xF0) == 0xB0)
            {
                REQUIRE ((bytes[0] & 0x0F) == 2); // channel 3, as the synth reports
            }
        }
    }

    SECTION ("the controls show it, and it is not counted as an edit made here")
    {
        REQUIRE ((int) mirror.cutoff.getValue() == 65535);
        REQUIRE (mirror.cutoff.getAlpha() > 0.99f);
        REQUIRE ((int) mirror.masterVolume.getValue() == 100);
        REQUIRE (mirror.oscASync.getToggleState());
        REQUIRE (mirror.lfoDestFilter.getToggleState());

        const auto& provenance = bench.session.getProvenance();
        REQUIRE (provenance.basis == SynthSession::Provenance::Basis::PANEL);
        REQUIRE_FALSE (provenance.isEdited());
        REQUIRE (provenance.describe() == "aligned with the panel");
    }

    SECTION ("the synth's echo of what we just sent is not mistaken for a knob movement there")
    {
        bench.midiHandler.handleIncomingMidiMessage (nullptr, juce::MidiMessage::controllerEvent (3, (int) Pro800CCMessages::FILTER_CUTOFF, 127));
        bench.messageThread.runFor (50);
        REQUIRE_FALSE (bench.session.getProvenance().editedOnSynth);

        // a movement the synth reports that we did not send is one
        bench.midiHandler.handleIncomingMidiMessage (nullptr, juce::MidiMessage::controllerEvent (3, (int) Pro800CCMessages::FILTER_RESONANCE, 40));
        bench.messageThread.runFor (50);
        REQUIRE (bench.session.getProvenance().editedOnSynth);
    }

    SECTION ("the one thing that did not work is named, and only that")
    {
        REQUIRE (bench.session.getLastError().contains ("LFO shape"));
        REQUIRE (bench.session.getLastError().length() < 100); // it shares one line with everything else
    }
}

TEST_CASE ("SynthSession: aligning resolves the LFO shape from the switch and the shape selected before", "[session][panel]")
{
    Bench bench;
    PanelMirror mirror (bench.midiHandler, bench.session);
    bench.connect();

    // the shape the session knows about is what the switch is resolved against: here, one the plugin just sent
    bench.midiHandler.sendMidiCCMessage (Pro800CCMessages::LFO_MOD_SHAPE, 66); // Sine, which names the Sine/Random pair

    // the switch is on its Tri/Sine/Saw side, so Sine stays
    bench.synth.panel[(uint8_t) Pro800PanelIndex::SWITCH_LFO_SHAPE] = 1;
    bench.session.alignWithPanel();
    bench.messageThread.runFor (800);

    REQUIRE (bench.synth.receivedCCs().at ((int) Pro800CCMessages::LFO_MOD_SHAPE) == 66); // Sine, per docs/Pro800CCMessages.md
    REQUIRE (mirror.lfoShape.getSelectedId() - MidiComponent::COMBO_BOX_ID_OFFSET == PROGRAM_LFO_SHAPE_SINE);

    SECTION ("the other switch position is the pair's other shape")
    {
        bench.synth.panel[(uint8_t) Pro800PanelIndex::SWITCH_LFO_SHAPE] = 0;
        bench.session.alignWithPanel();
        bench.messageThread.runFor (800);

        REQUIRE (bench.synth.receivedCCs().at ((int) Pro800CCMessages::LFO_MOD_SHAPE) == 44); // Random
    }
}

TEST_CASE ("SynthSession: the shape the switch is resolved against comes from the preset last shown here", "[session][panel]")
{
    Bench bench;
    PanelMirror mirror (bench.midiHandler, bench.session);

    ProgramMessage brass (toMidi (programDump (105)));
    brass.setValue (Pro800ProgramField::LFO_SHAPE, PROGRAM_LFO_SHAPE_NOISE);
    brass.setProgramName ("Brass");
    bench.synth.programs[105] = brass.getRawData();

    bench.connect();
    bench.session.selectProgram (105); // shows the record, so its shape is what the session knows
    bench.messageThread.runFor (1200);

    bench.synth.panel[(uint8_t) Pro800PanelIndex::SWITCH_LFO_SHAPE] = 1; // the Tri/Sine/Saw side
    bench.session.alignWithPanel();
    bench.messageThread.runFor (800);

    REQUIRE (bench.synth.receivedCCs().at ((int) Pro800CCMessages::LFO_MOD_SHAPE) == 110); // Noise's partner Saw
    REQUIRE (bench.session.getLastError().isEmpty()); // nothing to report
}

TEST_CASE ("SynthSession: aligning is refused when the synth would not hear the CC", "[session][panel]")
{
    SECTION ("MIDI CC Mode does not receive")
    {
        Bench bench;
        bench.synth.setSetting (Pro800Settings::MIDI_CC_MODE, SETTINGS_MIDI_MODE_TX);
        bench.connect();
        const size_t sentBefore = bench.synth.received.size();

        bench.session.alignWithPanel();
        bench.messageThread.runFor (300);

        REQUIRE (bench.session.getLastError().contains ("CC Mode"));
        REQUIRE (bench.synth.received.size() == sentBefore); // not even the panel was read
        REQUIRE (bench.session.getProvenance().basis == SynthSession::Provenance::Basis::UNKNOWN);
    }

    SECTION ("the synth's MIDI channel is OFF")
    {
        Bench bench;
        bench.synth.setSetting (Pro800Settings::MIDI_RX_CHANNEL, SETTINGS_MIDI_RX_OFF);
        bench.connect();

        bench.session.alignWithPanel();
        bench.messageThread.runFor (300);

        REQUIRE (bench.session.getLastError().contains ("OFF"));
        REQUIRE (bench.synth.receivedCCs().empty());
    }
}

//==============================================================================
namespace
{
    /** "B05" for 105, as the synth's own display writes it. */
    juce::String programLabelForTest (int program)
    {
        return juce::String::formatted ("%c%02d", 'A' + program / SettingsMessage::PROGRAMS_PER_BANK, program % SettingsMessage::PROGRAMS_PER_BANK);
    }

    /** How many program reads (0x77 at a program address) the synth was asked for. */
    int programReads (const FakePro800& synth)
    {
        int reads = 0;
        for (const auto& bytes : synth.received)
        {
            const bool isSettings = bytes.size() > Pro800DataMessage::ADDRESS_MSB_POS && bytes[Pro800DataMessage::ADDRESS_LSB_POS] == SettingsMessage::ADDRESS_LOW
                                    && bytes[Pro800DataMessage::ADDRESS_MSB_POS] == SettingsMessage::ADDRESS_HIGH;

            if (bytes.size() > Pro800DataMessage::ADDRESS_MSB_POS && bytes[Pro800MidiMessage::POS_MESSAGE_TYPE] == Pro800DataMessage::REQUEST_ID && !isSettings)
            {
                reads++;
            }
        }
        return reads;
    }
}

TEST_CASE ("SynthSession: reading all programs walks the whole address space, empty slots included", "[session][programs]")
{
    Bench bench;
    bench.connect();
    const int readsBefore = programReads (bench.synth);

    bench.session.readAllPrograms();
    REQUIRE (bench.session.isBusy());
    REQUIRE (bench.session.isActivityCancellable());
    REQUIRE (bench.session.getActivityTotal() == ProgramMessage::NUM_PROGRAMS);
    REQUIRE (bench.session.getActivity().contains ("0/400"));

    bench.messageThread.runFor (4000);

    REQUIRE_FALSE (bench.session.isBusy());
    REQUIRE_FALSE (bench.session.isActivityCancellable()); // the progress bar goes away with it
    REQUIRE (bench.session.getLastError().isEmpty());

    // one request per slot and no more: an empty slot answers with F0 F7, so nothing timed out and was retried
    REQUIRE (programReads (bench.synth) - readsBefore == ProgramMessage::NUM_PROGRAMS);
}

TEST_CASE ("SynthSession: a dump can be stopped, and gives up if the synth goes quiet", "[session][programs]")
{
    SECTION ("cancelling stops it after the request in flight")
    {
        Bench bench;
        bench.connect();
        const int readsBefore = programReads (bench.synth);

        bench.session.readAllPrograms();
        bench.session.cancelActivity(); // the first slot is already on its way
        bench.messageThread.runFor (1000);

        REQUIRE_FALSE (bench.session.isBusy());
        REQUIRE (programReads (bench.synth) - readsBefore < 5);
        REQUIRE (bench.session.getLastError().contains ("Stopped after"));
    }

    SECTION ("a synth that stops answering ends it rather than timing out four hundred times")
    {
        Bench bench;
        bench.connect();
        const int readsBefore = programReads (bench.synth);

        bench.synth.dropNextRequests = 1000;
        bench.session.readAllPrograms();
        bench.messageThread.runFor (8000);

        REQUIRE_FALSE (bench.session.isBusy());

        // three slots, each tried twice, and then it stops
        REQUIRE (programReads (bench.synth) - readsBefore <= 2 * SynthSession::MAX_CONSECUTIVE_FAILURES);

        // A00 was the first slot that went unanswered, so nothing was read at all
        REQUIRE (bench.session.getLastError() == "The synth did not answer any program request.");

        // and the synth is gone, so the connection goes with it rather than waiting for the poll to notice
        REQUIRE_FALSE (bench.session.isConnected());
    }
}

TEST_CASE ("SynthSession: a dump that dies part-way names where the data ends", "[session][programs]")
{
    Bench bench;
    bench.connect();

    bench.synth.dropFromProgram = 55; // A55 and everything after it goes unanswered
    bench.session.readAllPrograms();
    bench.messageThread.runFor (8000);

    REQUIRE_FALSE (bench.session.isBusy());

    // A55, A56 and A57 were all tried and none answered, so the list holds nothing for them: the message names
    // the first of the three and the last slot that did answer, not the last one tried
    REQUIRE (bench.session.getLastError() == "The synth stopped answering at program A55; the list holds what was read up to A54.");
    REQUIRE_FALSE (bench.session.isConnected());
}

TEST_CASE ("SynthSession: writing programs confirms each one by reading the slot back", "[session][programs]")
{
    Bench bench;
    bench.connect();

    ProgramMessage first (toMidi (programDump (200)));
    first.setProgramName ("Written A");
    ProgramMessage second (toMidi (programDump (201)));
    second.setProgramName ("Written B");

    const std::vector<std::shared_ptr<ProgramMessage>> programs = { std::make_shared<ProgramMessage> (first), std::make_shared<ProgramMessage> (second) };

    bench.session.writePrograms (programs);
    REQUIRE (bench.session.getActivityTotal() == 2);
    bench.messageThread.runFor (2000);

    REQUIRE_FALSE (bench.session.isBusy());
    REQUIRE (bench.session.getLastError().isEmpty());
    REQUIRE (bench.synth.programs.count (200) == 1);
    REQUIRE (ProgramMessage (toMidi (bench.synth.programs.at (200))).getProgramName() == "Written A");
    REQUIRE (ProgramMessage (toMidi (bench.synth.programs.at (201))).getProgramName() == "Written B");

    SECTION ("each slot was written and then read back")
    {
        REQUIRE (bench.synth.indexOfSent (Pro800DataMessage::RESPONSE_ID, { 0x48, 0x01 }) >= 0); // write of 200
        REQUIRE (bench.synth.indexOfSent (Pro800DataMessage::REQUEST_ID, { 0x48, 0x01 }) > bench.synth.indexOfSent (Pro800DataMessage::RESPONSE_ID, { 0x48, 0x01 }));
    }
}

TEST_CASE ("SynthSession: a write the synth accepts but does not store is named", "[session][programs]")
{
    Bench bench;
    bench.connect();
    bench.synth.ignoreProgramWrites = true; // status OK, nothing stored - what a read-back is there to catch

    ProgramMessage program (toMidi (programDump (200)));
    program.setProgramName ("Lost");
    bench.session.writePrograms ({ std::make_shared<ProgramMessage> (program) });
    bench.messageThread.runFor (2000);

    REQUIRE_FALSE (bench.session.isBusy());
    REQUIRE (bench.session.getLastError().contains ("could not be confirmed"));
    REQUIRE (bench.session.getLastError().contains ("C00")); // program 200
}
