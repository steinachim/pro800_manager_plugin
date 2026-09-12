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

#include "SynthSession.h"

#include "../midi/LiveParameterMessage.h"
#include "../midi/PanelMessage.h"
#include "../midi/Pro800MessageFactory.h"
#include "../midi/ProgramMessage.h"
#include "../midi/ReloadMessage.h"
#include "../midi/SettingsMessage.h"
#include "../midi/StatusMessage.h"
#include "../midi/VersionMessage.h"
#include "../tailoring/Pro800CCUtils.h"

#include <algorithm>

namespace
{
    juce::String programLabel (int program)
    {
        return juce::String::formatted ("%c%02d", 'A' + program / SettingsMessage::PROGRAMS_PER_BANK, program % SettingsMessage::PROGRAMS_PER_BANK);
    }

    bool isAccepted (const juce::MidiMessage* reply)
    {
        return reply != nullptr && StatusMessage (*reply).getStatus() == StatusMessage::STATUS_OK;
    }

    const Pro800PanelIndex DIP_INDICES[] = { Pro800PanelIndex::DIP_1, Pro800PanelIndex::DIP_2, Pro800PanelIndex::DIP_3, Pro800PanelIndex::DIP_4 };
}

//==============================================================================
std::optional<int> SynthSession::Channels::sendChannel() const
{
    if (!this->automatic)
    {
        return this->manualChannel;
    }

    if (!this->readFromSynth)
    {
        return Pro800ChannelResolver::CHANNEL_FOR_ALL; // nothing known yet: the plugin's historical default
    }

    return this->rx.sendChannel();
}

int SynthSession::Channels::receiveChannel() const
{
    if (!this->receiveAutomatic)
    {
        return this->manualReceiveChannel;
    }

    // THRU, an unread DIP setting or an invalid value leave nothing to filter on
    return this->readFromSynth ? this->tx.sendChannel().value_or (0) : 0;
}

juce::String SynthSession::Channels::describeReceive() const
{
    using Kind = Pro800MidiChannel::Kind;

    if (!this->receiveAutomatic)
    {
        return this->manualReceiveChannel == 0 ? juce::String ("All") : juce::String (this->manualReceiveChannel);
    }

    if (!this->readFromSynth)
    {
        return "Auto (not read yet)";
    }

    switch (this->tx.kind)
    {
        case Kind::CHANNEL:
            return "Auto (" + juce::String (this->tx.channel) + ", from synth)";
        case Kind::DIP:
            return this->tx.channel > 0 ? "Auto (" + juce::String (this->tx.channel) + ", DIP switches)" : "Auto (any - DIP switches not read)";
        case Kind::THRU:
            return "Auto (any - synth is on THRU)";
        case Kind::ALL:
        case Kind::OFF:
        case Kind::INVALID:
        default:
            return "Auto (any - invalid " + juce::String (this->tx.raw) + ")";
    }
}

juce::String SynthSession::Channels::describe() const
{
    using Kind = Pro800MidiChannel::Kind;

    if (!this->automatic)
    {
        juce::String description (this->manualChannel);
        if (this->readFromSynth && this->rx.sendChannel() != this->manualChannel)
        {
            description += " (synth reports " + juce::String (this->rx.toString()) + ")";
        }
        return description;
    }

    if (!this->readFromSynth)
    {
        return "Auto (not read yet)";
    }

    switch (this->rx.kind)
    {
        case Kind::CHANNEL:
            return "Auto (" + juce::String (this->rx.channel) + ", from synth)";
        case Kind::DIP:
            return this->rx.channel > 0 ? "Auto (" + juce::String (this->rx.channel) + ", DIP switches)" : "Auto (DIP switches, not read)";
        case Kind::ALL:
            return "Auto (any - using " + juce::String (Pro800ChannelResolver::CHANNEL_FOR_ALL) + ")";
        case Kind::OFF:
            return "Auto (OFF!)";
        case Kind::THRU:
        case Kind::INVALID:
        default:
            return "Auto (invalid " + juce::String (this->rx.raw) + " - synth is deaf)";
    }
}

juce::String SynthSession::Pointer::label() const
{
    return this->program.has_value() ? programLabel (*this->program) : juce::String ("-");
}

juce::String SynthSession::Provenance::describe() const
{
    juce::String edits;
    if (this->editedInApp && this->editedOnSynth)
    {
        edits = " + edits (in app and on synth)";
    }
    else if (this->editedInApp)
    {
        edits = " + edits (in app)";
    }
    else if (this->editedOnSynth)
    {
        edits = " + edits (on synth)";
    }

    switch (this->basis)
    {
        case Basis::STORED:
            return programLabel (this->program.value_or (0)) + (edits.isEmpty() ? " as stored" : edits);
        case Basis::PANEL:
            return "aligned with the panel" + edits;
        case Basis::UNKNOWN:
        default:
            return "unknown" + edits;
    }
}

//==============================================================================
SynthSession::SynthSession (MidiHandler& handler) : midiHandler (handler)
{
    this->midiHandler.addListener (this);
}

SynthSession::~SynthSession()
{
    this->masterReference.clear(); // from here on no callback reaches this object
    stopTimer();
    this->writeDebounce.stopTimer();
    this->midiHandler.removeListener (this);
    this->midiHandler.cancelExchanges();
}

void SynthSession::addListener (Listener* listener)
{
    this->listeners.add (listener);
}

void SynthSession::removeListener (Listener* listener)
{
    this->listeners.remove (listener);
}

