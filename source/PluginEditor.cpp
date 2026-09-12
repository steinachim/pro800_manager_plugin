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
    : AudioProcessorEditor (&p), MidiComponent (handler, false, { MessageType::PRO800_VERSION })
{
    // midi connection area
    button_ConnectMidi.onClick = [this] { connectMidiDevices(); };
    button_RefreshMidi.onClick = [this] { refreshMidiDeviceLists(); };

    spinBox_MidiChannel.setRange (1, 16, 1);
    spinBox_MidiChannel.setValue (1);
    getMidiHandler().setMidiChannel (1);

    spinBox_MidiChannel.onValueChange = [this] {
        uint8_t channel = (uint8_t) spinBox_MidiChannel.getValue();
        this->keyboardPanel.setMidiChannel (channel);
        getMidiHandler().setMidiChannel (channel);
    };

    addAndMakeVisible (label_FirmwareVersion);
    addAndMakeVisible (label_MidiChannel);
    addAndMakeVisible (spinBox_MidiChannel);
    addAndMakeVisible (label_MidiInput);
    addAndMakeVisible (combo_MidiInputList);
    addAndMakeVisible (label_MidiOutput);
    addAndMakeVisible (combo_MidiOutputList);
    addAndMakeVisible (button_RefreshMidi);
    addAndMakeVisible (button_ConnectMidi);

    // main widget
    tabBar = std::make_unique<MainWidget> (handler);
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
}

Pro800ManagerEditor::~Pro800ManagerEditor()
{
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
    button_ConnectMidi.setBounds (midiArea.removeFromRight (100).reduced (0, 4));
    button_RefreshMidi.setBounds (midiArea.removeFromRight (100).reduced (0, 4));

    combo_MidiOutputList.setBounds (midiArea.removeFromRight (200).reduced (0, 4));
    label_MidiOutput.setBounds (midiArea.removeFromRight (80));
    combo_MidiInputList.setBounds (midiArea.removeFromRight (200).reduced (0, 4));
    label_MidiInput.setBounds (midiArea.removeFromRight (80));
    spinBox_MidiChannel.setBounds (midiArea.removeFromRight (120).reduced (0, 4));
    label_MidiChannel.setBounds (midiArea.removeFromRight (100));

    label_FirmwareVersion.setBounds (midiArea);

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
    sendMidiMessage (juce::MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
}

void Pro800ManagerEditor::handleNoteOff (juce::MidiKeyboardState* /*source*/, int midiChannel, int midiNoteNumber, float velocity)
{
    sendMidiMessage (juce::MidiMessage::noteOff (midiChannel, midiNoteNumber, velocity));
}

void Pro800ManagerEditor::refreshMidiDeviceLists()
{
    combo_MidiInputList.setDevices (juce::MidiInput::getAvailableDevices());
    combo_MidiOutputList.setDevices (juce::MidiOutput::getAvailableDevices());
}

void Pro800ManagerEditor::connectMidiDevices()
{
    label_FirmwareVersion.setText ("Not Connected", juce::NotificationType::dontSendNotification);

    getMidiHandler().connectMidiDevices (combo_MidiInputList.getSelectedDeviceIdentifier(),
        combo_MidiOutputList.getSelectedDeviceIdentifier());
}

void Pro800ManagerEditor::handlePro800VersionUpdate()
{
    std::shared_ptr<VersionMessage> versionMessage = getCurrentVersion();
    if (!versionMessage || !versionMessage->isValid())
    {
        return;
    }

    if (!this->getCurrentVersion()->isSupported())
    {
        juce::String messageText = "The firmware version of your Pro-800 is not supported. Proceed with extreme caution!\n\n";
        messageText = messageText + "Installed version:  " + this->getCurrentVersion()->getVersionString() + "\n";
        messageText = messageText + "Supported versions: " + this->getCurrentVersion()->getSupportedVersions();

        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Unsupported Firmware Version", messageText);
    }

    label_FirmwareVersion.setText ("Firmware Version: " + versionMessage->getVersionString(), juce::NotificationType::dontSendNotification);
}