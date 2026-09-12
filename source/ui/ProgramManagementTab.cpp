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

#include "ProgramManagementTab.h"

#include "../midi/Pro800MessageFactory.h"
#include "../midi/ProgramMessage.h"
#include "ProgramModel.h"

ProgramManagementTab::ProgramManagementTab (MidiHandler* midiHandler) : juce::Component(), MidiComponent (midiHandler, false, { MessageType::PRO800_PROGRAM })
{
    model_ProgramListSynth = std::make_unique<ProgramModel> (ProgramModel::SYNTH, &listBox_ProgramListSynth);
    model_ProgramListLocal = std::make_unique<ProgramModel> (ProgramModel::LOCAL, &listBox_ProgramListLocal);
    listBox_ProgramListSynth.setModel (model_ProgramListSynth.get());
    listBox_ProgramListSynth.setMultipleSelectionEnabled (true);

    listBox_ProgramListLocal.setModel (model_ProgramListLocal.get());
    listBox_ProgramListLocal.setMultipleSelectionEnabled (true);

    button_RefreshDump.onClick = [this] {
        // the list only ever shows what the synth answered to the latest dump request
        model_ProgramListSynth->reset();
        requestProgramDump();
    };

    // clang-format off
    button_Compare.onClick = [this] { compareSelectedPrograms(); };
    button_Load.onClick    = [this] { loadSelectedProgram(); };
    button_Export.onClick  = [this] { exportPrograms(); };
    button_Import.onClick  = [this] { importPrograms(); };
    // clang-format on

    button_SynthToLocal.onClick = [this] {
        copyPrograms (validPrograms (*model_ProgramListSynth, listBox_ProgramListSynth.getSelectedRows()), *model_ProgramListLocal, false);
    };

    button_SynthToLocalAll.onClick = [this] {
        copyPrograms (validPrograms (*model_ProgramListSynth, allRows (*model_ProgramListSynth)), *model_ProgramListLocal, false);
    };

    button_LocalToSynth.onClick = [this] {
        copyPrograms (validPrograms (*model_ProgramListLocal, listBox_ProgramListLocal.getSelectedRows()), *model_ProgramListSynth, true);
    };

    button_LocalToSynthAll.onClick = [this] { sendAllProgramsToSynth(); };

    button_CancelTransfer.onClick = [this] { getMidiHandler().cancelBackgroundSending(); };
    progressBar_Transfer.setPercentageDisplay (false);

    addAndMakeVisible (label_Synth);
    addAndMakeVisible (label_Local);
    addAndMakeVisible (listBox_ProgramListSynth);
    addAndMakeVisible (listBox_ProgramListLocal);

    addAndMakeVisible (button_SynthToLocal);
    addAndMakeVisible (button_SynthToLocalAll);
    addAndMakeVisible (button_LocalToSynth);
    addAndMakeVisible (button_LocalToSynthAll);

    addAndMakeVisible (button_Load);
    addAndMakeVisible (button_RefreshDump);
    addAndMakeVisible (button_Compare);
    addAndMakeVisible (button_Export);
    addAndMakeVisible (button_Import);

    addChildComponent (progressBar_Transfer); // hidden until a transfer starts
    addChildComponent (button_CancelTransfer);

    getMidiHandler().addListener (this);
}

ProgramManagementTab::~ProgramManagementTab()
{
    getMidiHandler().removeListener (this);
}

//==============================================================================
void ProgramManagementTab::backgroundSendingProgress (const juce::String& description, int numSent, int numTotal)
{
    if (!progressBar_Transfer.isVisible())
    {
        setTransferRunning (true);
    }

    transferProgress = numTotal > 0 ? (double) numSent / (double) numTotal : 0.0;
    progressBar_Transfer.setTextToDisplay (description + " " + juce::String (numSent) + "/" + juce::String (numTotal));
}

void ProgramManagementTab::backgroundSendingFinished (bool /*cancelled*/)
{
    setTransferRunning (false);
}

void ProgramManagementTab::setTransferRunning (bool running)
{
    transferProgress = 0.0;
    progressBar_Transfer.setVisible (running);
    button_CancelTransfer.setVisible (running);

    // everything that would start another transfer (and thereby cancel this one)
    button_RefreshDump.setEnabled (!running);
    button_LocalToSynth.setEnabled (!running);
    button_LocalToSynthAll.setEnabled (!running);
}