//==============================================================================
void SynthSession::connect()
{
    stopTimer();
    this->midiHandler.cancelExchanges();

    this->connectionState = ConnectionState::PROBING;
    this->firmwareVersion.clear();
    this->firmwareSupported = false;
    this->channels.readFromSynth = false;
    this->pointer = Pointer();
    this->provenance = Provenance();
    this->settings = nullptr;
    this->pointerProgram = nullptr;
    this->pointerProgramReadFor.reset();
    this->lastPanelState.reset();
    this->currentLfoShape.reset();
    this->cancelRequested = false;
    this->pollFailures = 0;
    this->pendingWrites.clear();
    this->failedWrites.clear();
    this->writeDebounce.stopTimer();
    this->settingsWriteStatus = SettingsWriteStatus();
    this->lastError.clear();

    if (!this->midiHandler.hasOpenDevices())
    {
        this->connectionState = ConnectionState::DISCONNECTED;
        fail ("Select a MIDI input and output first.");
        return;
    }

    setActivity ("Connecting...");

    // the first request on a port is routinely lost, and a port that sat idle loses two: hence the extra retries
    sendRequest (VersionMessage::request(), SysExMatchers::isVersionReply, "firmware version request", SysExExchange::FIRST_REQUEST_RETRIES, false, [this] (const juce::MidiMessage* reply) {
        if (reply == nullptr)
        {
            this->connectionState = ConnectionState::DISCONNECTED;
            setActivity ("");
            fail ("No Pro-800 answered. Check the MIDI ports and the cables.");
            return;
        }

        const VersionMessage version (*reply);
        this->firmwareVersion = version.getVersionString();
        this->firmwareSupported = version.isSupported();

        setActivity ("Reading settings...");
        readSettings (false, [this] (std::shared_ptr<SettingsMessage> newSettings) { continueConnectWithSettings (newSettings); });
    });
}

void SynthSession::continueConnectWithSettings (std::shared_ptr<SettingsMessage> newSettings)
{
    if (newSettings == nullptr)
    {
        // identified, but blind: the user has to pick the channel
        this->channels.automatic = false;
        applyChannels();
        this->connectionState = ConnectionState::CONNECTED;
        startTimer (POLL_INTERVAL_MS);
        setActivity ("");
        fail ("Connected, but the settings could not be read - set the MIDI channel manually.");
        return;
    }

    applySettings (newSettings);

    if (this->channels.rx.kind == Pro800MidiChannel::Kind::DIP || this->channels.tx.kind == Pro800MidiChannel::Kind::DIP)
    {
        setActivity ("Reading DIP switches...");
        readDipSwitches ([this] (std::optional<int> dipSum) {
            if (dipSum.has_value())
            {
                applySettings (this->settings, dipSum);
            }
            finishConnect();
        });
        return;
    }

    finishConnect();
}

void SynthSession::finishConnect()
{
    this->connectionState = ConnectionState::CONNECTED;
    this->lastError.clear();
    setActivity ("Reading current preset...");

    // the pointer is shown, the controls are not touched: the sound is not known to be the stored record
    updatePointer (false, false, [this] {
        setActivity ("");
        startTimer (POLL_INTERVAL_MS);
    });
}

void SynthSession::disconnect()
{
    stopTimer();

    // a dump or transfer in progress ends here: cancelling the exchange completes its request in flight with no
    // reply, and the step that gets it would otherwise count one failure and ask for the next slot
    this->cancelRequested = true;
    this->midiHandler.cancelExchanges();
    this->connectionState = ConnectionState::DISCONNECTED;
    this->channels.readFromSynth = false;
    this->pointer = Pointer();
    this->provenance = Provenance();
    this->activity.clear();
    this->writeDebounce.stopTimer();
    this->pendingWrites.clear();
    this->failedWrites.clear();
    this->settingsWriteStatus = SettingsWriteStatus();
    notify();
}

//==============================================================================
bool SynthSession::canStartAction() const
{
    return isConnected() && !isBusy();
}

void SynthSession::selectProgram (int program)
{
    if (!canStartAction() || program < 0 || program >= ProgramMessage::NUM_PROGRAMS)
    {
        return;
    }

    const juce::String label = programLabel (program);
    setActivity ("Selecting " + label + "...");

    readSettings (false, [this, program, label] (std::shared_ptr<SettingsMessage> current) {
        if (current == nullptr)
        {
            setActivity ("");
            fail ("Could not read the synth's settings to select " + label + ".");
            return;
        }

        // the freshest block, with any setting still committing kept as written (see protectPendingWrites())
        applySettings (current);
        this->settings->setCurrentProgram (program);
        auto patched = std::make_shared<SettingsMessage> (*this->settings);

        // until the synth shows the new pointer, a settings change must not write the old one back. Marked as
        // written: only a written entry is confirmed or expired by protectPendingWrites(), and one that never is
        // would put this pointer back over every block read from then on - a preset picked on the synth's own
        // panel would go unnoticed.
        const double deadline = juce::Time::getMillisecondCounterHiRes() + WRITE_CONFIRM_WINDOW_MS;
        this->pendingWrites[Pro800Settings::PRESET_NUM] = { patched->getValue (Pro800Settings::PRESET_NUM), deadline, true };
        this->pendingWrites[Pro800Settings::CURRENT_BANK] = { patched->getValue (Pro800Settings::CURRENT_BANK), deadline, true };

        this->pointer.freshness = PointerFreshness::PENDING;
        notify();

        writeSettings (patched, [this, program, label, deadline] (bool accepted) {
            if (!accepted)
            {
                this->pointer.freshness = PointerFreshness::UNCONFIRMED;
                setActivity ("");
                fail ("The synth did not accept the selection of " + label + ".");
                return;
            }

            waitForSettings ([program] (const SettingsMessage& s) { return s.getCurrentProgram() == program; }, deadline, [this, program, label] (bool confirmed) {
                if (!confirmed)
                {
                    this->pointer.program = this->settings != nullptr ? this->settings->getCurrentProgram() : std::nullopt;
                    this->pointer.freshness = PointerFreshness::UNCONFIRMED;
                    setActivity ("");
                    fail ("The synth did not confirm the selection of " + label + " - try again.");
                    return;
                }

                this->pointer.program = program;
                this->pointer.freshness = PointerFreshness::CONFIRMED;

                // only now: a reload sent before the pointer has committed recalls the preset on its way out
                sendReload ([this, program, label] (bool reloaded) {
                    if (!reloaded)
                    {
                        fail ("The synth did not acknowledge the reload: its display shows " + label + " but it may still play the previous preset.");
                    }
                    else
                    {
                        this->lastError.clear();
                    }

                    showStoredProgram (program, [this] { setActivity (""); });
                }); });
        });
    });
}

