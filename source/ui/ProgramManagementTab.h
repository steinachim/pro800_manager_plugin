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
    juce::TextButton button_RefreshDump { "Refresh Dump" };
    juce::TextButton button_Compare { "Compare" };

    ProgramModel *model_ProgramList;
};