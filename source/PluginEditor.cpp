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

#include "PluginEditor.h"
#include "PluginProcessor.h"

#include "midi/MidiHandler.h"
#include "midi/VersionMessage.h"

Pro800ManagerEditor::Pro800ManagerEditor (MidiHandler* handler, Pro800ManagerAudioProcessor& p)
    : AudioProcessorEditor (&p), MidiComponent (handler, p.getSynthSession(), false), statusStrip (p.getSynthSession())
{
    // midi connection area
    button_ConnectMidi.onClick = [this] { connectMidiDevices(); };
    button_RefreshMidi.onClick = [this] { refreshMidiDeviceLists(); };

    // Send: the channel the synth listens on (its MIDI RX Channel setting, resolved through the DIP switches if need be).
    // Receive: the channel the synth transmits its knob movements on (its MIDI TX Channel setting).
    combo_MidiChannel.addItem ("Auto", CHANNEL_ITEM_AUTO);
    combo_ReceiveChannel.addItem ("Auto", CHANNEL_ITEM_AUTO);
    combo_ReceiveChannel.addItem ("All", CHANNEL_ITEM_ALL);
    for (int channel = 1; channel <= 16; channel++)
    {
        combo_MidiChannel.addItem (juce::String (channel), channel);
        combo_ReceiveChannel.addItem (juce::String (channel), channel);
    }
    label_MidiInput.setTooltip ("MIDI input port: the synth's USB port, pre-selected when it is plugged in");
    label_MidiOutput.setTooltip ("MIDI output port: the synth's USB port, pre-selected when it is plugged in");
    label_MidiChannel.setTooltip ("The MIDI channel notes and CC are sent to the synth on: its MIDI RX Channel setting");
    label_ReceiveChannel.setTooltip ("The MIDI channel knob movements are accepted from the synth on: its MIDI TX Channel setting");
    combo_MidiChannel.setTooltip ("Auto reads the synth's MIDI RX Channel after connecting; pick a number only if that fails or you know better.");
    combo_ReceiveChannel.setTooltip ("Auto reads the synth's MIDI TX Channel after connecting; pick a number (or All) only if that fails or you know better.");
    combo_MidiChannel.onChange = [this] {
        const int id = combo_MidiChannel.getSelectedId();
        getSynthSession().setManualChannel (id == CHANNEL_ITEM_AUTO ? std::nullopt : std::optional<int> (id));
    };
    combo_ReceiveChannel.onChange = [this] {
        const int id = combo_ReceiveChannel.getSelectedId();
        getSynthSession().setManualReceiveChannel (id == CHANNEL_ITEM_AUTO ? std::nullopt : std::optional<int> (id == CHANNEL_ITEM_ALL ? 0 : id));
    };

    label_Connection.setTooltip ("Connect: identifies the synth, reads its settings and follows the preset it is on");

    statusStrip.onFixChannel = [this] {
        if (tabBar != nullptr)
        {
            tabBar->setCurrentTabIndex (2); // Settings
        }
    };

    addAndMakeVisible (label_Connection);
    addAndMakeVisible (label_MidiChannel);
    addAndMakeVisible (combo_MidiChannel);
    addAndMakeVisible (label_ReceiveChannel);
    addAndMakeVisible (combo_ReceiveChannel);
    addAndMakeVisible (label_MidiInput);
    addAndMakeVisible (combo_MidiInputList);
    addAndMakeVisible (label_MidiOutput);
    addAndMakeVisible (combo_MidiOutputList);
    addAndMakeVisible (button_RefreshMidi);
    addAndMakeVisible (button_ConnectMidi);
    addAndMakeVisible (statusStrip);

    // main widget
    tabBar = std::make_unique<MainWidget> (handler, getSynthSession());
    addAndMakeVisible (tabBar.get());

    // keyboard at the bottom
    keyboardState.addListener (this);

    keyboardPanel.setKeyPressBaseOctave (3);
    keyboardPanel.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    keyboardPanel.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, getLookAndFeel().findColour (juce::Slider::thumbColourId));

    button_ShowHideKeyboard.setAlwaysOnTop (true);
    button_ShowHideKeyboard.onClick = [this] {
        keyboardPanel.setVisible (!keyboardPanel.isVisible());
        button_ShowHideKeyboard.setButtonText (keyboardPanel.isVisible() ? "Hide Keyboard" : "Show Keyboard");
        resized();
    };

    addAndMakeVisible (keyboardPanel);
    addAndMakeVisible (button_ShowHideKeyboard);

    setSize (1400, 900);
    setResizable (true, true);
    // below ~1100 x 700 the MIDI bar and the front panel groups no longer fit; the maximum just keeps the layout sane on huge screens
    setResizeLimits (1100, 700, 3840, 2400);

    refreshMidiDeviceLists();

    getSynthSession().addListener (this);
    synthSessionChanged();
}

