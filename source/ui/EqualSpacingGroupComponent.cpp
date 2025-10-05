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

EqualSpacingGroupComponent::EqualSpacingGroupComponent(const juce::String &text, uint8_t outlineAlpha, int rows, int cols) : EqualSpacingGroupComponent()
{
    this->setText(text);
    this->numRows = rows;
    this->numCols = cols;
    
    setOutlineAlpha(outlineAlpha);
}

void EqualSpacingGroupComponent::setOutlineAlpha(uint8_t outlineAlpha)
{
    auto outlineColour = this->findColour(outlineColourId);
    this->setColour(outlineColourId, outlineColour.withAlpha(outlineAlpha));
}

void EqualSpacingGroupComponent::setInnerMargin(int margin)
{
    this->innerMargin = margin;
}

void EqualSpacingGroupComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(15);
    
    int fullWidth = area.getWidth();
    int fullHeight = area.getHeight();

    int colWidth = fullWidth/numCols;
    int rowHeight = fullHeight/numRows;

    int col = 0;
    int row = 0;
    for( auto *widget : children )
    {
       int widgetHeight = (int)(fullHeight * rowSpan[widget] / numRows);
       int widgetWidth = (int)(fullWidth * colSpan[widget] / numCols);
       widget->setBounds(area.withTrimmedLeft(col * colWidth).withTrimmedTop(row * rowHeight).withWidth(widgetWidth).withHeight(widgetHeight).reduced(innerMargin));

       col+= colSpan[widget];
       if ( col == numCols )
       {
        col = 0;
        row+= rowSpan[widget];
       }
    } 
}

void EqualSpacingGroupComponent::addComponent(juce::Component *component, int rows, int cols)
{
    addComponents( {component}, {rows}, {cols});
}

void EqualSpacingGroupComponent::addComponents(const juce::Array<juce::Component *> &components, const juce::Array<int> rows, const juce::Array<int> cols)
{
    for ( int i = 0; i < components.size(); i++ )
    {
        this->rowSpan.set(components[i], rows.isEmpty() ? 1 : rows[i]);
        this->colSpan.set(components[i], cols.isEmpty() ? 1 : cols[i]);
        this->children.add(components[i]);
        addAndMakeVisible(components[i]);
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
        rowSpan.remove(component);
        colSpan.remove(component);
    }
}
