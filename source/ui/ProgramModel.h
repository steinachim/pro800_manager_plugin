#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class ProgramMessage;

class ProgramModel : public juce::ListBoxModel
{
public:
    ProgramModel();
    ~ProgramModel() override;

    int getNumRows() override;
    juce::String getNameForRow(int rowNumber) override;
    std::shared_ptr<ProgramMessage > getProgramForRow(int rowNumber);

    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent &event) override;

    void clear();
    void addElement(std::shared_ptr<ProgramMessage> message);

private:
    juce::Array<std::shared_ptr<ProgramMessage>> rows;


};