void SynthSession::reloadStoredProgram()
{
    if (!canStartAction())
    {
        return;
    }

    setActivity ("Reverting to the stored preset...");

    sendReload ([this] (bool reloaded) {
        if (!reloaded)
        {
            setActivity ("");
            fail ("The synth did not acknowledge the reload.");
            return;
        }

        this->lastError.clear();
        if (this->pointer.program.has_value())
        {
            showStoredProgram (*this->pointer.program, [this] { setActivity (""); });
            return;
        }

        // the pointer was never read: read it now and show that record
        readSettings (false, [this] (std::shared_ptr<SettingsMessage> newSettings) {
            if (newSettings != nullptr)
            {
                applySettings (newSettings);
            }
            updatePointer (true, false, [this] { setActivity (""); });
        });
    });
}

void SynthSession::refresh()
{
    if (!canStartAction())
    {
        return;
    }

    poll (false);
}

void SynthSession::writeSetting (Pro800Settings setting, int value)
{
    if (this->settings == nullptr)
    {
        fail ("Load the settings first.");
        return;
    }

    // the block the components look at is patched in place, so they show the new value right away
    this->settings->setValue (setting, value);
    this->pendingWrites[setting] = { value, juce::Time::getMillisecondCounterHiRes() + WRITE_DEBOUNCE_MS + WRITE_CONFIRM_WINDOW_MS };

    // one block write for a run of changes (a spin box fires per click)
    this->writeDebounce.onFire = [weak = juce::WeakReference<SynthSession> (this)] {
        if (weak != nullptr)
        {
            weak->flushPendingWrites();
        }
    };
    this->writeDebounce.startTimer (WRITE_DEBOUNCE_MS);

    this->settingsWriteStatus.state = SettingsWriteStatus::State::SAVING;
    this->settingsWriteStatus.message = "Saving " + juce::String (PRO800_SETTINGS_FIELDS.at (setting).name) + "...";
    this->settingsWriteStatus.changedOnSynth.clear();
    notify();
}

void SynthSession::flushPendingWrites()
{
    if (this->settings == nullptr || this->pendingWrites.empty())
    {
        return;
    }

    juce::StringArray names;
    const double deadline = juce::Time::getMillisecondCounterHiRes() + WRITE_CONFIRM_WINDOW_MS;
    for (auto& [setting, pending] : this->pendingWrites)
    {
        if (!pending.written)
        {
            pending.written = true;
            pending.deadline = deadline;
            names.add (PRO800_SETTINGS_FIELDS.at (setting).name);
        }
    }

    writeSettings (std::make_shared<SettingsMessage> (*this->settings), [this, names] (bool accepted) {
        if (!accepted)
        {
            fail ("The synth did not accept the new " + names.joinIntoString (", ") + ".");
        }
    });

    verifyPendingWrites();
}

void SynthSession::verifyPendingWrites()
{
    if (this->verifyingWrites)
    {
        return; // the running loop picks up whatever was added meanwhile
    }
    this->verifyingWrites = true;

    juce::Timer::callAfterDelay (WRITE_CONFIRM_STEP_MS, [weak = juce::WeakReference<SynthSession> (this)] {
        if (weak == nullptr)
        {
            return;
        }

        weak->readSettings (false, [weak] (std::shared_ptr<SettingsMessage> newSettings) {
            if (weak == nullptr)
            {
                return;
            }

            weak->verifyingWrites = false;

            if (newSettings != nullptr)
            {
                weak->applySettings (newSettings); // confirms or expires the pending writes
            }

            const bool stillPending = std::any_of (weak->pendingWrites.begin(), weak->pendingWrites.end(), [] (const auto& entry) { return entry.second.written; });
            if (stillPending || weak->writeDebounce.isTimerRunning())
            {
                weak->verifyPendingWrites();
                return;
            }

            // everything has either landed or given up
            auto& status = weak->settingsWriteStatus;
            if (weak->failedWrites.empty())
            {
                status.state = SettingsWriteStatus::State::IDLE;
                status.message.clear();
            }
            else
            {
                juce::StringArray what;
                for (const auto& [setting, pending] : weak->failedWrites)
                {
                    what.add (juce::String (PRO800_SETTINGS_FIELDS.at (setting).name) + " = " + juce::String (pending.value) + " (it reads back " + juce::String (pending.readBack) + ")");
                }
                status.state = SettingsWriteStatus::State::FAILED;
                status.message = "The synth did not take " + what.joinIntoString (", ") + ". Is a settings menu open on the synth?";
                weak->failedWrites.clear();
            }
            weak->notify();
        });
    });
}

