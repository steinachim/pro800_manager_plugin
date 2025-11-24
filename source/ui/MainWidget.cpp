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

MainWidget::MainWidget(MidiHandler *midiHandler) : TabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    auto colour = findColour (juce::ResizableWindow::backgroundColourId);

    addTab("Front Panel", colour, new FrontPanelTab(midiHandler), true, TAB_FRONTPANEL);
    addTab("Performance", colour, new PerformanceTab(midiHandler), true, TAB_PERFORMANCE);
    addTab("Settings", colour, new SettingsTab(midiHandler), true, TAB_SETTINGS);
    addTab("Program Management", colour, new ProgramManagementTab(midiHandler, this), true, TAB_PROGRAM);
    addTab("Advanced", colour, new AdvancedTab(midiHandler), true, TAB_ADVANCED);
}

MainWidget::~MainWidget()
{
    
}

void MainWidget::loadFromProgram(const std::shared_ptr<ProgramMessage> &programMessage)
{
    auto* frontPanelTab = dynamic_cast<FrontPanelTab*>(getTabContentComponent(TAB_FRONTPANEL));
    if ( frontPanelTab != nullptr )
    {
        frontPanelTab->loadFromProgram(programMessage);
    }

    auto* performanceTab = dynamic_cast<PerformanceTab*>(getTabContentComponent(TAB_PERFORMANCE));
    if ( performanceTab != nullptr )
    {
        performanceTab->loadFromProgram(programMessage);
    }
}