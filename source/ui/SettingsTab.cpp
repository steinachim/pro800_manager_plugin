/*
  ==============================================================================

    SettingsTab.cpp
    Created: 9 Sep 2025 11:12:00am
    Author:  Achim Stein

  ==============================================================================
*/

#include "SettingsTab.h"

#include "EqualSpacingGroupComponent.h"

#include "../midi/MidiHandler.h"
#include "../midi/SettingsMessage.h"
#include "../midi/VersionMessage.h"

SettingsTab::SettingsTab(MidiHandler *midiHandler) : Component(), MidiComponent(midiHandler, {Pro800MessageType::PRO800_SETTINGS_MESSAGE, Pro800MessageType::PRO800_VERSION_MESSAGE})
{
    addAndMakeVisible(label_FirmwareVersion);

    button_RefreshSettings.onClick = [midiHandler] {
        midiHandler->sendMidiMessage(SettingsMessage::request());
    };

    addAndMakeVisible(button_RefreshSettings);

    setupGroupConnections();
    setupGroupTranspose();
    setupGroupPresetDump();
    setupGroupVoices();
    setupGroupTuning();
    setupGroupRetuneEncoder();
    setupGroupDisplay();
    setupGroupAutoTune();
    setupGroupMiscellaneous();
    setupGroupSync();
    setupGroupFactoryReset();
}

SettingsTab::~SettingsTab() 
{
    delete group_PresetDump;
    delete group_FactoryReset;
}

void SettingsTab::handlePro800SettingsUpdate()
{
  std::shared_ptr<SettingsMessage> settingsMessage = getCurrentSettings();
  if(!settingsMessage || !settingsMessage->isValid())
  {
    return;
  }

  spinBox_DisplayBrightness.setValue( settingsMessage->getValue(Pro800Settings::SETTINGS_BRIGHTNESS), juce::NotificationType::dontSendNotification );
}

void SettingsTab::handlePro800VersionUpdate()
{
    std::shared_ptr<VersionMessage> versionMessage = getCurrentVersion();
    if( !versionMessage || !versionMessage->isValid())
    {
      return;
    }

    label_FirmwareVersion.setText( "Firmware Version: " + versionMessage->getVersionString(), juce::NotificationType::dontSendNotification);
}

void SettingsTab::resized()
{
    const int refreshHeight = 30;
    const int numVerticalElements = 27;

    auto area = getLocalBounds().reduced (10);
    int elementHeight = (area.getHeight() - refreshHeight) / numVerticalElements;
    int groupWidth = area.getWidth() / 3;

    auto leftColumn = area.withRight (groupWidth);
    auto middleColumn = area.withLeft (groupWidth).withRight (2 * groupWidth);
    auto rightColumn = area.withLeft (2 * groupWidth);

    label_FirmwareVersion.setBounds(leftColumn.removeFromTop(refreshHeight));
    middleColumn.removeFromTop(refreshHeight);
    button_RefreshSettings.setBounds(rightColumn.removeFromTop(refreshHeight));

    this->group_Connections.setBounds (leftColumn.removeFromTop (11*elementHeight));
    this->group_Transpose.setBounds (middleColumn.removeFromTop (11*elementHeight));
    this->group_PresetDump->setBounds (rightColumn.removeFromTop (11*elementHeight));

    this->group_Voices.setBounds (leftColumn.removeFromTop (6*elementHeight));
    this->group_Tuning.setBounds (middleColumn.removeFromTop (6*elementHeight));
    this->group_RetuneEncoder.setBounds (rightColumn.removeFromTop (6*elementHeight));

    this->group_Display.setBounds (leftColumn.removeFromTop (4*elementHeight));
    this->group_AutoTune.setBounds (middleColumn.removeFromTop (4*elementHeight));
    this->group_Miscellaneous.setBounds (rightColumn.removeFromTop (4*elementHeight));

    this->group_Sync.setBounds (leftColumn.removeFromTop (6*elementHeight));
    this->group_FactoryReset->setBounds (middleColumn.removeFromTop (6*elementHeight));
}