void ProgramManagementTab::resized()
{
    const int buttonWidth = 120;
    const int buttonHeight = 30;
    auto area = getLocalBounds().reduced (4);

    auto listArea = area.removeFromTop (area.getHeight() - buttonHeight).reduced (0, 4);
    int listBoxWidth = (listArea.getWidth() - buttonWidth) / 2; // two lists, buttons in the middle

    label_Synth.setBounds (listArea.withBottom (buttonHeight).withTrimmedRight (listBoxWidth + buttonWidth));
    label_Local.setBounds (listArea.withBottom (buttonHeight).withTrimmedLeft (listBoxWidth + buttonWidth));
    listArea.removeFromTop (buttonHeight);

    listBox_ProgramListSynth.setBounds (listArea.removeFromLeft (listBoxWidth));

    auto moveButtonArea = listArea.removeFromLeft (buttonWidth).reduced (4);
    moveButtonArea.removeFromTop ((moveButtonArea.getHeight() - 5 * buttonHeight) / 2);
    button_SynthToLocal.setBounds (moveButtonArea.removeFromTop (buttonHeight).reduced (0, 4));
    button_SynthToLocalAll.setBounds (moveButtonArea.removeFromTop (buttonHeight).reduced (0, 4));
    moveButtonArea.removeFromTop (buttonHeight);
    button_LocalToSynth.setBounds (moveButtonArea.removeFromTop (buttonHeight).reduced (0, 4));
    button_LocalToSynthAll.setBounds (moveButtonArea.removeFromTop (buttonHeight).reduced (0, 4));

    listBox_ProgramListLocal.setBounds (listArea);

    button_Compare.setBounds (area.removeFromLeft (120).reduced (4, 0));
    button_RefreshDump.setBounds (area.removeFromLeft (120).reduced (4, 0));
    button_Load.setBounds (area.removeFromLeft (120).reduced (4, 0));

    button_Export.setBounds (area.removeFromRight (120).reduced (4, 0));
    button_Import.setBounds (area.removeFromRight (120).reduced (4, 0));

    // the free space between the button groups shows the progress while a transfer is running
    button_CancelTransfer.setBounds (area.removeFromRight (100).reduced (4, 0));
    progressBar_Transfer.setBounds (area.reduced (4, 0));
}

void ProgramManagementTab::handlePro800ProgramDump (const std::shared_ptr<ProgramMessage>& programMessage)
{
    model_ProgramListSynth->updateElement (programMessage);
}

//==============================================================================
ProgramManagementTab::ProgramList ProgramManagementTab::validPrograms (ProgramModel& model, const juce::SparseSet<int>& rows)
{
    ProgramList programs;

    for (int i = 0; i < rows.size(); i++)
    {
        auto program = model.getProgramForRow (rows[i]);
        if (program && program->isValid())
        {
            programs.push_back (program);
        }
    }

    return programs;
}

juce::SparseSet<int> ProgramManagementTab::allRows (ProgramModel& model)
{
    juce::SparseSet<int> rows;
    rows.addRange (juce::Range<int> (0, model.getNumRows()));
    return rows;
}

void ProgramManagementTab::copyPrograms (const ProgramList& programs, ProgramModel& to, bool sendToSynth)
{
    // Only valid programs get here: copying an empty slot would erase the target slot, and on the
    // synth that means overwriting a stored preset with "uninitialized".
    ProgramList copies;
    copies.reserve (programs.size());

    for (const auto& program : programs)
    {
        // the two lists must never share objects: renaming in one list must not change the other
        auto copy = std::make_shared<ProgramMessage> (*program);
        to.updateElement (copy);
        copies.push_back (copy);
    }

    if (sendToSynth)
    {
        sendPrograms (copies);
    }
}

void ProgramManagementTab::sendAllProgramsToSynth()
{
    const ProgramList programs = validPrograms (*model_ProgramListLocal, allRows (*model_ProgramListLocal));

    if (programs.empty())
    {
        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "Nothing to send", "The local list does not contain any programs.");
        return;
    }

    juce::Component::SafePointer<ProgramManagementTab> safeThis (this);
    auto* callback = juce::ModalCallbackFunction::create ([safeThis, programs] (int result) {
        if (result == 1 && safeThis != nullptr)
        {
            safeThis->copyPrograms (programs, *safeThis->model_ProgramListSynth, true);
        }
    });

    juce::AlertWindow::showOkCancelBox (juce::MessageBoxIconType::WarningIcon,
        "Send all programs to the synth",
        juce::String (programs.size()) + " programs will be written to the Pro-800, overwriting the presets stored in those slots. Continue?",
        "Send",
        "Cancel",
        this,
        callback);
}

void ProgramManagementTab::loadSelectedProgram()
{
    auto selectedRows = listBox_ProgramListSynth.getSelectedRows();
    if (selectedRows.isEmpty())
    {
        return;
    }

    auto programMessage = model_ProgramListSynth->getProgramForRow (selectedRows[0]);
    if (!programMessage)
    {
        return;
    }

    loadProgram (*programMessage);
}

