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

#include "StatusStrip.h"

StatusStrip::StatusStrip (SynthSession& session) : synthSession (session)
{
    label_Preset.setTooltip ("The preset the synth's own settings say it is on. Followed every few seconds; also changes when you select a preset on the synth.");
    label_Controls.setTooltip ("What the Front Panel and Performance controls show. The Pro-800 cannot report the sound it is playing: "
                               "'as stored' means the stored preset was just loaded or reverted to, '+ edits' that something changed since.");

    button_Revert.setTooltip ("Makes the synth reload the stored preset it is on, discarding unsaved edits - like reselecting it on the synth. "
                              "Does not change the synth's panel mode.");
    button_Revert.onClick = [this] { revert(); };

    button_FixChannel.onClick = [this] {
        if (onFixChannel)
        {
            onFixChannel();
        }
    };

    addAndMakeVisible (label_Preset);
    addAndMakeVisible (label_Controls);
    addAndMakeVisible (button_Revert);
    addAndMakeVisible (label_Status);
    addChildComponent (button_FixChannel);

    synthSession.addListener (this);
    synthSessionChanged();
}

StatusStrip::~StatusStrip()
{
    synthSession.removeListener (this);
}

void StatusStrip::paint (juce::Graphics& g)
{
    // one band across the whole window, so the strip reads as a single line of state rather than a few loose labels
    g.setColour (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId).brighter (0.08f));
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 4.0f);
}

void StatusStrip::resized()
{
    auto area = getLocalBounds().reduced (6, 0);

    // buttons at the right edge
    const int buttonWidth = 130;
    button_Revert.setBounds (area.removeFromRight (buttonWidth).reduced (0, 4));
    if (button_FixChannel.isVisible())
    {
        area.removeFromRight (8);
        button_FixChannel.setBounds (area.removeFromRight (buttonWidth).reduced (0, 4));
    }
    area.removeFromRight (12);

    const auto widthOf = [] (const juce::Label& label) {
        return juce::GlyphArrangement::getStringWidthInt (label.getFont(), label.getText()) + label.getBorderSize().getLeftAndRight() + 4;
    };

    // the preset at the left and the status text (if any) right before the buttons, each as wide as its text
    label_Preset.setBounds (area.removeFromLeft (juce::jmin (area.getWidth() / 3, widthOf (label_Preset))));
    area.removeFromLeft (16);

    const int statusWidth = label_Status.getText().isEmpty() ? 0 : juce::jmin (area.getWidth() - 200, widthOf (label_Status));
    label_Status.setBounds (area.removeFromRight (juce::jmax (0, statusWidth)));
    area.removeFromRight (16);

    // the controls label centred on the strip, or on what is left when that would collide
    const int controlsWidth = juce::jmin (area.getWidth(), widthOf (label_Controls));
    auto controlsBounds = juce::Rectangle<int> (controlsWidth, area.getHeight()).withCentre (getLocalBounds().getCentre());
    if (!area.contains (controlsBounds))
    {
        controlsBounds = area.withSizeKeepingCentre (controlsWidth, area.getHeight());
    }
    label_Controls.setBounds (controlsBounds);
}

void StatusStrip::synthSessionChanged()
{
    using Freshness = SynthSession::PointerFreshness;
    using Kind = Pro800MidiChannel::Kind;

    const bool connected = synthSession.isConnected();
    const auto& pointer = synthSession.getPointer();
    const auto& provenance = synthSession.getProvenance();
    const auto& channels = synthSession.getChannels();

    // the preset
    juce::String presetText;
    juce::Colour presetColour = getLookAndFeel().findColour (juce::Label::textColourId);
    if (!connected)
    {
        presetText = "Preset: -";
    }
    else if (pointer.freshness == Freshness::PENDING)
    {
        presetText = "Preset: " + pointer.label() + "...";
    }
    else if (pointer.freshness == Freshness::UNCONFIRMED)
    {
        presetText = "Preset: " + pointer.label() + "? - not confirmed";
        presetColour = juce::Colours::orange;
    }
    else if (pointer.freshness == Freshness::UNKNOWN)
    {
        presetText = "Preset: " + pointer.label() + " (not reachable)";
        presetColour = juce::Colours::grey;
    }
    else
    {
        presetText = "Preset: " + pointer.label() + (pointer.name.empty() ? juce::String() : " \"" + juce::String (pointer.name) + "\"");
    }
    label_Preset.setText (presetText, juce::dontSendNotification);
    label_Preset.setColour (juce::Label::textColourId, presetColour);

    // the controls
    juce::String controlsText = "Controls: " + (connected ? provenance.describe() : juce::String ("unknown"));
    if (connected && provenance.basis == SynthSession::Provenance::Basis::UNKNOWN)
    {
        controlsText += " - Load or Revert to sync";
    }
    label_Controls.setText (controlsText, juce::dontSendNotification);
    label_Controls.setColour (juce::Label::textColourId,
        provenance.basis == SynthSession::Provenance::Basis::PANEL && !provenance.panelIsSound ? juce::Colours::orange
                                                                                               : getLookAndFeel().findColour (juce::Label::textColourId));

    // revert: only when a stored preset is known to point at
    button_Revert.setButtonText (pointer.program.has_value() ? "Revert to " + pointer.label() : "Revert");
    button_Revert.setEnabled (synthSession.canStartAction() && pointer.program.has_value() && pointer.freshness == Freshness::CONFIRMED);

    // activity, or the channel problem, or the last error
    juce::String statusText;
    juce::Colour statusColour = getLookAndFeel().findColour (juce::Label::textColourId);
    bool showFix = false;

    if (synthSession.isBusy())
    {
        statusText = synthSession.getActivity();
    }
    else if (connected && channels.automatic && channels.readFromSynth && !channels.sendChannel().has_value())
    {
        statusText = channels.rx.kind == Kind::OFF
                         ? "The synth's MIDI channel is OFF: notes and CC are ignored."
                         : (channels.rx.kind == Kind::DIP
                                   ? "The synth's MIDI channel comes from its DIP switches, which could not be read."
                                   : "The synth holds an invalid MIDI channel (" + juce::String (channels.rx.raw) + ") and ignores MIDI.");
        statusColour = juce::Colours::orange;
        showFix = channels.rx.kind != Kind::DIP;
    }
    else if (synthSession.getLastError().isNotEmpty())
    {
        statusText = synthSession.getLastError();
        statusColour = juce::Colours::orange;
    }

    label_Status.setText (statusText, juce::dontSendNotification);
    label_Status.setColour (juce::Label::textColourId, statusColour);
    label_Status.setJustificationType (juce::Justification::centredRight);

    button_FixChannel.setVisible (showFix);
    resized(); // the label widths follow their text
}

void StatusStrip::revert()
{
    const juce::String label = synthSession.getPointer().label();

    juce::AlertWindow::showOkCancelBox (juce::MessageBoxIconType::QuestionIcon,
        "Reload " + label + " from memory?",
        "Unsaved changes to the sound on the synth are lost - the same as reselecting the preset on the synth.",
        "Revert",
        "Cancel",
        this,
        juce::ModalCallbackFunction::create ([safeThis = juce::Component::SafePointer<StatusStrip> (this)] (int result) {
            if (result == 1 && safeThis != nullptr)
            {
                safeThis->synthSession.reloadStoredProgram();
            }
        }));
}
