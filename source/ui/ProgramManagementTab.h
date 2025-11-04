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
#include "LocalProgramListBox.h"

class ProgramModel;

class ProgramManagementTab : public juce::Component, public juce::DragAndDropContainer, public MidiComponent
{
public:
    ProgramManagementTab(MidiHandler *handler);
    virtual ~ProgramManagementTab() override;

    void resized() override;

    virtual void handlePro800ProgramDump(std::shared_ptr<ProgramMessage> &programMessage) override;


private:
    void compareSelectedPrograms();

    juce::ListBox listBox_ProgramListSynth;
    LocalProgramListBox listBox_ProgramListLocal;

    juce::TextButton button_RefreshDump { "Refresh Dump" };
    juce::TextButton button_Compare { "Compare" };
    juce::TextButton button_Export { "Export" };
    juce::TextButton button_Import { "Import" };

    juce::TextButton button_SynthToLocal { ">>" };
    juce::TextButton button_SynthToLocalAll { "All >>" };
    juce::TextButton button_LocalToSynth { "<<" };
    juce::TextButton button_LocalToSynthAll { "All <<" };

    juce::Label label_Local { "", "Local" };
    juce::Label label_Synth { "", "Synth" };


    ProgramModel *model_ProgramListSynth;
    ProgramModel *model_ProgramListLocal;

    std::unique_ptr<juce::FileChooser> fileChooser;
};