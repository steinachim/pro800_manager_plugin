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

#include "MidiComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

class MidiHandler;

class AdvancedTab : public juce::Component, public MidiComponent
{
public:
    AdvancedTab (MidiHandler* midiHandler, SynthSession& synthSession);
    virtual ~AdvancedTab() override;

    void resized() override;
    void handleMidiLog (const juce::MidiMessage& message, const juce::String& logPrefix, bool isPolling) override;
    void handlePro800VersionUpdate() override;

private:
    // the log keeps at most this many lines (oldest are dropped); a program dump is ~100 lines
    static constexpr int MAX_LOG_LINES = 20000;

    void sendInputMessage();
    void logPanelReading();
    void addLogMessage (const juce::String& newMessage);

    juce::ComboBox combo_PreparedMessages;
    juce::TextEditor textEdit_inputMidiMessage;

    // CodeEditorComponent instead of TextEditor: it is line-based and only lays out the visible
    // lines, so appending stays cheap no matter how long the log gets
    juce::CodeDocument logDocument;
    juce::CodeEditorComponent codeEditor_midiMessageLog { logDocument, nullptr };

    juce::TextButton button_sendMessage;
    juce::TextButton button_readPanel { "Read Panel to Log" };

    juce::ToggleButton checkBox_enableLogging { "Enable Logging" };
    juce::ToggleButton checkBox_hidePolling { "Hide Polling" };
    juce::TextButton button_clearLog { "Clear Log" };

    // off by default and reset on every connect: the named hazards (Pro800Hazards) are refused unless it is on
    juce::ToggleButton checkBox_allowHazards { "Allow hazardous messages" };

#if JUCE_DEBUG
    // developer aid for probing unknown SysEx commands, not part of the release UI
    juce::TextButton button_debug;
    juce::Slider slider_debugInput { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft };
#endif

    enum PreparedMessageId {
        Custom = 1,
        DumpProgram,
        RequestVersion,
        RequestDeviceName,
        RequestProductCode,
        GetSettings,
        ReloadPreset,
        ReadPanel,
        ReadKnob,
        SetKnob,
        PressButton,
        LockPanel,
        UnlockPanel,
        LightsOff,
        LightsOn,
        PitchBendRange,
        NoteOn,
        NoteOff
    };

    struct PreparedMessage
    {
        juce::String name;
        juce::String byteString;
        juce::String description;
    };

    // clang-format off
        static inline const std::map<PreparedMessageId, PreparedMessage> PREPARED_MESSAGES = {
            {Custom,             {"Custom", "", "Enter your own command"}},
            {DumpProgram,        {"Dump Program", "F0 00 20 32 00 01 24 00 77 XX XX F7", "Replace XX XX with the program number (7-bit LSB, MSB). An empty slot answers F0 F7, an address above 399 a failure status"}},
            {RequestVersion,     {"Request Version", "F0 00 20 32 00 01 24 00 08 00 F7", "Report the firmware version, also checks compatibility"}},
            {RequestDeviceName,  {"Request Device Name", "F0 00 20 32 00 01 24 00 06 F7", "Answers with 'PRO-800'"}},
            {RequestProductCode, {"Request Product Code", "F0 00 20 32 00 01 24 00 04 F7", "Answers with Behringer's product code 'P0E9I'"}},
            {GetSettings,        {"Get Settings", "F0 00 20 32 00 01 24 00 77 7E 03 F7", "Get the settings block (address 510)"}},
            {ReloadPreset,       {"Reload Stored Preset", "F0 00 20 32 00 01 24 00 32 00 F7", "Recalls the preset the settings block points at and discards unsaved edits (what Revert does). Only parameter 00 is safe!"}},
            {ReadPanel,          {"Read Panel State", "F0 00 20 32 00 01 24 00 70 XX F7", "Reads one physical control (answer: 71 XX VV):\n00-14 = buttons, 1 only while held (same codes as Press Button)\n18 Osc A Rect, 19 Osc B Rect\n1A/1B Filter Keyboard Full/Half\n1C LFO Shape (1 = Tri/Sine/Saw)\n1D/1E/1F LFO Dest Freq/PW/Filter\n20 Osc A Saw, 21 Osc A Tri, 22 Osc A Sync\n23 Osc B Saw, 24 Osc B Tri\n25/26 Poly-Mod Dest Freq A/Filter\n27 Poly-Mod Unison Track\n28-2B DIP switches 1-4 (weights 1/2/4/8, sum = MIDI channel - 1)"}},
            {ReadKnob,           {"Read Knob Position", "F0 00 20 32 00 01 24 00 72 XX F7", "Reads one knob's physical position, 0-127 (answer: 73 XX VV). Not the preset: knobs do not move when one is loaded.\n00 Amp Release, 01 Amp Sustain, 02 Amp Decay, 03 Amp Attack\n04 Filter Release, 05 Filter Sustain, 06 Filter Decay, 07 Filter Attack\n08 Filter Resonance, 09 Filter Env Amount\n0A Osc A Freq, 0B Osc B Freq, 0C Osc B Fine, 0D Osc A PW, 0E Osc B PW, 0F Osc A Level\n10 Glide, 11 LFO Freq, 12 LFO Initial Amount (does not round-trip), 13 Master Tune, 14 Master Volume\n15 Osc B Level, 16 Poly-Mod Osc B, 17 Poly-Mod Filter Env, 18 Noise, 19 Filter Cutoff"}},
            {SetKnob,            {"Set Knob Position", "F0 00 20 32 00 01 24 00 73 XX VV F7", "Sets one knob's value (0-127) in the edit buffer: audible at once, invisible to a program read until the preset is saved. Indices as for Read Knob Position"}},
            {PressButton,        {"Press Button", "F0 00 20 32 00 01 24 00 71 XX F7", "Emulate a button press:\n00-09 = 0-9\n0A = ARP UP-DN\n0B = ARP ASSIGN\n0C = PRESET (also recalls the preset)\n0D = REC\n0E = PERF\n0F = SETTINGS\n10 = SEQ1\n11 = SEQ2\n12 = TUNE\n13 = SYNC CLOCK\n14 = SYNC SOURCE\n16/17 = value wheel: steps the preset selection in preset mode!\nREC, two digits, PRESET saves the edit buffer into that slot of the current bank"}},
            {LockPanel,          {"Lock Front Panel", "F0 00 20 32 00 01 24 00 36 00 F7", "Locks the synth's front panel (36 00); any other parameter unlocks it"}},
            {UnlockPanel,        {"Unlock Front Panel", "F0 00 20 32 00 01 24 00 36 01 F7", "Unlocks the synth's front panel"}},
            {LightsOff,          {"Button Lights Off", "F0 00 20 32 00 01 24 00 37 00 F7", "Turns every button light off (37 00); any other parameter lights them all"}},
            {LightsOn,           {"Button Lights On", "F0 00 20 32 00 01 24 00 37 01 F7", "Lights every button"}},
            {PitchBendRange,     {"Pitch Bend Range", "F0 00 20 32 00 01 24 00 11 NN F7", "Sets the current preset's pitch bend range to NN semitones (00-1F) in the edit buffer; the same thing CC 42 does"}},
            {NoteOn,             {"Note On", "9[channel] KK VV", "Send note-on"}},
            {NoteOff,            {"Note Off", "8[channel] KK VV", "Send note-off"}}
        };
    // clang-format on

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdvancedTab)
};
