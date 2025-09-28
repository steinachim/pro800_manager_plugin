/*
  ==============================================================================

    MainWidget.cpp
    Created: 9 Sep 2025 11:11:03am
    Author:  Achim Stein

  ==============================================================================
*/

#include "MainWidget.h"

#include "../midi/MidiHandler.h"
#include "../midi/VersionMessage.h"

#include "FrontPanelTab.h"
#include "SettingsTab.h"
#include "AdvancedTab.h"
#include "PerformanceTab.h"

MainWidget::MainWidget(MidiHandler *midiHandler) : TabbedComponent(juce::TabbedButtonBar::TabsAtTop), MidiComponent(midiHandler, false, {PRO800_VERSION_MESSAGE})
{
    auto colour = findColour (juce::ResizableWindow::backgroundColourId);

    addTab("Front Panel", colour, new FrontPanelTab(midiHandler), true, TAB_FRONTPANEL);
    addTab("Performance", colour, new PerformanceTab(midiHandler), true, TAB_PERFORMANCE);
    addTab("Settings", colour, new SettingsTab(midiHandler), true, TAB_SETTINGS);
    addTab("Advanced", colour, new AdvancedTab(midiHandler), true, TAB_ADVANCED);

    this->getTabContentComponent(TAB_FRONTPANEL)->setEnabled(false);
    this->getTabContentComponent(TAB_PERFORMANCE)->setEnabled(false);
    this->getTabContentComponent(TAB_SETTINGS)->setEnabled(true);
    this->getTabContentComponent(TAB_ADVANCED)->setEnabled(false);

    midiHandler->sendMidiMessage(VersionMessage::request());    
}

MainWidget::~MainWidget()
{
    
}

void MainWidget::handlePro800VersionUpdate()
{
  if ( !this->getCurrentVersion() )
  {
    return;
  }
  
  if ( !this->getCurrentVersion()->isSupported() )
  {
    juce::String messageText = "The firmware version of your Pro-800 is not supported. Proceed with extreme caution!\n\n";
    messageText = messageText + "Installed version:  " + this->getCurrentVersion()->getVersionString() + "\n";
    messageText = messageText + "Supported versions: " + this->getCurrentVersion()->getSupportedVersions();

    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Unsupported Firmware Version", messageText);
  }

  this->getTabContentComponent(TAB_FRONTPANEL)->setEnabled(true);
  this->getTabContentComponent(TAB_PERFORMANCE)->setEnabled(true);
  this->getTabContentComponent(TAB_SETTINGS)->setEnabled(true);
 this->getTabContentComponent(TAB_ADVANCED)->setEnabled(true);
}