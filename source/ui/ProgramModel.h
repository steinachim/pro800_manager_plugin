#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class ProgramMessage;

class ProgramModel : public juce::ListBoxModel
{
public:
    enum ModelType
    {
        LOCAL,
        SYNTH
    };

    ProgramModel(ModelType type);
    ~ProgramModel() override;

    int getNumRows() override;
    juce::String getNameForRow(int rowNumber) override;
    std::shared_ptr<ProgramMessage > getProgramForRow(int rowNumber);

    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent &event) override;

    void reset();
    void updateElement(std::shared_ptr<ProgramMessage> message);

private:
    enum NameChangeResult{
        OK,
        CANCEL
    };
    static constexpr std::string NAME_CHANGE_INPUT = "NameChangeInput";
    std::unique_ptr<juce::AlertWindow> nameChangeMessageBox;

    ModelType modelType;
    juce::Array<std::shared_ptr<ProgramMessage>> rows;
};