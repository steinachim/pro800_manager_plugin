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

SettingsTab::SettingsTab(MidiHandler *midiHandler) : Component(), MidiComponent(midiHandler, false, {Pro800MessageType::PRO800_SETTINGS_MESSAGE, Pro800MessageType::PRO800_VERSION_MESSAGE})
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

    setSettingsGroupsEnabled(false);
}

SettingsTab::~SettingsTab() 
{
  this->settingsListeners.clear();
}

void SettingsTab::handlePro800SettingsUpdate()
{
  std::shared_ptr<SettingsMessage> settingsMessage = getCurrentSettings();
  if(!settingsMessage || !settingsMessage->isValid())
  {
    setSettingsGroupsEnabled(false);
    return;
  }

  setSettingsGroupsEnabled(true);

  for (juce::HashMap<Pro800Settings, juce::Component*>::Iterator it(this->settingsListeners); it.next();)
  {
    Pro800Settings setting = it.getKey();
    juce::Component *component = it.getValue();

    // special case handling for weird UI cases
    if ( setting == Pro800Settings::SETTINGS_VOICE_KILL )
    {
        uint8_t value = settingsMessage->getVoiceStatus();

        for ( int i = 0; i < 8; i++ )
        {
          this->checkBox_Voice[i].setToggleState( value & 0x01, juce::NotificationType::dontSendNotification );
          value >>= 1;
        }

        continue;
    }

    if ( juce::Slider *slider = dynamic_cast<juce::Slider*>(component))
    {
        slider->setValue( settingsMessage->getValue(setting) , juce::NotificationType::dontSendNotification);
    }
    else if ( juce::ComboBox *comboBox = dynamic_cast<juce::ComboBox*>(component))
    {
        comboBox->setSelectedId( settingsMessage->getValue(setting) + 1, juce::NotificationType::dontSendNotification);
    }
    else if ( juce::ToggleButton *button = dynamic_cast<juce::ToggleButton*>(component))
    {
        button->setToggleState( settingsMessage->getValue(setting) == Pro800SettingsOnOff::SETTINGS_ON, juce::NotificationType::dontSendNotification);
    }
    else 
    {
        std::cerr << "[WARNING] handlePro800SettingsUpdate(): Unknown component type for setting " << setting << std::endl;
    }

  }

  combo_ConnectionsMidiCC.setSelectedId( settingsMessage->getValue(Pro800Settings::SETTINGS_MIDI_CC_MODE)+1, juce::NotificationType::dontSendNotification);
  checkBox_ConnectionsSyncInForwardEnabled.setToggleState(settingsMessage->getValue(Pro800Settings::SETTINGS_SYNC_IN_FORWARD) == Pro800SettingsOnOff::SETTINGS_ON, juce::NotificationType::dontSendNotification);
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
    this->group_PresetDump.setBounds (rightColumn.removeFromTop (11*elementHeight));

    this->group_Voices.setBounds (leftColumn.removeFromTop (6*elementHeight));
    this->group_Tuning.setBounds (middleColumn.removeFromTop (6*elementHeight));
    this->group_RetuneEncoder.setBounds (rightColumn.removeFromTop (6*elementHeight));

    this->group_Display.setBounds (leftColumn.removeFromTop (4*elementHeight));
    this->group_AutoTune.setBounds (middleColumn.removeFromTop (4*elementHeight));
    this->group_Miscellaneous.setBounds (rightColumn.removeFromTop (4*elementHeight));

    this->group_Sync.setBounds (leftColumn.removeFromTop (6*elementHeight));
    this->group_FactoryReset.setBounds (middleColumn.removeFromTop (6*elementHeight));
}