void SynthSession::setManualChannel (std::optional<int> channel)
{
    this->channels.automatic = !channel.has_value();
    if (channel.has_value())
    {
        this->channels.manualChannel = juce::jlimit (1, 16, *channel);
    }

    applyChannels();
    notify();
}

void SynthSession::setManualReceiveChannel (std::optional<int> channel)
{
    this->channels.receiveAutomatic = !channel.has_value();
    if (channel.has_value())
    {
        this->channels.manualReceiveChannel = juce::jlimit (0, 16, *channel);
    }

    applyChannels();
    notify();
}

juce::String SynthSession::reasonCCWouldNotArrive() const
{
    if (!this->channels.sendChannel().has_value())
    {
        return "the synth's MIDI channel is " + juce::String (this->channels.rx.toString()) + ", so it ignores CC";
    }

    if (this->settings != nullptr && this->settings->isValid())
    {
        const int ccMode = this->settings->getValue (Pro800Settings::MIDI_CC_MODE);
        if (ccMode != SETTINGS_MIDI_MODE_RX && ccMode != SETTINGS_MIDI_MODE_TX_RX)
        {
            return "the synth's MIDI CC Mode does not receive CC";
        }
    }

    return {};
}

void SynthSession::alignWithPanel()
{
    if (!canStartAction())
    {
        return;
    }

    if (const auto reason = reasonCCWouldNotArrive(); reason.isNotEmpty())
    {
        fail ("Cannot align with the panel: " + reason + ". Change it in the Settings tab and try again.");
        return;
    }

    setActivity ("Reading the panel...");

    readPanelState ([this, previousLfoShape = this->currentLfoShape] (std::optional<Pro800PanelState> state) {
        if (!state.has_value())
        {
            setActivity ("");
            fail ("The synth stopped answering while its panel was being read; nothing was changed.");
            return;
        }

        this->lastPanelState = state;

        // send first, then show: the sends are what make the sound agree with the panel, and they set the
        // "edited here" flag that the provenance below deliberately starts from again
        const auto ccValues = Pro800PanelConversion::toCCValues (*state, previousLfoShape);
        const auto channel = this->channels.sendChannel().value_or (Pro800ChannelResolver::CHANNEL_FOR_ALL);

        std::vector<juce::MidiMessage> messages;
        messages.reserve (ccValues.size());
        for (const auto& [cc, value] : ccValues)
        {
            messages.push_back (juce::MidiMessage::controllerEvent (channel, static_cast<int> (cc), value));
        }

        setActivity ("Sending the panel to the synth...");
        this->midiHandler.sendChannelVoiceBurst (messages);

        this->midiHandler.mirrorPanel (Pro800PanelConversion::toValues (*state, previousLfoShape));

        this->provenance = Provenance();
        this->provenance.basis = Provenance::Basis::PANEL;
        setActivity ("");

        // the shape switch carries one bit, and without a shape to resolve it against there is nothing to send;
        // everything else worked, so this is the only thing worth a word (the button's tooltip carries the rest)
        if (ccValues.count (Pro800CCMessages::LFO_MOD_SHAPE) == 0)
        {
            warn ("Aligned, except the LFO shape - select one and align again.");
        }
    });
}

void SynthSession::readAllPrograms()
{
    if (!canStartAction())
    {
        return;
    }

    this->cancelRequested = false;
    setActivity ("Reading programs", 0, ProgramMessage::NUM_PROGRAMS);
    readAllProgramsStep (0, 0);
}

void SynthSession::readAllProgramsStep (int program, int consecutiveFailures)
{
    if (this->cancelRequested || program >= ProgramMessage::NUM_PROGRAMS)
    {
        const bool cancelled = this->cancelRequested;
        this->cancelRequested = false;
        setActivity ("");

        if (cancelled)
        {
            warn ("Stopped after " + juce::String (program) + " of " + juce::String (ProgramMessage::NUM_PROGRAMS) + " programs.");
        }
        else
        {
            this->lastError.clear();
            notify();
        }
        return;
    }

    readProgram (program, false, [this, program, consecutiveFailures] (std::shared_ptr<ProgramMessage> record, bool slotIsEmpty) {
        // an empty slot answers too - with the bare F0 F7 - so only silence counts as a failure here
        const bool answered = (record != nullptr || slotIsEmpty);
        const int failures = answered ? 0 : consecutiveFailures + 1;

        if (failures >= MAX_CONSECUTIVE_FAILURES)
        {
            // the failures were consecutive by construction, so this is where the answers stopped - and the slots
            // named after it hold nothing that was read, whatever the list still shows for them
            const int firstUnanswered = program - (MAX_CONSECUTIVE_FAILURES - 1);

            this->cancelRequested = false;
            setActivity ("");
            disconnect();
            fail (firstUnanswered > 0
                      ? "The synth stopped answering at program " + programLabel (firstUnanswered) + "; the list holds what was read up to "
                            + programLabel (firstUnanswered - 1) + "."
                      : juce::String ("The synth did not answer any program request."));
            return;
        }

        setActivityProgress (program + 1);
        readAllProgramsStep (program + 1, failures);
    });
}

