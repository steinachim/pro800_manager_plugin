/*
  ==============================================================================

    SettingsTab.h
    Created: 9 Sep 2025 11:12:08am
    Author:  Achim Stein

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiComponent.h"

#include "EqualSpacingGroupComponent.h"

class SettingsTab : public juce::Component, public MidiComponent
{
    public:
        SettingsTab(MidiHandler *midiHandler);
        ~SettingsTab() override;
        virtual void handlePro800SettingsUpdate() override;
        virtual void handlePro800VersionUpdate() override;

    
        void resized() override;

private:
    void setupGroupConnections();
    void setupGroupTranspose();
    void setupGroupPresetDump();
    void setupGroupVoices();
    void setupGroupTuning();
    void setupGroupRetuneEncoder();
    void setupGroupDisplay();
    void setupGroupAutoTune();
    void setupGroupMiscellaneous();
    void setupGroupSync();
    void setupGroupFactoryReset();

    void setSettingsGroupsEnabled(bool enable);

    void setupSettingsComponent(Pro800Settings setting, juce::Component *component);
    juce::HashMap<Pro800Settings, juce::Component*> settingsListeners;

    juce::Label label_FirmwareVersion;
    juce::TextButton button_Reconnect { "Reconnect"};
    juce::TextButton button_RefreshSettings { "Refresh Settings" };

    EqualSpacingGroupComponent group_Connections { "1 - Connections", 255, 10, 2 };
    juce::Label label_ConnectionsMidiInputChannel {"", "MIDI Input Channel"};
    juce::ComboBox combo_ConnectionsMidiInputChannel;
    juce::Label label_ConnectionsMidiOutputChannel {"", "MIDI Output Channel"};
    juce::ComboBox combo_ConnectionsMidiOutputChannel;
    juce::Label label_ConnectionsMidiCC {"", "MIDI CC Mode"};
    juce::ComboBox combo_ConnectionsMidiCC;
    juce::Label label_ConnectionsMidiPC {"", "MIDI PC Mode"};
    juce::ComboBox combo_ConnectionsMidiPC;
    juce::Label label_ConnectionsSyncInPolarity {"", "Sync In Polarity"};
    juce::ComboBox combo_ConnectionsSyncInPolarity;
    juce::Label label_ConnectionsSyncInPPQN {"", "Sync In PPQN"};
    juce::ComboBox combo_ConnectionsSyncInPPQN;
    juce::Label label_ConnectionsSyncItForwardEnabled {"", "Sync In Forward"};
    juce::ToggleButton checkBox_ConnectionsSyncInForwardEnabled;
    juce::Label label_ConnectionsSyncInStartStopEnabled {"", "Sync In Start-Stop"};
    juce::ToggleButton checkBox_ConnectionsSyncInStartStopEnabled;
    juce::Label label_ConnectionsLocalEnable {"", "Local Enable"};
    juce::ToggleButton checkBox_ConnectionsLocalEnable;
    juce::Label label_ConnectionsSoftThru {"", "Soft Thru"};
    juce::ToggleButton checkBox_ConnectionsSoftThru;

    EqualSpacingGroupComponent group_Transpose { "2 - Transpose", 255, 10, 2 };
    juce::Label label_TransposeAmount {"", "Transpose Amount"};
    juce::Slider spinBox_TransposeAmount { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    EqualSpacingGroupComponent group_PresetDump { "3 - Preset Dump", 255, 10 };
    juce::TextButton button_PresetDump { "Request Dump" };

    EqualSpacingGroupComponent group_Voices { "4 - Voices", 255, 5, 2};
    juce::Label label_VoiceKill { "", "Voice Enable:" };
    juce::ToggleButton checkBox_Voice[8];

    EqualSpacingGroupComponent group_Tuning { "5 - Tuning", 255, 5, 2 };
    juce::Label label_TuningRetuneElement {"", "Retune Element"};
    juce::ComboBox combo_TuningRetuneElement;
    juce::Label label_TuningRetuneOctave {"", "Retune Octave"};
    juce::ComboBox combo_TuningRetuneOctave;

    EqualSpacingGroupComponent group_RetuneEncoder { "6 - Retune Encoder (selected in 5)", 255, 5, 2};
    juce::Label label_RetuneTuning {"", "Tuning"};
    juce::Slider spinBox_RetuneTuning { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    EqualSpacingGroupComponent group_Display { "7 - Display", 255, 3, 2 };
    juce::Label label_DisplayBrightness {"", "Brightness"};
    juce::Slider spinBox_DisplayBrightness { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Label label_DisplayParameterTime {"", "Display Parameter Time"};
    juce::Slider spinBox_DisplayParameterTime { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Label label_DisplayPresetNameEnabled {"", "Show Preset Names"};
    juce::ToggleButton checkBox_DisplayPresetNameEnabled;

    EqualSpacingGroupComponent group_AutoTune { "8 - Auto Tune", 255, 3, 2 };
    juce::Label label_AutoTunePrecision {"", "Precision"};
    juce::ComboBox combo_AutoTunePrecision;

    EqualSpacingGroupComponent group_Miscellaneous { "9 - Miscellaneous", 255, 3, 2 };
    juce::Label label_MiscExternalFilterModAmount {"", "External Filter Mod Amount"};
    juce::Slider spinBox_MiscExternalFilterModAmount { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Label label_MiscVoicePriority {"", "Voice Priority"};
    juce::ComboBox combo_MiscVoicePriority;
    juce::Label label_MiscPedalPriority {"", "Pedal Priority"};
    juce::ComboBox combo_MiscPedalPriority;

    EqualSpacingGroupComponent group_Sync { "Sync", 255, 5, 2 };
    juce::Label label_SyncClockBPM {"", "Clock BPM"};
    juce::Slider spinBox_SyncClockBPM { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Label label_SyncClockSwing {"", "Clock Swing"};
    juce::Slider spinBox_SyncClockSwing { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Label label_SyncClockNoteLength  {"", "Clock Note Length"};
    juce::Slider spinBox_SyncClockNoteLength { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Label label_SyncSource {"", "Source"};
    juce::ComboBox combo_SyncSource;
    juce::Label label_SyncClockSubdivision {"", "Clock Subdivision"};
    juce::ComboBox combo_SyncClockSubdivision;

    EqualSpacingGroupComponent group_FactoryReset { "0 - Factory Reset", 255, 5 };
    juce::TextButton button_FactoryReset { "Factory Reset" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsTab)
};
