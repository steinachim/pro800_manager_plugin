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