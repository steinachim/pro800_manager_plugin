/*
  ==============================================================================

    EqualSpacingGroupComponent.h
    Created: 13 Sep 2025 9:18:57am
    Author:  Achim Stein

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>


class EqualSpacingGroupComponent : public juce::GroupComponent
{
public:
    EqualSpacingGroupComponent();
    EqualSpacingGroupComponent(const juce::String &text, uint8_t outlineAlpha = 255, int numRows = 1, int numCols = 1);
    
    void setOutlineAlpha(uint8_t outlineAlpha);
    
    void resized() override;
    
    void addComponent(juce::Component *component, int rowSpan = 1, int colSpan = 1);
    void addComponents(const juce::Array<juce::Component *> &components, const juce::Array<int> rowSpan = {}, juce::Array<int> colSpan = {} );
    void removeComponent(juce::Component *component);
    void removeComponents(juce::Array<juce::Component *> components);

private:
    int numRows;
    int numCols;

    juce::Array<juce::Component*> children;
    juce::HashMap<juce::Component*, int> rowSpan;
    juce::HashMap<juce::Component*, int> colSpan;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualSpacingGroupComponent)
};
