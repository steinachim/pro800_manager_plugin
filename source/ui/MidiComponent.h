#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Pro800Constants.h"

class MidiHandler;
class SettingsMessage;

class MidiComponent
{
public:
    MidiComponent(MidiHandler *midiHandler, bool registerSettings = false);
    virtual ~MidiComponent();

    void handlePro800SettingsMessage(std::shared_ptr<SettingsMessage> &settingsMessage);
    virtual void handlePro800SettingsUpdate();

protected:
    void setupMidiCCComponent(uint8_t midiCC, juce::Component *component);
    void removeMidiCCComponent(uint8_t midiCC, juce::Component *component);

    void setupMidiLogComponent(juce::Component *component);
    void removeMidiLogComponent(juce::Component *component);

    std::shared_ptr<SettingsMessage> &getCurrentSettings();
    void updateSettings(Pro800Settings setting, int value);
    
private:
    bool registerSettings = false;
    MidiHandler *midiHandler;
    std::shared_ptr<SettingsMessage> currentSettings = std::shared_ptr<SettingsMessage>();
};