/** 
 * Pro800 Manager Plugin
 * Copyright (C) 2025 Achim Stein
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

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