//==============================================================================
void ProgramManagementTab::exportPrograms()
{
    auto selectedRows = listBox_ProgramListLocal.getSelectedRows();
    juce::String exportTitle = "Export selected programs to file";
    if (selectedRows.isEmpty())
    {
        exportTitle = "Export all programs to file";
        selectedRows = allRows (*model_ProgramListLocal);
    }

    fileChooser = std::make_unique<juce::FileChooser> (exportTitle,
        juce::File::getSpecialLocation (juce::File::userHomeDirectory),
        "*.syx");

    juce::Component::SafePointer<ProgramManagementTab> safeThis (this);
    const int flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting;

    fileChooser->launchAsync (flags, [safeThis, selectedRows] (const juce::FileChooser& chooser) {
        if (safeThis == nullptr)
        {
            return;
        }

        juce::File exportFile (chooser.getResult());
        if (exportFile == juce::File())
        {
            return; // cancelled
        }

        // empty slots are not exported: they would come back as garbage programs on import
        const ProgramList programs = validPrograms (*safeThis->model_ProgramListLocal, selectedRows);

        juce::MemoryBlock fileData;
        for (const auto& program : programs)
        {
            fileData.append (program->getRawData().data(), program->getRawDataSize());
        }

        if (!exportFile.replaceWithData (fileData.getData(), fileData.getSize()))
        {
            juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Export failed", "Could not write " + exportFile.getFullPathName());
            return;
        }

        juce::Logger::writeToLog ("Exported " + juce::String (programs.size()) + " programs to " + exportFile.getFullPathName());
    });
}

void ProgramManagementTab::importPrograms()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Please select the file to import...",
        juce::File::getSpecialLocation (juce::File::userHomeDirectory),
        "*.syx");

    juce::Component::SafePointer<ProgramManagementTab> safeThis (this);
    const int flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync (flags, [safeThis] (const juce::FileChooser& chooser) {
        if (safeThis == nullptr)
        {
            return;
        }

        juce::File importFile (chooser.getResult());
        if (!importFile.existsAsFile())
        {
            return; // cancelled
        }

        juce::MemoryBlock fileData;
        if (!importFile.loadFileAsData (fileData))
        {
            juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Import failed", "Could not read " + importFile.getFullPathName());
            return;
        }

        // split the file into SysEx messages (F0 ... F7) and keep only those that parse as valid programs
        const auto* bytes = static_cast<const uint8_t*> (fileData.getData());
        const size_t numBytes = fileData.getSize();

        int numImported = 0;
        int numSkipped = 0;
        size_t start = 0;
        for (size_t i = 0; i < numBytes; i++)
        {
            if (i == start && bytes[i] != 0xF0)
            {
                // not at a SysEx start: the rest of the file is not usable
                numSkipped++;
                break;
            }

            if (bytes[i] == 0xF7)
            {
                const juce::MidiMessage sysEx (bytes + start, (int) (i - start + 1));
                auto program = std::dynamic_pointer_cast<ProgramMessage> (Pro800MessageFactory::createMidiMessage (sysEx));

                if (program && program->isValid() && safeThis->model_ProgramListLocal->updateElement (program))
                {
                    numImported++;
                }
                else
                {
                    numSkipped++;
                }

                start = i + 1;
            }
        }

        if (numSkipped > 0 || numImported == 0)
        {
            juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "Import", "Imported " + juce::String (numImported) + " programs. " + juce::String (numSkipped) + " messages in the file are not valid Pro-800 programs and were skipped.");
        }
    });
}

//==============================================================================
void ProgramManagementTab::compareSelectedPrograms()
{
    auto selectedRows = listBox_ProgramListSynth.getSelectedRows();

    if (selectedRows.size() != 2)
    {
        return;
    }

    auto firstProgram = model_ProgramListSynth->getProgramForRow (selectedRows[0]);
    auto secondProgram = model_ProgramListSynth->getProgramForRow (selectedRows[1]);
    if (!firstProgram || !secondProgram)
    {
        return;
    }

    const auto& firstData = firstProgram->getRawData();
    const auto& secondData = secondProgram->getRawData();
    const int firstSize = (int) firstData.size();
    const int secondSize = (int) secondData.size();
    const int minNumBytes = std::min (firstSize, secondSize);

    juce::String diffs;
    for (int i = 0; i < minNumBytes; i++)
    {
        const uint8_t firstByte = firstData[(size_t) i];
        const uint8_t secondByte = secondData[(size_t) i];

        if (firstByte != secondByte)
        {
            const int dataOffset = i - (int) Pro800DataMessage::DATA_START_POS; // negative for header bytes
            diffs += juce::String::formatted ("byte: %3d (offset: %3d) - 0x%02x (%3d)  <=>   0x%02x (%3d)\n", i, dataOffset, firstByte, firstByte, secondByte, secondByte);
        }
    }

    if (firstSize != secondSize)
    {
        diffs += juce::String::formatted ("different sizes: %d  <=>  %d\n", firstSize, secondSize);
    }

    if (diffs.isEmpty())
    {
        diffs += "No differences found\n";
    }

    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "Program Diff", diffs);
}
