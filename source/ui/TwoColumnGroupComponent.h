#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TwoColumnGroupComponent : public juce::GroupComponent
{
public:
    TwoColumnGroupComponent(const juce::String &title, int numRows = -1);
    ~TwoColumnGroupComponent() override;
    void resized() override;

    void addComponentPairs(const juce::Array<juce::Component *> &leftColumn, const juce::Array<juce::Component *> &rightColumn);
    void addLabelledComponents(const juce::Array<juce::String> &labels, const juce::Array<juce::Component *> &components);

private:
    int numRows = -1;
    juce::Array<juce::Component*> leftComponents;
    juce::Array<juce::Component*> rightComponents;
    juce::Array<juce::Component*> componentsToDelete;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TwoColumnGroupComponent)
};
