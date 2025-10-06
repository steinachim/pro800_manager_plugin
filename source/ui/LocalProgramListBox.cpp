#include "LocalProgramListBox.h"

#include "ProgramModel.h"
#include "../midi/ProgramMessage.h"

LocalProgramListBox::LocalProgramListBox() : juce::ListBox()
{
}

LocalProgramListBox::LocalProgramListBox(const juce::String &componentName, juce::ListBoxModel *model) : juce::ListBox(componentName, model)
{
}

LocalProgramListBox::~LocalProgramListBox()
{
}

bool LocalProgramListBox::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) 
{
    bool interested = dragSourceDetails.description.toString().startsWith(ProgramModel::DRAG_SOURCE_DESCRIPTION);
    return interested;
}

void LocalProgramListBox::itemDropped (const SourceDetails& dragSourceDetails)
{
    juce::StringArray sourceRows;
    sourceRows.addTokens (dragSourceDetails.description.toString().substring (ProgramModel::DRAG_SOURCE_DESCRIPTION.length() + 1), ",", "");

    auto sourceListBox = dynamic_cast<juce::ListBox*> (dragSourceDetails.sourceComponent.get());
    if (!sourceListBox)
        return;

    auto sourceModel = dynamic_cast<ProgramModel*> (sourceListBox->getListBoxModel());
    if (!sourceModel)
        return;

    auto localModel = dynamic_cast<ProgramModel*> (getListBoxModel());
    if (!localModel)
        return;

    for (int i = 0; i < sourceRows.size(); ++i)
    {
        int rowIndex = sourceRows[i].getIntValue() - 1;
        if (rowIndex < 0)
            continue;

        int targetRow = this->getRowContainingPosition (dragSourceDetails.localPosition.getX(), dragSourceDetails.localPosition.getY());
        if (targetRow < 0 || targetRow >= localModel->getNumRows())
            continue;

        auto programMessage = sourceModel->getProgramForRow (rowIndex);
        if (!programMessage)
            continue;

        std::shared_ptr<ProgramMessage> targetProgramMessage = std::make_shared<ProgramMessage> (*(programMessage.get()));

        bool switchPrograms = true;
        if (sourceModel != localModel || !targetProgramMessage->isValid())
        {
            switchPrograms = false;
        }

        if (switchPrograms)
        {
            // Switch the program at the target row with the source program
            auto tempProgramMessage = localModel->getProgramForRow (targetRow);
            if (tempProgramMessage && tempProgramMessage->isValid())
            {
                // Set the program number to the source row
                tempProgramMessage->setProgramNumber ((uint16_t) rowIndex);
                localModel->updateElement (tempProgramMessage);
            }
        }

        // Set the program number to the target row
        targetProgramMessage->setProgramNumber ((uint16_t) targetRow);
        localModel->updateElement (targetProgramMessage);


    }
    
    localModel->highlightRow (-1);
    this->deselectAllRows();
    sourceListBox->deselectAllRows();
}

void LocalProgramListBox::itemDragEnter (const SourceDetails& dragSourceDetails) 
{
    // nothing to be done
}

void LocalProgramListBox::itemDragMove (const SourceDetails& dragSourceDetails) 
{
    if (! isInterestedInDragSource(dragSourceDetails))
        return; 

    int targetRow = this->getRowContainingPosition(dragSourceDetails.localPosition.getX(), dragSourceDetails.localPosition.getY());

    auto localModel = dynamic_cast<ProgramModel*>(getListBoxModel());
    if ( !localModel )
         return;

    localModel->highlightRow(targetRow);
    this->repaint();

}

void LocalProgramListBox::itemDragExit (const SourceDetails& dragSourceDetails) 
{
    
    auto localModel = dynamic_cast<ProgramModel*>(getListBoxModel());
    if ( !localModel )
         return;

    localModel->highlightRow(-1);
    this->repaint();
}