void SettingsTab::setupGroupConnections()
{
  combo_ConnectionsMidiInputChannel.addItem("Dip Switches", Pro800MidiReceiveChannel::MIDI_RX_DIPS + 1); // TODO
  combo_ConnectionsMidiInputChannel.addItem("All", Pro800MidiReceiveChannel::MIDI_RX_ALL + 1);
  combo_ConnectionsMidiInputChannel.addItem("Off", Pro800MidiReceiveChannel::MIDI_RX_OFF + 1);
  for ( int ch = 0; ch < 16; ch++ )
  {
    combo_ConnectionsMidiInputChannel.addItem(juce::String::formatted("%d", ch+1), Pro800MidiReceiveChannel::MIDI_RX_1 + ch + 1);
  }

  combo_ConnectionsMidiOutputChannel.addItem("Dip Switches", Pro800MidiTransmitChannel::MIDI_TX_DIPS + 1);
  combo_ConnectionsMidiOutputChannel.addItem("Thru", Pro800MidiTransmitChannel::MIDI_TX_THRU + 1);
  for ( int ch = 0; ch < 16; ch++ )
  {
    combo_ConnectionsMidiOutputChannel.addItem(juce::String::formatted("%d", ch+1), Pro800MidiTransmitChannel::MIDI_TX_1 + ch + 1);
  }

  combo_ConnectionsMidiCC.addItem("Send & Receive", Pro800MidiMode::MIDI_MODE_TX_RX+1);
  combo_ConnectionsMidiCC.addItem("Send", Pro800MidiMode::MIDI_MODE_TX+1);
  combo_ConnectionsMidiCC.addItem("Receive", Pro800MidiMode::MIDI_MODE_RX+1);
  combo_ConnectionsMidiCC.addItem("Off", Pro800MidiMode::MIDI_MODE_OFF+1);

  combo_ConnectionsMidiPC.addItem("Send & Receive", Pro800MidiMode::MIDI_MODE_TX_RX+1); // TODO
  combo_ConnectionsMidiPC.addItem("Send", Pro800MidiMode::MIDI_MODE_TX+1);
  combo_ConnectionsMidiPC.addItem("Receive", Pro800MidiMode::MIDI_MODE_RX+1);
  combo_ConnectionsMidiPC.addItem("Off", Pro800MidiMode::MIDI_MODE_OFF+1);

  combo_ConnectionsSyncInPolarity.addItem("Rise", Pro800SettingsPolarity::POLARITY_RISE+1); // TODO
  combo_ConnectionsSyncInPolarity.addItem("Fall", Pro800SettingsPolarity::POLARITY_FALL+1);
  combo_ConnectionsSyncInPolarity.addItem("Both", Pro800SettingsPolarity::POLARITY_BOTH+1);

  combo_ConnectionsSyncInPPQN.addItem("1PPS", Pro800SettingsSyncInPPQN::SYNC_IN_1PPS+1); // TODO
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

  this->setupSettingsComponent(Pro800Settings::SETTINGS_MIDI_RX_CHANNEL, &combo_ConnectionsMidiInputChannel);
  this->setupSettingsComponent(Pro800Settings::SETTINGS_MIDI_TX_CHANNEL, &combo_ConnectionsMidiOutputChannel);
  this->setupSettingsComponent(Pro800Settings::SETTINGS_MIDI_CC_MODE, &combo_ConnectionsMidiCC);
  this->setupSettingsComponent(Pro800Settings::SETTINGS_MIDI_PC_MODE, &combo_ConnectionsMidiPC);
  this->setupSettingsComponent(Pro800Settings::SETTINGS_SYNC_IN_FORWARD, &checkBox_ConnectionsSyncInForwardEnabled);
  this->setupSettingsComponent(Pro800Settings::SETTINGS_SYNC_IN_POLARITY, &combo_ConnectionsSyncInPolarity);
  this->setupSettingsComponent(Pro800Settings::SETTINGS_SYNC_IN_START_STOP, &checkBox_ConnectionsSyncInStartStopEnabled);
  this->setupSettingsComponent(Pro800Settings::SETTINGS_SYNC_IN_PPQN, &combo_ConnectionsSyncInPPQN);
  this->setupSettingsComponent(Pro800Settings::SETTINGS_LOCAL_ENABLE, &checkBox_ConnectionsLocalEnable);
  this->setupSettingsComponent(Pro800Settings::SETTINGS_SOFT_THRU, &checkBox_ConnectionsSoftThru);

  addAndMakeVisible(group_Connections);
}

void SettingsTab::setupGroupTranspose()
{
  spinBox_TransposeAmount.setRange(-35.0, 35.0, 1.0);
  this->group_Transpose.addLabelledComponents(
    { "Transpose Amount:" },
    { &spinBox_TransposeAmount }
  );

  this->setupSettingsComponent(Pro800Settings::SETTINGS_TRANSPOSE, &spinBox_TransposeAmount);

  addAndMakeVisible(group_Transpose);
}