Pro800ManagerEditor::~Pro800ManagerEditor()
{
    getSynthSession().removeListener (this);
    keyboardState.removeListener (this);
}

void Pro800ManagerEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void Pro800ManagerEditor::resized()
{
    auto area = getLocalBounds().reduced (4);

    // MIDI connection area
    auto midiArea = area.removeFromTop (30);
    button_ConnectMidi.setBounds (midiArea.removeFromRight (90).reduced (0, 4));
    button_RefreshMidi.setBounds (midiArea.removeFromRight (90).reduced (0, 4));

    combo_MidiOutputList.setBounds (midiArea.removeFromRight (170).reduced (0, 4));
    label_MidiOutput.setBounds (midiArea.removeFromRight (40));
    combo_MidiInputList.setBounds (midiArea.removeFromRight (170).reduced (0, 4));
    label_MidiInput.setBounds (midiArea.removeFromRight (32));
    midiArea.removeFromRight (12);
    combo_ReceiveChannel.setBounds (midiArea.removeFromRight (180).reduced (0, 4));
    label_ReceiveChannel.setBounds (midiArea.removeFromRight (65));
    combo_MidiChannel.setBounds (midiArea.removeFromRight (180).reduced (0, 4));
    label_MidiChannel.setBounds (midiArea.removeFromRight (50));

    label_Connection.setBounds (midiArea);

    area.removeFromTop (4);

    // what the plugin knows about the synth
    statusStrip.setBounds (area.removeFromTop (30));
    area.removeFromTop (4);

    // tab bar
    const double mainWindowPercentage = 9.0 / 10.0;

    int tabBarHeight = (int) (mainWindowPercentage * area.getHeight());

    // keyboard at the bottom
    if (!this->keyboardPanel.isVisible())
    {
        tabBarHeight = area.getHeight() - 30;
    }

    if (this->tabBar != nullptr)
    {
        this->tabBar->setBounds (area.removeFromTop (tabBarHeight));
    }

    this->keyboardPanel.setBounds (area.removeFromLeft (area.getWidth() - 100));

    this->button_ShowHideKeyboard.setBounds (area.removeFromBottom (30));
}

