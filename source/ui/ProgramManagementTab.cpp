#include "ProgramManagementTab.h"

#include "../midi/ProgramMessage.h"
#include "ProgramModel.h"

ProgramManagementTab::ProgramManagementTab(MidiHandler *midiHandler) : juce::Component(), MidiComponent(midiHandler, false, {MessageType::PRO800_PROGRAM_MESSAGE})
{
    model_ProgramList = new ProgramModel();
    listBox_ProgramList.setModel(model_ProgramList);
    listBox_ProgramList.setMultipleSelectionEnabled(true);

    spinBox_MaxProgramNumber.setRange(0.0, 399.0, 1.0);

    button_RefreshDump.onClick = [this] {
        requestProgramDump(0, (int)this->spinBox_MaxProgramNumber.getValue()+1);
    };

    button_Compare.onClick = [this] { 
        compareSelectedPrograms(); 
    };

    button_Clear.onClick = [this] {
        //model_ProgramList->clear();

        auto selectedRows = listBox_ProgramList.getSelectedRows();
        if (selectedRows.size() != 1)
        {
            return;
        }

        auto program = model_ProgramList->getProgramForRow(selectedRows[0]);
        program->setProgramName("ABCDEFGHIJKLMNOPQ");

        listBox_ProgramList.updateContent();
    };

    button_Export.onClick = [this] {
        auto selectedRows = listBox_ProgramList.getSelectedRows();
        if (selectedRows.size() != 1)
        {
            return;
        }

        fileChooser = std::make_unique<juce::FileChooser> ("Please select the filename to export...",
            juce::File::getSpecialLocation (juce::File::userHomeDirectory),
            "*.syx");

        fileChooser->launchAsync (juce::FileBrowserComponent::saveMode, [this, selectedRows] (const juce::FileChooser& chooser) {
            juce::File exportFile (chooser.getResult());

            auto programMessage = model_ProgramList->getProgramForRow (selectedRows[0]);
            exportFile.replaceWithData(programMessage->getRawData()->data(), programMessage->getRawData()->size());
        });
    };

    button_Import.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser> ("Please select the file to import...",
            juce::File::getSpecialLocation (juce::File::userHomeDirectory),
            "*.syx");

        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& chooser) {
            juce::File importFile (chooser.getResult());

            if ( !importFile.exists() )
            {
                return;
            }

            juce::MemoryBlock memBlock;
            importFile.loadFileAsData(memBlock);
            std::shared_ptr<ProgramMessage> programMessage(new ProgramMessage((const uint8_t*)memBlock.getData(), (int)memBlock.getSize()));            
            this->handlePro800ProgramDump(programMessage);
        });
    };

    addAndMakeVisible(listBox_ProgramList);

    addAndMakeVisible(spinBox_MaxProgramNumber);
    addAndMakeVisible(button_RefreshDump);
    addAndMakeVisible(button_Compare);
    addAndMakeVisible(button_Clear);
    addAndMakeVisible(button_Export);
    addAndMakeVisible(button_Import);
}

ProgramManagementTab::~ProgramManagementTab()
{
    delete model_ProgramList;
}


void ProgramManagementTab::resized()
{
    auto area = getLocalBounds().reduced(4);

    listBox_ProgramList.setBounds(area.removeFromTop(area.getHeight()-30));
    
    button_Clear.setBounds(area.removeFromRight(100));
    button_Compare.setBounds(area.removeFromRight(100));
    button_RefreshDump.setBounds(area.removeFromRight(100));
    spinBox_MaxProgramNumber.setBounds(area.removeFromRight(100));
    button_Export.setBounds(area.removeFromRight(100));
    button_Import.setBounds(area.removeFromRight(100));
}

void ProgramManagementTab::handlePro800ProgramDump(std::shared_ptr<ProgramMessage> &programMessage)
{
    model_ProgramList->addElement( programMessage );
    listBox_ProgramList.updateContent();
}

void ProgramManagementTab::compareSelectedPrograms()
{
    auto selectedRows = listBox_ProgramList.getSelectedRows();
    
    if ( selectedRows.size() != 2)
    {
        return;
    }

    auto firstProgram = model_ProgramList->getProgramForRow(selectedRows[0]);
    auto secondProgram = model_ProgramList->getProgramForRow(selectedRows[1]);

    size_t firstSize = firstProgram->getRawData()->size();
    size_t secondSize = secondProgram->getRawData()->size();
    size_t minNumBytes = std::min(firstSize, secondSize);

    juce::String diffs;
    for( size_t i = 0; i < minNumBytes; i++ )
    {
        uint8_t firstByte = firstProgram->getRawData()->at(i);
        uint8_t secondByte = secondProgram->getRawData()->at(i);

        if (firstByte != secondByte)
        {
            diffs += juce::String::formatted("byte: %3d (offset: %3d) - 0x%02x (%3d)  <=>   0x%02x (%3d)\n", i, i - Pro800MidiMessage::POS_MESSAGE_START, firstByte, firstByte, secondByte, secondByte);
        }
    }

    if ( firstSize != secondSize )
    {
        diffs += juce::String::formatted("different sizes: %d  <=>  %d\n", firstSize, secondSize);
    }

    if ( diffs.isEmpty() )
    {
        diffs += "No differences found\n";
    }

    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Program Diff", diffs);

}