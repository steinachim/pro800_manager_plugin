#include "ProgramManagementTab.h"

#include "../midi/ProgramMessage.h"
#include "ProgramModel.h"

ProgramManagementTab::ProgramManagementTab(MidiHandler *midiHandler) : juce::Component(), MidiComponent(midiHandler, false, {MessageType::PRO800_PROGRAM_MESSAGE})
{
    model_ProgramListSynth = new ProgramModel(ProgramModel::SYNTH);
    model_ProgramListLocal = new ProgramModel(ProgramModel::LOCAL);
    listBox_ProgramListSynth.setModel(model_ProgramListSynth);
    listBox_ProgramListSynth.setMultipleSelectionEnabled(true);

    listBox_ProgramListLocal.setModel(model_ProgramListLocal);
    listBox_ProgramListLocal.setMultipleSelectionEnabled(true);

    spinBox_MaxProgramNumber.setRange(0.0, 399.0, 1.0);

    button_RefreshDump.onClick = [this] {
        requestProgramDump(0, (int)this->spinBox_MaxProgramNumber.getValue()+1);
    };

    button_Compare.onClick = [this] { 
        compareSelectedPrograms(); 
    };

    button_Clear.onClick = [this] {
        model_ProgramListSynth->reset();
        listBox_ProgramListSynth.updateContent();
        listBox_ProgramListSynth.repaint();

        model_ProgramListLocal->reset();
        listBox_ProgramListLocal.updateContent();
        listBox_ProgramListLocal.repaint();
    };

    button_Export.onClick = [this] {
        auto selectedRows = listBox_ProgramListLocal.getSelectedRows();
        if (selectedRows.size() != 1)
        {
            return;
        }

        fileChooser = std::make_unique<juce::FileChooser> ("Please select the filename to export...",
            juce::File::getSpecialLocation (juce::File::userHomeDirectory),
            "*.syx");

        fileChooser->launchAsync (juce::FileBrowserComponent::saveMode, [this, selectedRows] (const juce::FileChooser& chooser) {
            juce::File exportFile (chooser.getResult());

            auto programMessage = model_ProgramListLocal->getProgramForRow (selectedRows[0]);
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
            model_ProgramListLocal->updateElement( programMessage );
            listBox_ProgramListLocal.updateContent();
            listBox_ProgramListLocal.repaint();
        });
    };

    button_SynthToLocal.onClick = [this] {
        auto selectedRows = listBox_ProgramListSynth.getSelectedRows();
        for ( int i = 0; i < selectedRows.size(); i++)
        {
            auto programMessage = model_ProgramListSynth->getProgramForRow(selectedRows[i]);
            model_ProgramListLocal->updateElement(programMessage);
            listBox_ProgramListLocal.updateContent();
            listBox_ProgramListLocal.repaint();
        }
    };

    button_SynthToLocalAll.onClick = [this] {
        for ( int i = 0; i < model_ProgramListSynth->getNumRows(); i++)
        {
            auto programMessage = model_ProgramListSynth->getProgramForRow(i);
            model_ProgramListLocal->updateElement(programMessage);
            listBox_ProgramListLocal.updateContent();
            listBox_ProgramListLocal.repaint();
        }
    };

    button_LocalToSynth.onClick = [this] {
        auto selectedRows = listBox_ProgramListLocal.getSelectedRows();
        for ( int i = 0; i < selectedRows.size(); i++)
        {
            auto programMessage = model_ProgramListLocal->getProgramForRow(selectedRows[i]);
            model_ProgramListSynth->updateElement(programMessage);
            listBox_ProgramListSynth.updateContent();
            listBox_ProgramListSynth.repaint();

            sendProgram(programMessage);
        }
    };

    button_SynthToLocalAll.onClick = [this] {
        for ( int i = 0; i < model_ProgramListLocal->getNumRows(); i++)
        {
            auto programMessage = model_ProgramListLocal->getProgramForRow(i);
            model_ProgramListSynth->updateElement(programMessage);
            listBox_ProgramListSynth.updateContent();
            listBox_ProgramListSynth.repaint();

            sendProgram(programMessage);
        }
    };

    addAndMakeVisible(label_Synth);
    addAndMakeVisible(label_Local);
    addAndMakeVisible(listBox_ProgramListSynth);
    addAndMakeVisible(listBox_ProgramListLocal);

    addAndMakeVisible (button_SynthToLocal);
    addAndMakeVisible (button_SynthToLocalAll);
    addAndMakeVisible (button_LocalToSynth);
    addAndMakeVisible (button_LocalToSynthAll);

    addAndMakeVisible (spinBox_MaxProgramNumber);
    addAndMakeVisible (button_RefreshDump);
    addAndMakeVisible (button_Compare);
    addAndMakeVisible(button_Clear);
    addAndMakeVisible(button_Export);
    addAndMakeVisible(button_Import);
}

