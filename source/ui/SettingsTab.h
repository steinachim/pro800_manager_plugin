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
#include "TwoColumnGroupComponent.h"

class EqualSpacingGroupComponent;

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
    juce::TextButton button_RefreshSettings { "Refresh Settings" };

    TwoColumnGroupComponent group_Connections { "1 - Connections", 10 };
    juce::ComboBox combo_ConnectionsMidiInputChannel;
    juce::ComboBox combo_ConnectionsMidiOutputChannel;
    juce::ComboBox combo_ConnectionsMidiCC;
    juce::ComboBox combo_ConnectionsMidiPC;
    juce::ComboBox combo_ConnectionsSyncInPolarity;
    juce::ComboBox combo_ConnectionsSyncInPPQN;
    juce::ToggleButton checkBox_ConnectionsSyncInForwardEnabled;
    juce::ToggleButton checkBox_ConnectionsSyncInStartStopEnabled;
    juce::ToggleButton checkBox_ConnectionsLocalEnable;
    juce::ToggleButton checkBox_ConnectionsSoftThru;

    TwoColumnGroupComponent group_Transpose { "2 - Transpose", 10 };
    juce::Slider spinBox_TransposeAmount { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    EqualSpacingGroupComponent group_PresetDump { "3 - Preset Dump", 255 };
    juce::TextButton button_PresetDump { "Request Dump" };

    TwoColumnGroupComponent group_Voices { "4 - Voices", 5 };
    juce::Label label_VoiceKill { "", "Voice Enable:" };
    juce::ToggleButton checkBox_Voice[8];

    TwoColumnGroupComponent group_Tuning { "5 - Tuning", 5 };
    juce::ComboBox combo_TuningRetuneElement;
    juce::ComboBox combo_TuningRetuneOctave;

    TwoColumnGroupComponent group_RetuneEncoder { "6 - Retune Encoder (selected in 5)", 5};
    juce::Slider spinBox_RetuneTuning { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};

    TwoColumnGroupComponent group_Display { "7 - Display", 3 };
    juce::Slider spinBox_DisplayBrightness { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Slider spinBox_DisplayParameterTime { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::ToggleButton checkBox_DisplayPresetNameEnabled;

    TwoColumnGroupComponent group_AutoTune { "8 - Auto Tune", 3 };
    juce::ComboBox combo_AutoTunePrecision;

    TwoColumnGroupComponent group_Miscellaneous { "9 - Miscellaneous", 3 };
    juce::Slider spinBox_MiscExternalFilterModAmount { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::ComboBox combo_MiscVoicePriority;
    juce::ComboBox combo_MiscPedalPriority;

    TwoColumnGroupComponent group_Sync { "Sync", 5 };
    juce::Slider spinBox_SyncClockBPM { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Slider spinBox_SyncClockSwing { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::Slider spinBox_SyncClockNoteLength { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft};
    juce::ComboBox combo_SyncSource;
    juce::ComboBox combo_SyncClockSubdivision;

    EqualSpacingGroupComponent group_FactoryReset { "0 - Factory Reset", 255 };
    juce::TextButton button_FactoryReset { "Factory Reset" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsTab)
};