void SynthSession::writePrograms (ProgramList programs)
{
    if (!canStartAction() || programs.empty())
    {
        return;
    }

    this->cancelRequested = false;
    setActivity ("Sending programs", 0, (int) programs.size());
    writeProgramsStep (std::make_shared<ProgramList> (std::move (programs)), 0, std::make_shared<std::vector<int>>(), 0);
}

void SynthSession::writeProgramsStep (std::shared_ptr<ProgramList> programs, size_t index, std::shared_ptr<std::vector<int>> unconfirmed, int consecutiveFailures)
{
    if (this->cancelRequested || index >= programs->size())
    {
        const bool cancelled = this->cancelRequested;
        this->cancelRequested = false;
        setActivity ("");

        juce::StringArray names;
        for (const int program : *unconfirmed)
        {
            names.add (programLabel (program));
        }

        if (!names.isEmpty())
        {
            fail (juce::String (names.size()) + " program(s) could not be confirmed on the synth: " + names.joinIntoString (", ")
                  + ". Read the programs again to see what is stored.");
        }
        else if (cancelled)
        {
            warn ("Stopped after " + juce::String (index) + " of " + juce::String (programs->size()) + " programs.");
        }
        else
        {
            this->lastError.clear();
            notify();
        }
        return;
    }

    const auto& program = *(*programs)[index];
    const int programNumber = program.getProgramNumber();

    writeProgramVerified (program, [this, programs, index, unconfirmed, consecutiveFailures, programNumber] (bool answered, bool confirmed) {
        const int failures = answered ? 0 : consecutiveFailures + 1;

        if (failures >= MAX_CONSECUTIVE_FAILURES)
        {
            // as above: the last few were all unanswered, so the writes that got through end before them
            const size_t written = index + 1 - MAX_CONSECUTIVE_FAILURES;
            const auto& firstUnanswered = *(*programs)[index + 1 - MAX_CONSECUTIVE_FAILURES];

            this->cancelRequested = false;
            setActivity ("");
            disconnect();
            fail ("The synth stopped answering at program " + programLabel (firstUnanswered.getProgramNumber()) + "; "
                  + juce::String ((int) written) + " program(s) were written before that.");
            return;
        }

        if (!confirmed)
        {
            unconfirmed->push_back (programNumber);
        }

        setActivityProgress ((int) index + 1);
        writeProgramsStep (programs, index + 1, unconfirmed, failures);
    });
}

void SynthSession::writeProgramVerified (const ProgramMessage& program, std::function<void (bool answered, bool confirmed)> callback)
{
    const int programNumber = program.getProgramNumber();
    const auto sent = std::make_shared<std::vector<uint8_t>> (program.getRawData());

    sendRequest (program.toMidiMessage(), SysExMatchers::isStatusReply, "program " + programLabel (programNumber) + " write", SysExExchange::DEFAULT_RETRIES, false, [this, programNumber, sent, callback] (const juce::MidiMessage* reply) {
        if (reply == nullptr)
        {
            callback (false, false);
            return;
        }

        if (!isAccepted (reply))
        {
            callback (true, false);
            return;
        }

        // the status says the write was taken, not that it was stored: read the slot back and compare
        readProgram (programNumber, false, [sent, callback] (std::shared_ptr<ProgramMessage> record, bool slotIsEmpty) {
            if (record == nullptr && !slotIsEmpty)
            {
                callback (false, false);
                return;
            }

            callback (true, record != nullptr && record->getRawData() == *sent);
        });
    });
}

void SynthSession::readPanelState (std::function<void (std::optional<Pro800PanelState>)> callback)
{
    readPanelStep (0, std::make_shared<Pro800PanelState>(), std::move (callback));
}

void SynthSession::readPanelStep (size_t step, std::shared_ptr<Pro800PanelState> state, std::function<void (std::optional<Pro800PanelState>)> callback)
{
    // first every panel index, then every knob
    const size_t numPanel = (size_t) Pro800PanelIndex::NUM_INDICES;
    const size_t numLive = (size_t) Pro800LiveIndex::NUM_INDICES;

    if (step >= numPanel + numLive)
    {
        callback (*state);
        return;
    }

    if (step < numPanel)
    {
        const auto index = (uint8_t) step;
        const auto matcher = [index] (const juce::MidiMessage& m) { return SysExMatchers::isPanelReplyFor (m, index); };
        sendRequest (PanelMessage::request (index), matcher, "panel read", SysExExchange::DEFAULT_RETRIES, false, [this, step, state, callback, index] (const juce::MidiMessage* reply) {
            if (reply == nullptr)
            {
                callback (std::nullopt);
                return;
            }

            const PanelMessage panel (*reply);
            if (panel.isValid()) // a status means the synth refuses the index: leave it out
            {
                state->panel[static_cast<Pro800PanelIndex> (index)] = panel.getValue();
            }
            readPanelStep (step + 1, state, callback);
        });
        return;
    }

    const auto index = (uint8_t) (step - numPanel);
    const auto matcher = [index] (const juce::MidiMessage& m) { return SysExMatchers::isLiveReplyFor (m, index); };
    sendRequest (LiveParameterMessage::request (index), matcher, "knob read", SysExExchange::DEFAULT_RETRIES, false, [this, step, state, callback, index] (const juce::MidiMessage* reply) {
        if (reply == nullptr)
        {
            callback (std::nullopt);
            return;
        }

        const LiveParameterMessage live (*reply);
        if (live.isValid())
        {
            state->live[static_cast<Pro800LiveIndex> (index)] = live.getValue();
        }
        readPanelStep (step + 1, state, callback);
    });
}