void SettingsTab::setupGroupConnections()
{
  combo_ConnectionsMidiInputChannel.addItem("Dip Switches", Pro800MidiReceiveChannel::MIDI_RX_DIPS + 1);
  combo_ConnectionsMidiInputChannel.addItem("All", Pro800MidiReceiveChannel::MIDI_RX_ALL + 1);
  combo_ConnectionsMidiInputChannel.addItem("Off", Pro800MidiReceiveChannel::MIDI_RX_OFF + 1);
  for ( int ch = 0; ch < 16; ch++ )
  {
    combo_ConnectionsMidiInputChannel.addItem(juce::String::formatted("%d", ch+1), Pro800MidiReceiveChannel::MIDI_RX_1 + ch + 1);
  }

  combo_ConnectionsMidiOutputChannel.addItem("Dip Switches", Pro800MidiTransmitChannel::MIDI_TX_DIPS + 1);
  combo_ConnectionsMidiOutputChannel.addItem("All", Pro800MidiTransmitChannel::MIDI_TX_THRU + 1);
  for ( int ch = 0; ch < 16; ch++ )
  {
    combo_ConnectionsMidiOutputChannel.addItem(juce::String::formatted("%d", ch+1), Pro800MidiTransmitChannel::MIDI_TX_1 + ch + 1);
  }

  combo_ConnectionsMidiCC.addItem("Send & Receive", Pro800MidiMode::MIDI_MODE_TX_RX+1);
  combo_ConnectionsMidiCC.addItem("Send", Pro800MidiMode::MIDI_MODE_TX+1);
  combo_ConnectionsMidiCC.addItem("Receive", Pro800MidiMode::MIDI_MODE_RX+1);
  combo_ConnectionsMidiCC.addItem("Off", Pro800MidiMode::MIDI_MODE_OFF+1);

  combo_ConnectionsMidiPC.addItem("Send & Receive", Pro800MidiMode::MIDI_MODE_TX_RX+1);
  combo_ConnectionsMidiPC.addItem("Send", Pro800MidiMode::MIDI_MODE_TX+1);
  combo_ConnectionsMidiPC.addItem("Receive", Pro800MidiMode::MIDI_MODE_RX+1);
  combo_ConnectionsMidiPC.addItem("Off", Pro800MidiMode::MIDI_MODE_OFF+1);

  combo_ConnectionsSyncInPolarity.addItem("Rise", Pro800SettingsPolarity::POLARITY_RISE+1);
  combo_ConnectionsSyncInPolarity.addItem("Fall", Pro800SettingsPolarity::POLARITY_FALL+1);
  combo_ConnectionsSyncInPolarity.addItem("Both", Pro800SettingsPolarity::POLARITY_BOTH+1);

  combo_ConnectionsSyncInPPQN.addItem("1PPS", Pro800SettingsSyncInPPQN::SYNC_IN_1PPS+1);
  combo_ConnectionsSyncInPPQN.addItem("1PPQN", Pro800SettingsSyncInPPQN::SYNC_IN_1PPQN+1);
  combo_ConnectionsSyncInPPQN.addItem("2PPQN", Pro800SettingsSyncInPPQN::SYNC_IN_2PPQN+1);
  combo_ConnectionsSyncInPPQN.addItem("4PPQN", Pro800SettingsSyncInPPQN::SYNC_IN_4PPQN+1);
  combo_ConnectionsSyncInPPQN.addItem("24PPQN", Pro800SettingsSyncInPPQN::SYNC_IN_24PPQN+1);
  combo_ConnectionsSyncInPPQN.addItem("48PPQN", Pro800SettingsSyncInPPQN::SYNC_IN_48PPQN+1);

  this->group_Connections.addLabelledComponents(
    { "MIDI Input Channel:", "MIDI Output Channel:", "MIDI CC: ", "MIDI PC:", "Sync In Forward:", 
      "Sync In Polarity:", "Sync In Start-Stop:", "Sync In PPQN:", "Local Enable:", "Soft Thru" },
    { &combo_ConnectionsMidiInputChannel, &combo_ConnectionsMidiOutputChannel, &combo_ConnectionsMidiCC, &combo_ConnectionsMidiPC, &checkBox_ConnectionsSyncInForwardEnabled,
      &combo_ConnectionsSyncInPolarity, &checkBox_ConnectionsSyncInStartStopEnabled, &combo_ConnectionsSyncInPPQN, &checkBox_ConnectionsLocalEnable, &checkBox_ConnectionsSoftThru }
  );

  addAndMakeVisible(group_Connections);
}

