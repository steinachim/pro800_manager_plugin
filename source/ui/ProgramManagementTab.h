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
#include "MainWidget.h"

#include <memory>
#include <vector>

class ProgramModel;

class ProgramManagementTab : public juce::Component, public juce::DragAndDropContainer, public MidiComponent
{
public:
    ProgramManagementTab(MidiHandler *handler, MainWidget *parent);
    virtual ~ProgramManagementTab() override;

    void resized() override;

    virtual void handlePro800ProgramDump(std::shared_ptr<ProgramMessage> &programMessage) override;

private:
    using ProgramList = std::vector<std::shared_ptr<ProgramMessage>>;

    void loadSelectedProgram();
    void compareSelectedPrograms();
    void exportPrograms();
    void importPrograms();
    void sendAllProgramsToSynth();

    /** The valid (non-empty) programs of the given rows. */
    static ProgramList validPrograms(ProgramModel &model, const juce::SparseSet<int> &rows);
    static juce::SparseSet<int> allRows(ProgramModel &model);

    /** Puts deep copies of the programs into the target list; optionally also sends them to the synth. */
    void copyPrograms(const ProgramList &programs, ProgramModel &to, bool sendToSynth);

    juce::ListBox listBox_ProgramListSynth;
    LocalProgramListBox listBox_ProgramListLocal;

    juce::TextButton button_RefreshDump { "Refresh Dump" };
    juce::TextButton button_Compare { "Compare" };
    juce::TextButton button_Load { "Load" };

    juce::TextButton button_Export { "Export" };
    juce::TextButton button_Import { "Import" };

    juce::TextButton button_SynthToLocal { ">>" };
    juce::TextButton button_SynthToLocalAll { "All >>" };
    juce::TextButton button_LocalToSynth { "<<" };
    juce::TextButton button_LocalToSynthAll { "All <<" };

    juce::Label label_Local { "", "Local" };
    juce::Label label_Synth { "", "Synth" };

    std::unique_ptr<ProgramModel> model_ProgramListSynth;
    std::unique_ptr<ProgramModel> model_ProgramListLocal;

    std::unique_ptr<juce::FileChooser> fileChooser;

    MainWidget *mainWidget = nullptr; // non-owning: MainWidget owns this tab

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramManagementTab)
};