//==============================================================================
void SynthSession::sendRequest (const juce::MidiMessage& message, SysExMatchers::Matcher matcher, const juce::String& description, int retries, bool isPolling, ReplyCallback callback)
{
    SysExExchange::Request request;
    request.message = message;
    request.matches = std::move (matcher);
    request.description = description;
    request.retries = retries;
    request.isPolling = isPolling;
    request.onComplete = [weak = juce::WeakReference<SynthSession> (this), callback] (const juce::MidiMessage* reply) {
        if (weak != nullptr)
        {
            callback (reply);
        }
    };

    this->midiHandler.exchange (std::move (request));
}

void SynthSession::readSettings (bool isPolling, SettingsCallback callback)
{
    const auto matcher = [] (const juce::MidiMessage& m) { return SysExMatchers::isDumpReplyFor (m, SettingsMessage::ADDRESS); };

    sendRequest (SettingsMessage::request(), matcher, "settings request", SysExExchange::DEFAULT_RETRIES, isPolling, [callback] (const juce::MidiMessage* reply) {
        if (reply == nullptr)
        {
            callback (nullptr);
            return;
        }

        auto received = std::dynamic_pointer_cast<SettingsMessage> (Pro800MessageFactory::createMidiMessage (*reply));
        callback (received != nullptr && received->isValid() ? received : nullptr);
    });
}

void SynthSession::readProgram (int program, bool isPolling, ProgramCallback callback)
{
    const auto matcher = [program] (const juce::MidiMessage& m) { return SysExMatchers::isDumpReplyFor (m, program); };

    sendRequest (ProgramMessage::request (program), matcher, "program " + programLabel (program) + " request", SysExExchange::DEFAULT_RETRIES, isPolling, [callback] (const juce::MidiMessage* reply) {
        if (reply == nullptr)
        {
            callback (nullptr, false);
            return;
        }

        if (SysExMatchers::isEmptySlotReply (*reply))
        {
            callback (nullptr, true);
            return;
        }

        auto received = std::dynamic_pointer_cast<ProgramMessage> (Pro800MessageFactory::createMidiMessage (*reply));
        callback (received != nullptr && received->isValid() ? received : nullptr, false);
    });
}

void SynthSession::readDipSwitches (std::function<void (std::optional<int> dipSum)> callback)
{
    readDipSwitch (0, std::make_shared<std::map<Pro800PanelIndex, int>>(), std::move (callback));
}

void SynthSession::readDipSwitch (size_t which, std::shared_ptr<std::map<Pro800PanelIndex, int>> values, std::function<void (std::optional<int> dipSum)> callback)
{
    if (which >= std::size (DIP_INDICES))
    {
        callback (Pro800ChannelResolver::dipChannelFromPanel (*values));
        return;
    }

    const Pro800PanelIndex index = DIP_INDICES[which];
    const auto matcher = [index] (const juce::MidiMessage& m) { return SysExMatchers::isPanelReplyFor (m, static_cast<uint8_t> (index)); };

    sendRequest (PanelMessage::request (index), matcher, "DIP switch read", SysExExchange::DEFAULT_RETRIES, false, [this, which, values, callback, index] (const juce::MidiMessage* reply) {
        if (reply == nullptr)
        {
            callback (std::nullopt);
            return;
        }

        const PanelMessage panel (*reply);
        if (!panel.isValid())
        {
            callback (std::nullopt); // a status: the synth refused the index
            return;
        }

        (*values)[index] = panel.getValue();
        readDipSwitch (which + 1, values, callback);
    });
}

void SynthSession::writeSettings (std::shared_ptr<SettingsMessage> newSettings, std::function<void (bool accepted)> callback)
{
    sendRequest (newSettings->toMidiMessage(), SysExMatchers::isStatusReply, "settings write", SysExExchange::DEFAULT_RETRIES, false, [callback] (const juce::MidiMessage* reply) {
        callback (isAccepted (reply));
    });
}

void SynthSession::sendReload (std::function<void (bool accepted)> callback)
{
    sendRequest (ReloadMessage::request(), SysExMatchers::isStatusReply, "preset reload", SysExExchange::DEFAULT_RETRIES, false, [callback] (const juce::MidiMessage* reply) {
        callback (isAccepted (reply));
    });
}

void SynthSession::waitForSettings (std::function<bool (const SettingsMessage&)> landed, double deadline, std::function<void (bool confirmed)> callback)
{
    juce::Timer::callAfterDelay (WRITE_CONFIRM_STEP_MS, [weak = juce::WeakReference<SynthSession> (this), landed, deadline, callback] {
        if (weak == nullptr)
        {
            return;
        }

        weak->readSettings (false, [weak, landed, deadline, callback] (std::shared_ptr<SettingsMessage> newSettings) {
            if (weak == nullptr)
            {
                return;
            }

            if (newSettings != nullptr)
            {
                const bool hasLanded = landed (*newSettings); // on the block as read, before protectPendingWrites() touches it
                weak->applySettings (newSettings);
                if (hasLanded)
                {
                    callback (true);
                    return;
                }
            }

            if (juce::Time::getMillisecondCounterHiRes() >= deadline)
            {
                callback (false);
                return;
            }

            weak->waitForSettings (landed, deadline, callback);
        });
    });
}