void SettingsTab::setupGroupTranspose()
{
  spinBox_TransposeAmount.setRange(-35.0, 35.0, 1.0);
  this->group_Transpose.addLabelledComponents(
    { "Transpose Amount:" },
    { &spinBox_TransposeAmount }
  );


  addAndMakeVisible(group_Transpose);
}

void SettingsTab::setupGroupPresetDump()
{
  this->group_PresetDump = new EqualSpacingGroupComponent(
    "3 - Preset Dump",
    {&button_PresetDump}
  );

  this->group_PresetDump->setTextLabelPosition(juce::Justification::left);

  this->group_PresetDump->setEnabled(false); // not implemented yet

  addAndMakeVisible(group_PresetDump);
}

void SettingsTab::setupGroupVoices()
{
  for ( int i = 0; i < 8; i++ )
  {
    checkBox_Voice[i].setButtonText("Voice " + juce::String::formatted("%d", (i+1)));
  }

  this->group_Voices.addComponentPairs(
    { &label_VoiceKill, &checkBox_Voice[0], &checkBox_Voice[1], &checkBox_Voice[2], &checkBox_Voice[3]},
    { nullptr,          &checkBox_Voice[4], &checkBox_Voice[5], &checkBox_Voice[6], &checkBox_Voice[7]}
  );

  addAndMakeVisible(group_Voices);
}

void SettingsTab::setupGroupTuning()
{
  this->group_Tuning.addLabelledComponents(
    { "Retune Element",           "Retune Octave" },
    { &combo_TuningRetuneElement, &combo_TuningRetuneOctave }
  );

  this->group_Tuning.setEnabled(false); // not implemented yet

  addAndMakeVisible(group_Tuning);
}

void SettingsTab::setupGroupRetuneEncoder()
{
  this->group_RetuneEncoder.addLabelledComponents(
    { "Tuning" },
    { &spinBox_RetuneTuning }
  );

  this->group_RetuneEncoder.setEnabled(false); // not implemented yet

  addAndMakeVisible(group_RetuneEncoder);
}

void SettingsTab::setupGroupDisplay()
{
  spinBox_DisplayBrightness.setRange(1.0, 16.0, 1.0);
  spinBox_DisplayBrightness.onValueChange = ([this] { updateSettings(Pro800Settings::SETTINGS_BRIGHTNESS, (int)spinBox_DisplayBrightness.getValue()); });

  spinBox_DisplayParameterTime.setRange(0.0, 100, 1.0);

  this->group_Display.addLabelledComponents(
    { "Brightness:",              "Display Parameter Time:",     "Show Preset Names:" },
    { &spinBox_DisplayBrightness, &spinBox_DisplayParameterTime, &checkBox_DisplayPresetNameEnabled }
  );

  addAndMakeVisible(group_Display);
}

void SettingsTab::setupGroupAutoTune()
{
  combo_AutoTunePrecision.addItem("0.5 cent", Pro800TunerPrecision::TUNER_PRECISION_0_5CT+1);
  combo_AutoTunePrecision.addItem("1.0 cent", Pro800TunerPrecision::TUNER_PRECISION_1CT+1);
  combo_AutoTunePrecision.addItem("1.5 cent", Pro800TunerPrecision::TUNER_PRECISION_1_5CT+1);
  combo_AutoTunePrecision.addItem("2.0 cent", Pro800TunerPrecision::TUNER_PRECISION_2CT+1);

  this->group_AutoTune.addLabelledComponents(
    { "Precision" },
    { &combo_AutoTunePrecision }
  );

  addAndMakeVisible(group_AutoTune);
}

