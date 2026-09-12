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

#include "../midi/PanelMessage.h"
#include "../midi/Pro800MessageFactory.h"
#include "../midi/ProgramMessage.h"
#include "../midi/ReloadMessage.h"
#include "../midi/SettingsMessage.h"
#include "../midi/StatusMessage.h"
#include "../midi/VersionMessage.h"

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
            return (this->panelIsSound ? "panel positions = sound (manual mode)" : "panel positions (not the sound)") + edits;
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
    this->pendingWrites.clear();
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
    this->midiHandler.cancelExchanges();
    this->connectionState = ConnectionState::DISCONNECTED;
    this->channels.readFromSynth = false;
    this->pointer = Pointer();
    this->provenance = Provenance();
    this->activity.clear();
    notify();
}

//==============================================================================
bool SynthSession::canStartAction() const
{
    return isConnected() && !isBusy() && !this->midiHandler.isBackgroundSending();
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

        // until the synth shows the new pointer, a settings change must not write the old one back
        const double deadline = juce::Time::getMillisecondCounterHiRes() + WRITE_CONFIRM_WINDOW_MS;
        this->pendingWrites[Pro800Settings::PRESET_NUM] = { patched->getValue (Pro800Settings::PRESET_NUM), deadline };
        this->pendingWrites[Pro800Settings::CURRENT_BANK] = { patched->getValue (Pro800Settings::CURRENT_BANK), deadline };

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
    this->pendingWrites[setting] = { value, juce::Time::getMillisecondCounterHiRes() + WRITE_CONFIRM_WINDOW_MS };

    const juce::String name = PRO800_SETTINGS_FIELDS.at (setting).name;
    writeSettings (std::make_shared<SettingsMessage> (*this->settings), [this, name] (bool accepted) {
        if (!accepted)
        {
            fail ("The synth did not accept the new " + name + ".");
        }
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

void SynthSession::notePanelMirrored (bool panelIsSound)
{
    this->provenance = Provenance();
    this->provenance.basis = Provenance::Basis::PANEL;
    this->provenance.panelIsSound = panelIsSound;
    notify();
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
            if (++this->pollFailures >= 2 && this->pointer.freshness == PointerFreshness::CONFIRMED)
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
    if (newSettings != this->settings)
    {
        protectPendingWrites (*newSettings);
    }
    this->settings = newSettings;
    this->channels.rx = Pro800ChannelResolver::resolveRx (newSettings->getValue (Pro800Settings::MIDI_RX_CHANNEL), dipSum);
    this->channels.tx = Pro800ChannelResolver::resolveTx (newSettings->getValue (Pro800Settings::MIDI_TX_CHANNEL), dipSum);
    this->channels.readFromSynth = true;
    applyChannels();
}

void SynthSession::protectPendingWrites (SettingsMessage& newSettings)
{
    const double now = juce::Time::getMillisecondCounterHiRes();

    for (auto it = this->pendingWrites.begin(); it != this->pendingWrites.end();)
    {
        const auto& [setting, pending] = *it;

        if (newSettings.getValue (setting) == pending.value)
        {
            it = this->pendingWrites.erase (it); // the synth shows it: confirmed
        }
        else if (now >= pending.deadline)
        {
            juce::Logger::writeToLog ("[WARNING] SynthSession: the synth never showed the new " + juce::String (PRO800_SETTINGS_FIELDS.at (setting).name)
                                      + " (" + juce::String (pending.value) + "); it reads " + juce::String (newSettings.getValue (setting)));
            it = this->pendingWrites.erase (it);
        }
        else
        {
            // a read within the synth's commit lag: keep what we wrote, or the next settings change would write the old value back
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

void SynthSession::setActivity (const juce::String& newActivity)
{
    this->activity = newActivity;
    notify();
}

void SynthSession::fail (const juce::String& error)
{
    juce::Logger::writeToLog ("[WARNING] SynthSession: " + error);
    this->lastError = error;
    notify();
}

void SynthSession::notify()
{
    this->listeners.call ([] (Listener& l) { l.synthSessionChanged(); });
}

//==============================================================================
void SynthSession::timerCallback()
{
    if (!isConnected() || isBusy() || this->midiHandler.isExchangeBusy() || this->midiHandler.isBackgroundSending()
        || this->midiHandler.channelVoiceSentWithin (CHANNEL_VOICE_QUIET_MS))
    {
        return;
    }

    poll (true);
}

void SynthSession::channelVoiceSent (const juce::MidiMessage& message)
{
    if (message.isController() && !this->provenance.editedInApp)
    {
        this->provenance.editedInApp = true;
        notify();
    }
}

void SynthSession::channelVoiceReceived (const juce::MidiMessage& message)
{
    if (message.isController() && !this->provenance.editedOnSynth)
    {
        this->provenance.editedOnSynth = true;
        notify();
    }
}
