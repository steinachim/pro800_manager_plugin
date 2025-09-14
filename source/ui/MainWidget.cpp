/*
  ==============================================================================

    MainWidget.cpp
    Created: 9 Sep 2025 11:11:03am
    Author:  Achim Stein

  ==============================================================================
*/

#include "MainWidget.h"

#include "FrontPanelTab.h"
#include "SettingsTab.h"
#include "AdvancedTab.h"
#include "PerformanceTab.h"

MainWidget::MainWidget(MidiHandler *midiHandler) : TabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    auto colour = findColour (juce::ResizableWindow::backgroundColourId);

    addTab("Front Panel", colour, new FrontPanelTab(midiHandler), true);
    addTab("Performance", colour, new PerformanceTab(), true);
    addTab("Settings", colour, new SettingsTab(midiHandler), true);
    addTab("Advanced", colour, new AdvancedTab(midiHandler), true);
    
}

MainWidget::~MainWidget()
{
    
}
