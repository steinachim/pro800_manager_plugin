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

#include "AdvancedTab.h"
#include "FrontPanelTab.h"
#include "PerformanceTab.h"
#include "ProgramManagementTab.h"
#include "SettingsTab.h"

MainWidget::MainWidget (MidiHandler* midiHandler, SynthSession& synthSession) : TabbedComponent (juce::TabbedButtonBar::TabsAtTop)
{
    auto colour = findColour (juce::ResizableWindow::backgroundColourId);

    // the tabs are owned by this TabbedComponent (deleteComponentWhenNotNeeded = true)
    // clang-format off
    addTab("Front Panel",        colour, new FrontPanelTab(midiHandler, synthSession),        true);
    addTab("Performance",        colour, new PerformanceTab(midiHandler, synthSession),       true);
    addTab("Settings",           colour, new SettingsTab(midiHandler, synthSession),          true);
    addTab("Program Management", colour, new ProgramManagementTab(midiHandler, synthSession), true);
    addTab("Advanced",           colour, new AdvancedTab(midiHandler, synthSession),          true);
    // clang-format on
}