void SettingsTab::setupGroupPresetDump()
{
  this->group_PresetDump.addComponent(&button_PresetDump, 0.1);
  this->group_PresetDump.setTextLabelPosition(juce::Justification::left);

  this->group_PresetDump.setEnabled(false); // not implemented yet

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

  for ( int i = 0; i < 8; i++ )
  {
    setupSettingsComponent(Pro800Settings::SETTINGS_VOICE_KILL, &checkBox_Voice[i]);
  }

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

  spinBox_DisplayParameterTime.setRange(0.0, 100, 1.0);

  this->group_Display.addLabelledComponents(
    { "Brightness:",              "Display Parameter Time:",     "Show Preset Names:" },
    { &spinBox_DisplayBrightness, &spinBox_DisplayParameterTime, &checkBox_DisplayPresetNameEnabled }
  );

  setupSettingsComponent(Pro800Settings::SETTINGS_BRIGHTNESS, &spinBox_DisplayBrightness);
  setupSettingsComponent(Pro800Settings::SETTINGS_DISPLAY_PARAMETER_TIME, &spinBox_DisplayParameterTime);
  setupSettingsComponent(Pro800Settings::SETTINGS_SHOW_PRESET_NAME, &checkBox_DisplayPresetNameEnabled);

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

  setupSettingsComponent(Pro800Settings::SETTINGS_TUNER_PRECISION, &combo_AutoTunePrecision);

  addAndMakeVisible(group_AutoTune);
}

void SettingsTab::setupGroupMiscellaneous()
{
  spinBox_MiscExternalFilterModAmount.setRange(0.0, 65535.0, 1.0);
  combo_MiscVoicePriority.addItem("Last", Pro800VoicePriority::VOICE_PRIORITY_LAST+1);
  combo_MiscVoicePriority.addItem("Low", Pro800VoicePriority::VOICE_PRIORITY_LOW+1);
  combo_MiscVoicePriority.addItem("High", Pro800VoicePriority::VOICE_PRIORITY_HIGH+1);

  combo_MiscPedalPriority.setEnabled(false); // not implemented yet (not in standard settings message)

  this->group_Miscellaneous.addLabelledComponents(
    { "External Filter Mod Amount:",         "Voice Priority",         "Pedal Priority" },
    {  &spinBox_MiscExternalFilterModAmount, &combo_MiscVoicePriority, &combo_MiscPedalPriority }
  );

  setupSettingsComponent(Pro800Settings::SETTINGS_EXTERNAL_CV_AMOUNT, &spinBox_MiscExternalFilterModAmount);
  setupSettingsComponent(Pro800Settings::SETTINGS_VOICE_PRIORITY, &combo_MiscVoicePriority);

  addAndMakeVisible(group_Miscellaneous);
}

void SettingsTab::setupGroupSync()
{
  combo_SyncSource.addItem("Internal", Pro800SyncSource::SYNC_SOURCE_INTERNAL+1);
  combo_SyncSource.addItem("MIDI", Pro800SyncSource::SYNC_SOURCE_MIDI+1);
  combo_SyncSource.addItem("USB", Pro800SyncSource::SYNC_SOURCE_USB+1);
  combo_SyncSource.addItem("External", Pro800SyncSource::SYNC_SOURCE_EXTERNAL+1);

  spinBox_SyncClockBPM.valueFromTextFunction = [](const juce::String &text) { return text.getFloatValue() * 10.0; };
  spinBox_SyncClockBPM.textFromValueFunction = [](double value)             { return juce::String::formatted("%.1f", value/10.0); };
  spinBox_SyncClockBPM.setRange(500, 4000, 1.0);

  combo_SyncClockSubdivision.addItem("1/4", Pro800SyncClockSubdivision::SYNC_CLOCK_SUBDIVISION_1_4+1);
  combo_SyncClockSubdivision.addItem("1/4T", Pro800SyncClockSubdivision::SYNC_CLOCK_SUBDIVISION_1_4T+1);
  combo_SyncClockSubdivision.addItem("1/8", Pro800SyncClockSubdivision::SYNC_CLOCK_SUBDIVISION_1_8+1);
  combo_SyncClockSubdivision.addItem("1/8T", Pro800SyncClockSubdivision::SYNC_CLOCK_SUBDIVISION_1_8T+1);
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

  setupSettingsComponent(Pro800Settings::SETTINGS_SYNC_SOURCE, &combo_SyncSource);
  setupSettingsComponent(Pro800Settings::SETTINGS_SYNC_CLOCK_BPM, &spinBox_SyncClockBPM);
  setupSettingsComponent(Pro800Settings::SETTINGS_SYNC_CLOCK_SUBDIVISION, &combo_SyncClockSubdivision);
  setupSettingsComponent(Pro800Settings::SETTINGS_SYNC_CLOCK_SWING, &spinBox_SyncClockSwing);
  setupSettingsComponent(Pro800Settings::SETTINGS_SYNC_CLOCK_NOTE_LENGTH, &spinBox_SyncClockNoteLength);

  addAndMakeVisible(group_Sync);
}

