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

#include "MainWidget.h"

#include "../midi/MidiHandler.h"
#include "../midi/VersionMessage.h"

#include "FrontPanelTab.h"
#include "SettingsTab.h"
#include "AdvancedTab.h"
#include "PerformanceTab.h"
#include "ProgramManagementTab.h"

MainWidget::MainWidget(MidiHandler *midiHandler) : TabbedComponent(juce::TabbedButtonBar::TabsAtTop), MidiComponent(midiHandler, false, {PRO800_VERSION_MESSAGE})
{
    auto colour = findColour (juce::ResizableWindow::backgroundColourId);

    addTab("Front Panel", colour, new FrontPanelTab(midiHandler), true, TAB_FRONTPANEL);
    addTab("Performance", colour, new PerformanceTab(midiHandler), true, TAB_PERFORMANCE);
    addTab("Settings", colour, new SettingsTab(midiHandler), true, TAB_SETTINGS);
    addTab("Program Management", colour, new ProgramManagementTab(midiHandler), true, TAB_PROGRAM);
    addTab("Advanced", colour, new AdvancedTab(midiHandler), true, TAB_ADVANCED);

    this->getTabContentComponent(TAB_FRONTPANEL)->setEnabled(false);
    this->getTabContentComponent(TAB_PERFORMANCE)->setEnabled(false);
    this->getTabContentComponent(TAB_SETTINGS)->setEnabled(true);
    this->getTabContentComponent(TAB_PROGRAM)->setEnabled(false);
    this->getTabContentComponent(TAB_ADVANCED)->setEnabled(true);

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
  this->getTabContentComponent(TAB_PROGRAM)->setEnabled(true);
}