ProgramManagementTab::~ProgramManagementTab()
{
    delete model_ProgramListLocal;
    delete model_ProgramListSynth;
}


void ProgramManagementTab::resized()
{
    const int buttonWidth = 120;
    const int buttonHeight = 30;
    auto area = getLocalBounds().reduced(4);

    auto listArea = area.removeFromTop(area.getHeight()-buttonHeight).reduced(0, 4);
    int listBoxWidth = (listArea.getWidth() - buttonWidth)/2; // two lists, buttons in the middle

    label_Synth.setBounds(listArea.withBottom(buttonHeight).withTrimmedRight(listBoxWidth + buttonWidth));
    label_Local.setBounds(listArea.withBottom(buttonHeight).withTrimmedLeft(listBoxWidth + buttonWidth));
    listArea.removeFromTop(buttonHeight);

    listBox_ProgramListSynth.setBounds(listArea.removeFromLeft( listBoxWidth ));
    
    auto moveButtonArea = listArea.removeFromLeft(buttonWidth).reduced(4);
    moveButtonArea.removeFromTop((moveButtonArea.getHeight() - 5*buttonHeight)/2);
    button_SynthToLocal.setBounds(moveButtonArea.removeFromTop(buttonHeight).reduced(0, 4));
    button_SynthToLocalAll.setBounds(moveButtonArea.removeFromTop(buttonHeight).reduced(0, 4));
    moveButtonArea.removeFromTop(buttonHeight);
    button_LocalToSynth.setBounds(moveButtonArea.removeFromTop(buttonHeight).reduced(0, 4));
    button_LocalToSynthAll.setBounds(moveButtonArea.removeFromTop(buttonHeight).reduced(0, 4));

    listBox_ProgramListLocal.setBounds(listArea);

    
    button_Clear.setBounds(area.removeFromRight(120).reduced(4, 0));
    button_Compare.setBounds(area.removeFromRight(120).reduced(4, 0));
    button_RefreshDump.setBounds(area.removeFromRight(120).reduced(4, 0));
    spinBox_MaxProgramNumber.setBounds(area.removeFromRight(120).reduced(4, 0));
    button_Export.setBounds(area.removeFromRight(120).reduced(4, 0));
    button_Import.setBounds(area.removeFromRight(120).reduced(4, 0));
}

void ProgramManagementTab::handlePro800ProgramDump(std::shared_ptr<ProgramMessage> &programMessage)
{
    model_ProgramListSynth->updateElement( programMessage );
    listBox_ProgramListSynth.updateContent();
    listBox_ProgramListSynth.repaint();
}

void ProgramManagementTab::compareSelectedPrograms()
{
    auto selectedRows = listBox_ProgramListSynth.getSelectedRows();
    
    if ( selectedRows.size() != 2)
    {
        return;
    }

    auto firstProgram = model_ProgramListSynth->getProgramForRow(selectedRows[0]);
    auto secondProgram = model_ProgramListSynth->getProgramForRow(selectedRows[1]);

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