void SettingsTab::setupGroupMiscellaneous()
{
  spinBox_MiscExternalFilterModAmount.setRange(0.0, 65535.0, 1.0);
  combo_MiscVoicePriority.addItem("Last", Pro800VoicePriority::VOICE_PRIORITY_LAST+1);
  combo_MiscVoicePriority.addItem("Low", Pro800VoicePriority::VOICE_PRIORITY_LOW+1);
  combo_MiscVoicePriority.addItem("High", Pro800VoicePriority::VOICE_PRIORITY_HIGH+1);

  combo_MiscPedalPriority.setEnabled(false); // not implemented yet

  this->group_Miscellaneous.addLabelledComponents(
    { "External Filter Mod Amount:",         "Voice Priority",         "Pedal Priority" },
    {  &spinBox_MiscExternalFilterModAmount, &combo_MiscVoicePriority, &combo_MiscPedalPriority }
  );

  addAndMakeVisible(group_Miscellaneous);
}

void SettingsTab::setupGroupSync()
{
  combo_SyncSource.addItem("Internal", Pro800SyncSource::SYNC_SOURCE_INTERNAL+1);
  combo_SyncSource.addItem("MIDI", Pro800SyncSource::SYNC_SOURCE_MIDI+1);
  combo_SyncSource.addItem("USB", Pro800SyncSource::SYNC_SOURCE_USB+1);
  combo_SyncSource.addItem("External", Pro800SyncSource::SYNC_SOURCE_EXTERNAL+1);

  spinBox_SyncClockBPM.setRange(50.0, 400.0, 0.1);

  combo_SyncClockSubdivision.addItem("1/4", Pro800SyncClockSubdivision::SYNC_CLOCK_SUBDIVISION_1_4+1);
  combo_SyncClockSubdivision.addItem("1/4T", Pro800SyncClockSubdivision::SYNC_CLOCK_SUBDIVISION_1_4T+1);
  combo_SyncClockSubdivision.addItem("1/8", Pro800SyncClockSubdivision::SYNC_CLOCK_SUBDIVISION_1_8+1);
  combo_SyncClockSubdivision.addItem("1/4T", Pro800SyncClockSubdivision::SYNC_CLOCK_SUBDIVISION_1_8T+1);
  combo_SyncClockSubdivision.addItem("1/16", Pro800SyncClockSubdivision::SYNC_CLOCK_SUBDIVISION_1_16+1);
  combo_SyncClockSubdivision.addItem("1/16T", Pro800SyncClockSubdivision::SYNC_CLOCK_SUBDIVISION_1_16T+1);
  combo_SyncClockSubdivision.addItem("1/32", Pro800SyncClockSubdivision::SYNC_CLOCK_SUBDIVISION_1_32+1);
  combo_SyncClockSubdivision.addItem("1/32T", Pro800SyncClockSubdivision::SYNC_CLOCK_SUBDIVISION_1_32T+1);

  spinBox_SyncClockSwing.setRange(50.0, 95.0, 1.0);
  spinBox_SyncClockNoteLength.setRange(1.0, 100.0, 1.0);

  this->group_Sync.addLabelledComponents(
    { "Sync Source:",    "Sync Clock BPM:",     "Sync Clock Subdivision:",   "Sync Clock Swing:",     "Sync Clock Note Length:" },
    { &combo_SyncSource, &spinBox_SyncClockBPM, &combo_SyncClockSubdivision, &spinBox_SyncClockSwing, &spinBox_SyncClockNoteLength }
  );

  addAndMakeVisible(group_Sync);
}

void SettingsTab::setupGroupFactoryReset()
{
  // only button_FactoryReset
  this->group_FactoryReset = new EqualSpacingGroupComponent(
    "0 - Factory Reset",
    {&button_FactoryReset}
  );

  this->group_FactoryReset->setTextLabelPosition(juce::Justification::left);

  this->group_FactoryReset->setEnabled(false); // not implemented yet

  addAndMakeVisible(group_FactoryReset);
}