/*
  ==============================================================================

    MainWidget.h
    Created: 9 Sep 2025 11:11:17am
    Author:  Achim Stein

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiComponent.h"

class MidiHandler;

class MainWidget : public juce::TabbedComponent, public MidiComponent
{
public:
    MainWidget(MidiHandler *midiHandler);
    ~MainWidget() override;

    void handlePro800VersionUpdate() override;

private:
    enum TabIds
    {
      TAB_FRONTPANEL = 0,
      TAB_PERFORMANCE,
      TAB_SETTINGS,
      TAB_PROGRAM,
      TAB_ADVANCED
    };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWidget)

};
