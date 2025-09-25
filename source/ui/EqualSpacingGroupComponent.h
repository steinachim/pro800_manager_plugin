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
    EqualSpacingGroupComponent(const juce::String &text, uint8_t outlineAlpha = 255, juce::FlexBox::Direction boxDirection = juce::FlexBox::Direction::row);
    EqualSpacingGroupComponent(const juce::String &text, const juce::Array<juce::Component*> components);

    void setOutlineAlpha(uint8_t outlineAlpha);
    void setDirection(juce::FlexBox::Direction boxDirection);
    
    void resized() override;
    
    void addComponent(juce::Component *component, double spaceFactor = 1.0);
    void addComponents(const juce::Array<juce::Component *> components);
    void removeComponent(juce::Component *component);
    void removeComponents(juce::Array<juce::Component *> components);

private:
    juce::FlexBox::Direction direction = juce::FlexBox::Direction::row;
    juce::Array<juce::Component*> children;
    juce::HashMap<juce::Component*, double> spaceFactor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualSpacingGroupComponent)
};
