/*
  ==============================================================================

    EqualSpacingGroupComponent.cpp
    Created: 13 Sep 2025 9:18:45am
    Author:  Achim Stein

  ==============================================================================
*/

#include "EqualSpacingGroupComponent.h"

EqualSpacingGroupComponent::EqualSpacingGroupComponent() : juce::GroupComponent()
{
    this->setTextLabelPosition(juce::Justification::centred);
}

EqualSpacingGroupComponent::EqualSpacingGroupComponent(const juce::String &text, uint8_t outlineAlpha, juce::FlexBox::Direction boxDirection) : EqualSpacingGroupComponent()
{
    this->setText(text);
    this->setDirection(boxDirection);
    
    setOutlineAlpha(outlineAlpha);
}

EqualSpacingGroupComponent::EqualSpacingGroupComponent(const juce::String &text, const juce::Array<juce::Component*> components) : EqualSpacingGroupComponent()
{
    this->setText(text);
    addComponents(components);
}


void EqualSpacingGroupComponent::setDirection(juce::FlexBox::Direction boxDirection)
{
    this->direction = boxDirection;
}

void EqualSpacingGroupComponent::setOutlineAlpha(uint8_t outlineAlpha)
{
    auto outlineColour = this->findColour(outlineColourId);
    this->setColour(outlineColourId, outlineColour.withAlpha(outlineAlpha));
}

void EqualSpacingGroupComponent::resized()
{
    auto area = getLocalBounds().reduced(15);
    
    int minWidth = area.getWidth();
    int minHeight = area.getHeight();
    
    int *directionSize = &minWidth;
    
    if( direction == juce::FlexBox::Direction::column || direction == juce::FlexBox::Direction::columnReverse)
    {
        directionSize = &minHeight;
    }

    *directionSize /= children.size();
    
    
    juce::FlexBox fb;
    fb.flexDirection = direction;

    for( auto *widget : children )
    {
        double itemSpaceFactor = (spaceFactor.contains(widget) ? spaceFactor[widget] : 1.0);
        int unscaledDirectionSize = *directionSize;
        *directionSize = (int)(*directionSize * itemSpaceFactor);
        
        juce::FlexItem item = juce::FlexItem (*widget).withMinWidth(minWidth).withMinHeight(minHeight);
        if ( dynamic_cast<juce::ComboBox*>(widget) != nullptr )
        {
            item = item.withMinHeight(25).withMaxHeight(25);
        }
        
        fb.items.add(item);
        
        *directionSize = unscaledDirectionSize;
    }
    
    fb.performLayout(area);
    
}

void EqualSpacingGroupComponent::addComponent(juce::Component *component, double itemSpaceFactor)
{
    if ( itemSpaceFactor != 1.0 )
    {
        spaceFactor.set(component, itemSpaceFactor);
    }
    
    addComponents( {component} );
}

void EqualSpacingGroupComponent::addComponents(const juce::Array<juce::Component *> components)
{
    for ( auto *component : components)
    {
        this->children.add(component);
        addAndMakeVisible(component);
    }
}

void EqualSpacingGroupComponent::removeComponent(juce::Component *component)
{
    removeComponents({component});
}

void EqualSpacingGroupComponent::removeComponents(juce::Array<juce::Component *> components)
{
    for ( auto *component : components)
    {
        removeChildComponent(component);
        this->children.removeFirstMatchingValue(component);
        spaceFactor.remove(component);
    }
}
