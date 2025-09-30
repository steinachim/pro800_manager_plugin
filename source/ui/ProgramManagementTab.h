#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "MidiComponent.h"

class ProgramModel;

class ProgramManagementTab : public juce::Component, public MidiComponent
{
public:
    ProgramManagementTab(MidiHandler *handler);
    virtual ~ProgramManagementTab() override;

    void resized() override;

    virtual void handlePro800ProgramDump(std::shared_ptr<ProgramMessage> &programMessage) override;


private:
    void compareSelectedPrograms();

    juce::ListBox listBox_ProgramList;
    juce::Slider spinBox_MaxProgramNumber { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft };
    juce::TextButton button_RefreshDump { "Refresh Dump" };
    juce::TextButton button_Compare { "Compare" };
    juce::TextButton button_Clear { "Clear" };

    ProgramModel *model_ProgramList;
};