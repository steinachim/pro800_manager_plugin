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

#pragma once

#include "../session/SynthSession.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

/**
 * One line, visible on every tab, that says what the plugin knows: the preset the synth is on, what the
 * Front Panel / Performance controls currently represent, and what the session is doing or what went wrong.
 * Hosts the Revert button, the counterpart of Load (stored record -> sound).
 */
class StatusStrip : public juce::Component, private SynthSession::Listener
{
public:
    explicit StatusStrip (SynthSession& session);
    ~StatusStrip() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    /** Called when the user wants to fix a MIDI channel the synth does not listen on (OFF / invalid): show the Settings tab. */
    std::function<void()> onFixChannel;

private:
    void synthSessionChanged() override;
    void revert();

    SynthSession& synthSession;

    juce::Label label_Preset;
    juce::Label label_Controls;
    juce::TextButton button_Revert { "Revert" };
    juce::Label label_Status;
    juce::TextButton button_FixChannel { "Fix in Settings" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatusStrip)
};
