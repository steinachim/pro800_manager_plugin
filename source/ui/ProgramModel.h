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

class ProgramMessage;

class ProgramModel : public juce::ListBoxModel
{
public:
    const static inline std::string DRAG_SOURCE_DESCRIPTION = "LocalProgramListBoxDrag";

    enum ModelType
    {
        LOCAL,
        SYNTH
    };

    ProgramModel(ModelType type, juce::ListBox* parent);
    ~ProgramModel() override;

    int getNumRows() override;
    juce::String getNameForRow(int rowNumber) override;
    std::shared_ptr<ProgramMessage > getProgramForRow(int rowNumber);

    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent &event) override;
    void deleteKeyPressed (int lastRowSelected) override;

    juce::var getDragSourceDescription (const juce::SparseSet<int>& selectedRows) override;

    void reset();
    void updateElement(std::shared_ptr<ProgramMessage> message);
    void highlightRow(int row);

private:
    enum NameChangeResult{
        OK,
        CANCEL
    };
    static constexpr std::string NAME_CHANGE_INPUT = "NameChangeInput";
    std::unique_ptr<juce::AlertWindow> nameChangeMessageBox;

    int highlightedRow = -1;

    ModelType modelType;
    juce::Array<std::shared_ptr<ProgramMessage>> rows;

    juce::ListBox* parentListBox = nullptr;
};