//==============================================================================
void SynthSession::poll (bool isPolling)
{
    readSettings (isPolling, [this, isPolling] (std::shared_ptr<SettingsMessage> newSettings) {
        if (newSettings == nullptr)
        {
            this->pollFailures++;

            if (this->pollFailures >= MAX_CONSECUTIVE_FAILURES)
            {
                // the synth is gone - unplugged, powered off, port taken by something else. Asking every
                // POLL_INTERVAL_MS from here to eternity would achieve nothing but a stream of timeouts.
                disconnect();
                fail ("The synth stopped answering. Check the cable and press Connect again.");
                return;
            }

            if (this->pollFailures >= 2 && this->pointer.freshness == PointerFreshness::CONFIRMED)
            {
                this->pointer.freshness = PointerFreshness::UNKNOWN;
                notify();
            }
            return;
        }

        this->pollFailures = 0;
        applySettings (newSettings);

        // the DIP switches are only consulted while a channel field says so; read them when that has just become the case
        if ((this->channels.rx.kind == Pro800MidiChannel::Kind::DIP && this->channels.rx.channel == 0)
            || (this->channels.tx.kind == Pro800MidiChannel::Kind::DIP && this->channels.tx.channel == 0))
        {
            readDipSwitches ([this] (std::optional<int> dipSum) {
                if (dipSum.has_value())
                {
                    applySettings (this->settings, dipSum);
                    notify();
                }
            });
        }

        // a preset selected on the synth itself has been recalled there, so the stored record is what it plays now
        updatePointer (true, isPolling, [] {});
    });
}

void SynthSession::updatePointer (bool mirrorIfChanged, bool isPolling, std::function<void()> then)
{
    const std::optional<int> program = this->settings != nullptr ? this->settings->getCurrentProgram() : std::nullopt;
    const bool changed = program != this->pointer.program;
    const bool knownBefore = this->pointer.program.has_value();

    this->pointer.program = program;
    this->pointer.freshness = PointerFreshness::CONFIRMED;

    if (!program.has_value())
    {
        this->pointer.name.clear();
        this->pointerProgram = nullptr;
        this->pointerProgramReadFor.reset();
        notify();
        then();
        return;
    }

    if (!changed && this->pointerProgramReadFor == program)
    {
        notify();
        then();
        return;
    }

    const bool mirror = mirrorIfChanged && changed && knownBefore;
    readProgram (*program, isPolling, [this, program, mirror, then] (std::shared_ptr<ProgramMessage> record, bool slotIsEmpty) {
        this->pointerProgram = record;
        this->pointerProgramReadFor = (record != nullptr || slotIsEmpty) ? program : std::nullopt;
        this->pointer.name = record != nullptr ? record->getProgramName() : (slotIsEmpty ? "(empty)" : "");

        if (mirror && record != nullptr)
        {
            this->currentLfoShape = record->getValue (Pro800ProgramField::LFO_SHAPE);
            this->midiHandler.mirrorProgram (*record);
            this->provenance = Provenance();
            this->provenance.basis = Provenance::Basis::STORED;
            this->provenance.program = program;
        }

        notify();
        then();
    });
}

void SynthSession::showStoredProgram (int program, std::function<void()> then)
{
    readProgram (program, false, [this, program, then] (std::shared_ptr<ProgramMessage> record, bool slotIsEmpty) {
        this->pointer.program = program;
        this->pointerProgram = record;
        this->pointerProgramReadFor = (record != nullptr || slotIsEmpty) ? std::optional<int> (program) : std::nullopt;
        this->pointer.name = record != nullptr ? record->getProgramName() : (slotIsEmpty ? "(empty)" : "");

        this->provenance = Provenance();
        if (record != nullptr)
        {
            this->currentLfoShape = record->getValue (Pro800ProgramField::LFO_SHAPE);
            this->midiHandler.mirrorProgram (*record);
            this->provenance.basis = Provenance::Basis::STORED;
            this->provenance.program = program;
        }

        notify();
        then();
    });
}

//==============================================================================
void SynthSession::applySettings (std::shared_ptr<SettingsMessage> newSettings)
{
    // keep a DIP reading that is already in hand
    std::optional<int> dipSum;
    if (this->channels.rx.kind == Pro800MidiChannel::Kind::DIP && this->channels.rx.channel > 0)
    {
        dipSum = this->channels.rx.channel - 1;
    }
    else if (this->channels.tx.kind == Pro800MidiChannel::Kind::DIP && this->channels.tx.channel > 0)
    {
        dipSum = this->channels.tx.channel - 1;
    }

    applySettings (newSettings, dipSum);
}

