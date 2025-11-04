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
    sourceRows.addTokens (dragSourceDetails.description.toString().substring ((int)ProgramModel::DRAG_SOURCE_DESCRIPTION.length() + 1), ",", "");

    if ( sourceRows.isEmpty() )
        return;

    auto sourceListBox = dynamic_cast<juce::ListBox*> (dragSourceDetails.sourceComponent.get());
    if (!sourceListBox)
        return;

    auto sourceModel = dynamic_cast<ProgramModel*> (sourceListBox->getListBoxModel());
    if (!sourceModel)
        return;

    auto localModel = dynamic_cast<ProgramModel*> (getListBoxModel());
    if (!localModel)
        return;

    int firstSourceIndex = sourceRows[0].getIntValue() - 1;
    for (int i = 0; i < sourceRows.size(); ++i)
    {
        int rowIndex = sourceRows[i].getIntValue() - 1;
        if (rowIndex < 0)
            continue;

        int indexDelta = rowIndex - firstSourceIndex;

        int targetRow = this->getRowContainingPosition (dragSourceDetails.localPosition.getX(), dragSourceDetails.localPosition.getY());
        targetRow += indexDelta; // for multiple inserts

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

void LocalProgramListBox::itemDragEnter(const SourceDetails& /*dragSourceDetails*/) 
{
    // nothing to be done
}

void LocalProgramListBox::itemDragMove(const SourceDetails& dragSourceDetails) 
{
    if (!isInterestedInDragSource(dragSourceDetails))
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
    if (!isInterestedInDragSource(dragSourceDetails))
        return; 

    auto localModel = dynamic_cast<ProgramModel*>(getListBoxModel());
    if ( !localModel )
         return;

    localModel->highlightRow(-1);
    this->repaint();
}
