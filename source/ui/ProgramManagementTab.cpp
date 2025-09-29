#include "ProgramManagementTab.h"

#include "../midi/ProgramMessage.h"
#include "ProgramModel.h"

ProgramManagementTab::ProgramManagementTab(MidiHandler *midiHandler) : juce::Component(), MidiComponent(midiHandler, false, {MessageType::PRO800_PROGRAM_MESSAGE})
{
    model_ProgramList = new ProgramModel();
    listBox_ProgramList.setModel(model_ProgramList);
    listBox_ProgramList.setMultipleSelectionEnabled(true);

    button_RefreshDump.onClick = [this] {
        //model_ProgramList->clear(); 
        requestProgramDump();
    };

    button_Compare.onClick = [this] { compareSelectedPrograms(); };

    addAndMakeVisible(listBox_ProgramList);
    addAndMakeVisible(button_RefreshDump);
    addAndMakeVisible(button_Compare);
}

ProgramManagementTab::~ProgramManagementTab()
{
    delete model_ProgramList;
}


void ProgramManagementTab::resized()
{
    auto area = getLocalBounds().reduced(4);

    listBox_ProgramList.setBounds(area.removeFromTop(area.getHeight()-30));
    button_RefreshDump.setBounds(area.removeFromRight(100));
    button_Compare.setBounds(area.removeFromRight(100));
}

void ProgramManagementTab::handlePro800ProgramDump(std::shared_ptr<ProgramMessage> &programMessage)
{
    std::cout << "ProgramManagementTab::handlePro800ProgramDump():\n"
              << programMessage->toString() << std::endl;

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

    int firstSize = firstProgram->getRawDataSize();
    int secondSize = secondProgram->getRawDataSize();
    int minNumBytes = std::min(firstSize, secondSize);

    juce::String diffs;
    for( int i = 0; i < minNumBytes; i++ )
    {
        uint8_t firstByte = firstProgram->getRawData()[i];
        uint8_t secondByte = secondProgram->getRawData()[i];

        if (firstProgram->getRawData()[i] != secondProgram->getRawData()[i])
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