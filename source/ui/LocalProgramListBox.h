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

class LocalProgramListBox : public juce::ListBox, public juce::DragAndDropTarget
{
public:
    LocalProgramListBox();
    LocalProgramListBox(const juce::String &componentName, juce::ListBoxModel *model);
    ~LocalProgramListBox() override; 

    virtual bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    virtual void itemDropped (const SourceDetails& dragSourceDetails) override;
    virtual void itemDragEnter (const SourceDetails& dragSourceDetails) override;
    virtual void itemDragMove (const SourceDetails& dragSourceDetails) override;
    virtual void itemDragExit (const SourceDetails& dragSourceDetails) override;
};