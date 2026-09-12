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
    this->setTextLabelPosition (juce::Justification::left); // every group in this plugin uses a left-aligned title
}

EqualSpacingGroupComponent::EqualSpacingGroupComponent (const juce::String& text, uint8_t outlineAlpha, int rows, int cols) : EqualSpacingGroupComponent()
{
    this->setText (text);
    this->numRows = juce::jmax (1, rows); // a zero-sized grid would divide by zero in resized()
    this->numCols = juce::jmax (1, cols);

    setOutlineAlpha (outlineAlpha);
}

void EqualSpacingGroupComponent::setOutlineAlpha (uint8_t outlineAlpha)
{
    auto outlineColour = this->findColour (outlineColourId);
    this->setColour (outlineColourId, outlineColour.withAlpha (outlineAlpha));
}

void EqualSpacingGroupComponent::setInnerMargin (int margin)
{
    this->innerMargin = margin;
}

void EqualSpacingGroupComponent::resized()
{
    auto area = getLocalBounds().reduced (10);
    area.removeFromTop (15);

    const int fullWidth = area.getWidth();
    const int fullHeight = area.getHeight();

    const int colWidth = fullWidth / numCols;
    const int rowHeight = fullHeight / numRows;

    int col = 0;
    int row = 0;
    for (auto* widget : getChildren())
    {
        const Span span = spanOf (widget);
        const int widgetHeight = fullHeight * span.rows / numRows;
        const int widgetWidth = fullWidth * span.cols / numCols;
        widget->setBounds (area.withTrimmedLeft (col * colWidth).withTrimmedTop (row * rowHeight).withWidth (widgetWidth).withHeight (widgetHeight).reduced (innerMargin));

        col += span.cols;
        if (col >= numCols) // >=: a span that overshoots the row must still wrap
        {
            col = 0;
            row += span.rows;
        }
    }
}

void EqualSpacingGroupComponent::addComponent (juce::Component* component, int rows, int cols)
{
    addComponents ({ component }, { rows }, { cols });
}

void EqualSpacingGroupComponent::addComponents (const juce::Array<juce::Component*>& components, const juce::Array<int>& rowSpans, const juce::Array<int>& colSpans)
{
    for (int i = 0; i < components.size(); i++)
    {
        // spans default to 1 where no (or no more) values were given
        Span span;
        span.rows = i < rowSpans.size() ? juce::jmax (1, rowSpans[i]) : 1;
        span.cols = i < colSpans.size() ? juce::jmax (1, colSpans[i]) : 1;

        this->spans[components[i]] = span;
        addAndMakeVisible (components[i]);
    }
}

EqualSpacingGroupComponent::Span EqualSpacingGroupComponent::spanOf (juce::Component* component) const
{
    const auto entry = this->spans.find (component);
    return entry != this->spans.end() ? entry->second : Span();
}

void EqualSpacingGroupComponent::childrenChanged()
{
    const auto& children = getChildren();
    for (auto entry = this->spans.begin(); entry != this->spans.end();)
    {
        entry = children.contains (entry->first) ? std::next (entry) : this->spans.erase (entry);
    }
}
