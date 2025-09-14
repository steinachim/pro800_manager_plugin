/*
  ==============================================================================

    PerformanceTab.h
    Created: 9 Sep 2025 11:12:08am
    Author:  Achim Stein

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class PerformanceTab : public juce::Component
{
    public:
        PerformanceTab();
    
        void resized() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerformanceTab)
};
