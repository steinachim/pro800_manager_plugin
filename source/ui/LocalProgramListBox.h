#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class LocalProgramListBox : public juce::ListBox, public juce::DragAndDropTarget
{
public:
    LocalProgramListBox();
    LocalProgramListBox(const juce::String &componentName, juce::ListBoxModel *model);
    ~LocalProgramListBox() override; 

    virtual bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    virtual void itemDropped (const SourceDetails& dragSourceDetails) override;
    virtual void itemDragEnter (const SourceDetails& dragSourceDetails) override;
    virtual void itemDragMove (const SourceDetails& dragSourceDetails) override;
    virtual void itemDragExit (const SourceDetails& dragSourceDetails) override;
};