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

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiComponent.h"

class MidiHandler;

class MainWidget : public juce::TabbedComponent
{
public:
    MainWidget(MidiHandler *midiHandler);
    ~MainWidget() override;

    void loadFromProgram(const std::shared_ptr<ProgramMessage> &programMessage);

private:
    enum TabIds
    {
      TAB_FRONTPANEL = 0,
      TAB_PERFORMANCE,
      TAB_SETTINGS,
      TAB_PROGRAM,
      TAB_ADVANCED
    };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWidget)

};
