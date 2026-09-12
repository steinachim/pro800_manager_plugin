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

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <map>

/**
 * A GroupComponent that lays out its children on a grid of equally sized cells, in the order they
 * were added (a child may span several rows/columns). Note: the layout order is the child order, so
 * do not reorder the children with toFront()/toBack().
 */
class EqualSpacingGroupComponent : public juce::GroupComponent
{
public:
    EqualSpacingGroupComponent();
    EqualSpacingGroupComponent(const juce::String &text, uint8_t outlineAlpha = 255, int numRows = 1, int numCols = 1);

    void setOutlineAlpha(uint8_t outlineAlpha);
    void setInnerMargin(int margin);

    void resized() override;

    void addComponent(juce::Component *component, int rowSpan = 1, int colSpan = 1);
    void addComponents(const juce::Array<juce::Component *> &components, const juce::Array<int> &rowSpans = {}, const juce::Array<int> &colSpans = {});

private:
    struct Span
    {
        int rows = 1;
        int cols = 1;
    };

    Span spanOf(juce::Component *component) const;

    // drops the span of a child that was removed (or destroyed), so that no stale entry can outlive it
    void childrenChanged() override;

    int numRows = 1;
    int numCols = 1;
    int innerMargin = 2;

    std::map<juce::Component *, Span> spans;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualSpacingGroupComponent)
};