void SettingsTab::setupGroupFactoryReset()
{
  this->group_FactoryReset.addComponent(&button_FactoryReset, 0.2);
  this->group_FactoryReset.setTextLabelPosition(juce::Justification::left);

  button_FactoryReset.onClick = [this] {
    const auto callback = juce::ModalCallbackFunction::create ([this] (int result) {
      if ( result == 1 )
      {
        this->requestFactoryReset();
      }
    });
    juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::WarningIcon, "Factory Reset", "This will trigger a factory reset of the device. Are you sure?", "OK", "Abort", this, callback);
  };

  addAndMakeVisible(group_FactoryReset);
}

void SettingsTab::setSettingsGroupsEnabled(bool enable)
{
  this->group_Connections.setEnabled(enable);
  this->group_Transpose.setEnabled(enable);
  this->group_PresetDump.setEnabled(false); // not implemented
  this->group_Voices.setEnabled(enable);
  this->group_Tuning.setEnabled(false); // not implemented
  this->group_RetuneEncoder.setEnabled(false); // not implemented
  this->group_Display.setEnabled(enable);
  this->group_AutoTune.setEnabled(enable);
  this->group_Miscellaneous.setEnabled(enable);
  this->group_Sync.setEnabled(enable);
  this->group_FactoryReset.setEnabled(true); // always enabled (does not depend on settings)
}

void SettingsTab::setupSettingsComponent(Pro800Settings setting, juce::Component *component)
{
    this->settingsListeners.set(setting, component);

    // special case handling for weird UI cases
    if ( setting == Pro800Settings::SETTINGS_VOICE_KILL )
    {
        juce::ToggleButton *checkBox = dynamic_cast<juce::ToggleButton*>(component);
        if ( checkBox == nullptr )
        {
            std::cerr << "[ERROR] Cannot setup voice kill checkboxes... Not a toggle button"<< std::endl;
            return;
        }

        checkBox->onClick = [this] {
          // ignore component and just create combined byte of all 8 bits
          uint8_t enabledVoices = 0;
          for ( int i = 7; i >= 0; i-- )
          {
            enabledVoices = (uint8_t)((enabledVoices << (uint8_t)1) | (this->checkBox_Voice[i].getToggleState() ? (uint8_t)1 : (uint8_t)0));
          }

          updateSettings(Pro800Settings::SETTINGS_VOICE_KILL, enabledVoices);
        };

        return;
    }

    if ( juce::Slider *slider = dynamic_cast<juce::Slider*>(component))
    {
        slider->onValueChange = ([this, slider, setting] { 
            updateSettings(setting, (int)slider->getValue()); 
        });
    }
    else if ( juce::ComboBox *comboBox = dynamic_cast<juce::ComboBox*>(component))
    {
        comboBox->onChange = ([this, comboBox, setting] { 
            updateSettings(setting, comboBox->getSelectedId()-1); 
        });
    }
    else if ( juce::ToggleButton *button = dynamic_cast<juce::ToggleButton*>(component))
    {
        button->onClick = ([this, button, setting] {
            updateSettings(setting, (button->getToggleState() ? Pro800SettingsOnOff::SETTINGS_ON : Pro800SettingsOnOff::SETTINGS_OFF ));
        });
    }
    else 
    {
        std::cerr << "[WARNING] setupSettingsComponent(): Unknown component type for setting " << setting << std::endl;
    }

}