void Pro800ManagerEditor::handleNoteOn (juce::MidiKeyboardState* /*source*/, int midiChannel, int midiNoteNumber, float velocity)
{
    getMidiHandler().sendChannelVoice (juce::MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
}

void Pro800ManagerEditor::handleNoteOff (juce::MidiKeyboardState* /*source*/, int midiChannel, int midiNoteNumber, float velocity)
{
    getMidiHandler().sendChannelVoice (juce::MidiMessage::noteOff (midiChannel, midiNoteNumber, velocity));
}

void Pro800ManagerEditor::refreshMidiDeviceLists()
{
    combo_MidiInputList.setDevices (juce::MidiInput::getAvailableDevices());
    combo_MidiOutputList.setDevices (juce::MidiOutput::getAvailableDevices());

    // the synth's class-compliant USB port is named after it: offer it, but let the user change it (Connect still identifies the
    // synth by its reply, not by the port name, so this is a convenience rather than a check)
    if (combo_MidiInputList.getSelectedDeviceIdentifier().isEmpty())
    {
        combo_MidiInputList.selectDeviceContaining (PRO800_PORT_NAME_FRAGMENT);
    }
    if (combo_MidiOutputList.getSelectedDeviceIdentifier().isEmpty())
    {
        combo_MidiOutputList.selectDeviceContaining (PRO800_PORT_NAME_FRAGMENT);
    }
}

void Pro800ManagerEditor::connectMidiDevices()
{
    getSynthSession().disconnect();
    getMidiHandler().connectMidiDevices (combo_MidiInputList.getSelectedDeviceIdentifier(),
        combo_MidiOutputList.getSelectedDeviceIdentifier());

    // a fresh connection derives the channels from the synth again
    combo_MidiChannel.setSelectedId (CHANNEL_ITEM_AUTO, juce::dontSendNotification);
    combo_ReceiveChannel.setSelectedId (CHANNEL_ITEM_AUTO, juce::dontSendNotification);
    getSynthSession().setManualChannel (std::nullopt);
    getSynthSession().setManualReceiveChannel (std::nullopt);
    warnedFirmwareVersion.clear();

    getSynthSession().connect();
}

void Pro800ManagerEditor::synthSessionChanged()
{
    using State = SynthSession::ConnectionState;

    // connection badge
    juce::String connectionText;
    juce::Colour connectionColour = getLookAndFeel().findColour (juce::Label::textColourId);
    switch (getSynthSession().getConnectionState())
    {
        case State::PROBING:
            connectionText = "Connecting...";
            break;
        case State::CONNECTED:
            connectionText = "Pro-800 " + getSynthSession().getFirmwareVersion();
            if (!getSynthSession().isFirmwareSupported())
            {
                connectionText += " - unsupported firmware";
                connectionColour = juce::Colours::orange;
            }
            else
            {
                connectionColour = juce::Colours::lightgreen;
            }
            break;
        case State::DISCONNECTED:
        default:
            connectionText = "Not connected";
            break;
    }
    label_Connection.setText (connectionText, juce::dontSendNotification);
    label_Connection.setColour (juce::Label::textColourId, connectionColour);

    if (getSynthSession().isConnected() && !getSynthSession().isFirmwareSupported())
    {
        warnAboutUnsupportedFirmware();
    }

    // channel combos: the Auto items name the channels the synth reports
    const auto& channels = getSynthSession().getChannels();
    combo_MidiChannel.changeItemText (CHANNEL_ITEM_AUTO, channels.automatic ? channels.describe() : juce::String ("Auto"));
    combo_MidiChannel.setSelectedId (channels.automatic ? CHANNEL_ITEM_AUTO : channels.manualChannel, juce::dontSendNotification);
    const bool synthIsDeaf = getSynthSession().isConnected() && !channels.sendChannel().has_value();
    combo_MidiChannel.setColour (juce::ComboBox::textColourId,
        synthIsDeaf ? juce::Colours::orange : getLookAndFeel().findColour (juce::ComboBox::textColourId));

    combo_ReceiveChannel.changeItemText (CHANNEL_ITEM_AUTO, channels.receiveAutomatic ? channels.describeReceive() : juce::String ("Auto"));
    combo_ReceiveChannel.setSelectedId (channels.receiveAutomatic ? CHANNEL_ITEM_AUTO : (channels.manualReceiveChannel == 0 ? CHANNEL_ITEM_ALL : channels.manualReceiveChannel),
        juce::dontSendNotification);

    if (channels.readFromSynth)
    {
        const juce::String synthChannels = "Synth settings: MIDI RX Channel " + juce::String (channels.rx.toString()) + ", MIDI TX Channel " + juce::String (channels.tx.toString());
        combo_MidiChannel.setTooltip (synthChannels);
        combo_ReceiveChannel.setTooltip (synthChannels);
    }

    // keyboard: silent while the synth would not hear it
    if (const auto channel = channels.sendChannel())
    {
        keyboardPanel.setMidiChannel (*channel);
    }
    keyboardPanel.setEnabled (!synthIsDeaf);
}

void Pro800ManagerEditor::warnAboutUnsupportedFirmware()
{
    const juce::String version = getSynthSession().getFirmwareVersion();
    if (version == warnedFirmwareVersion)
    {
        return; // once per connection
    }
    warnedFirmwareVersion = version;

    juce::String messageText = "The firmware version of your Pro-800 is not supported. Proceed with extreme caution!\n\n";
    messageText << "Installed version:  " << version << "\n";
    messageText << "Supported versions: " << juce::String (VersionMessage::supportedVersionsText());

    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Unsupported Firmware Version", messageText);
}