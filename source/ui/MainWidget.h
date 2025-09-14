/*
  ==============================================================================

    MainWidget.h
    Created: 9 Sep 2025 11:11:17am
    Author:  Achim Stein

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class MidiHandler;

class MainWidget : public juce::TabbedComponent
{
public:
    MainWidget(MidiHandler *midiHandler);
    ~MainWidget();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWidget)

};