void SynthSession::applySettings (std::shared_ptr<SettingsMessage> newSettings, std::optional<int> dipSum)
{
    const bool freshRead = (newSettings != this->settings);
    if (freshRead)
    {
        // the front panel is a second writer to this block: closing a settings menu on the synth commits its value
        std::vector<Pro800Settings> changedOnSynth;
        if (this->settings != nullptr && this->settings->isValid())
        {
            for (const auto& [setting, parameter] : PRO800_SETTINGS_FIELDS)
            {
                if (setting != Pro800Settings::PRESET_NUM && setting != Pro800Settings::CURRENT_BANK && this->pendingWrites.count (setting) == 0
                    && newSettings->getValue (setting) != this->settings->getValue (setting))
                {
                    changedOnSynth.push_back (setting);
                }
            }
        }

        protectPendingWrites (*newSettings);

        if (!changedOnSynth.empty() && this->settingsWriteStatus.state == SettingsWriteStatus::State::IDLE)
        {
            this->settingsWriteStatus.changedOnSynth = changedOnSynth;
            juce::Timer::callAfterDelay (CHANGED_ON_SYNTH_NOTICE_MS, [weak = juce::WeakReference<SynthSession> (this)] {
                if (weak != nullptr)
                {
                    weak->settingsWriteStatus.changedOnSynth.clear();
                    weak->notify();
                }
            });
        }
    }
    this->settings = newSettings;
    this->channels.rx = Pro800ChannelResolver::resolveRx (newSettings->getValue (Pro800Settings::MIDI_RX_CHANNEL), dipSum);
    this->channels.tx = Pro800ChannelResolver::resolveTx (newSettings->getValue (Pro800Settings::MIDI_TX_CHANNEL), dipSum);
    this->channels.readFromSynth = true;
    applyChannels();

    if (freshRead)
    {
        notifySettingsChanged();
    }
}

void SynthSession::notifySettingsChanged()
{
    this->listeners.call ([] (Listener& l) { l.synthSessionSettingsChanged(); });
}

void SynthSession::protectPendingWrites (SettingsMessage& newSettings)
{
    const double now = juce::Time::getMillisecondCounterHiRes();

    for (auto it = this->pendingWrites.begin(); it != this->pendingWrites.end();)
    {
        auto& [setting, pending] = *it;
        const int readBack = newSettings.getValue (setting);

        if (pending.written && readBack == pending.value)
        {
            it = this->pendingWrites.erase (it); // the synth shows it: confirmed
        }
        else if (pending.written && now >= pending.deadline)
        {
            juce::Logger::writeToLog ("[WARNING] SynthSession: the synth never showed the new " + juce::String (PRO800_SETTINGS_FIELDS.at (setting).name)
                                      + " (" + juce::String (pending.value) + "); it reads " + juce::String (readBack));
            pending.readBack = readBack;
            this->failedWrites.emplace_back (setting, pending);
            it = this->pendingWrites.erase (it); // the block keeps what the synth shows
        }
        else
        {
            // not sent yet, or a read within the synth's commit lag: keep what we wrote, or the next settings change
            // would write the old value back
            newSettings.setValue (setting, pending.value);
            ++it;
        }
    }
}

void SynthSession::applyChannels()
{
    if (const auto channel = this->channels.sendChannel())
    {
        this->midiHandler.setMidiChannel ((uint8_t) *channel);
    }

    this->midiHandler.setInboundChannel ((uint8_t) this->channels.receiveChannel());
}

void SynthSession::setActivity (const juce::String& newActivity, int done, int total)
{
    this->activityDescription = newActivity;
    this->activityDone = done;
    this->activityTotal = total;
    this->activity = (total > 0 && newActivity.isNotEmpty()) ? newActivity + " " + juce::String (done) + "/" + juce::String (total) : newActivity;
    notify();
}

void SynthSession::setActivityProgress (int done)
{
    this->activityDone = done;
    this->activity = this->activityDescription + " " + juce::String (done) + "/" + juce::String (this->activityTotal);

    // one notification per slot would have the whole UI re-laying itself out a few hundred times a second
    if (done % PROGRESS_NOTIFY_EVERY == 0 || done == this->activityTotal)
    {
        notify();
    }
}

void SynthSession::cancelActivity()
{
    if (this->activityTotal > 0)
    {
        this->cancelRequested = true;
    }
}

void SynthSession::fail (const juce::String& error)
{
    juce::Logger::writeToLog ("[WARNING] SynthSession: " + error);
    this->lastError = error;
    notify();
}

void SynthSession::warn (const juce::String& note)
{
    juce::Logger::writeToLog ("[INFO] SynthSession: " + note);
    this->lastError = note;
    notify();
}

void SynthSession::notify()
{
    this->listeners.call ([] (Listener& l) { l.synthSessionChanged(); });
}

//==============================================================================
void SynthSession::timerCallback()
{
    if (!isConnected() || isBusy() || this->verifyingWrites || this->midiHandler.isExchangeBusy()
        || this->midiHandler.channelVoiceSentWithin (CHANNEL_VOICE_QUIET_MS))
    {
        return;
    }

    poll (true);
}

void SynthSession::channelVoiceSent (const juce::MidiMessage& message)
{
    noteLfoShapeFromCC (message);

    if (message.isController() && !this->provenance.editedInApp)
    {
        this->provenance.editedInApp = true;
        notify();
    }
}

void SynthSession::channelVoiceReceived (const juce::MidiMessage& message)
{
    noteLfoShapeFromCC (message);

    if (message.isController() && !this->provenance.editedOnSynth)
    {
        this->provenance.editedOnSynth = true;
        notify();
    }
}

void SynthSession::noteLfoShapeFromCC (const juce::MidiMessage& message)
{
    // whoever changed the shape - the plugin's own combo, an align, or the synth - said so with this CC
    if (message.isController() && message.getControllerNumber() == static_cast<int> (Pro800CCMessages::LFO_MOD_SHAPE))
    {
        this->currentLfoShape = Pro800CCUtils::programEnumValueFromCC (message.getControllerValue(), PROGRAM_LFO_SHAPE_NUM_VALUES);
    }
}
