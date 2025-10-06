#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "MidiComponent.h"
#include "LocalProgramListBox.h"

class ProgramModel;

class ProgramManagementTab : public juce::Component, public juce::DragAndDropContainer, public MidiComponent
{
public:
    ProgramManagementTab(MidiHandler *handler);
    virtual ~ProgramManagementTab() override;

    void resized() override;

    virtual void handlePro800ProgramDump(std::shared_ptr<ProgramMessage> &programMessage) override;


private:
    void compareSelectedPrograms();

    juce::ListBox listBox_ProgramListSynth;
    LocalProgramListBox listBox_ProgramListLocal;

    juce::Slider spinBox_MaxProgramNumber { juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft };
    juce::TextButton button_RefreshDump { "Refresh Dump" };
    juce::TextButton button_Compare { "Compare" };
    juce::TextButton button_Clear { "Clear" };
    juce::TextButton button_Export { "Export" };
    juce::TextButton button_Import { "Import" };

    juce::TextButton button_SynthToLocal { ">>" };
    juce::TextButton button_SynthToLocalAll { "All >>" };
    juce::TextButton button_LocalToSynth { "<<" };
    juce::TextButton button_LocalToSynthAll { "All <<" };

    juce::Label label_Local { "", "Local" };
    juce::Label label_Synth { "", "Synth" };


    ProgramModel *model_ProgramListSynth;
    ProgramModel *model_ProgramListLocal;

    std::unique_ptr<juce::FileChooser> fileChooser;
};