#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Pro800Constants.h"

class MidiHandler;
class SettingsMessage;
class VersionMessage;
class Pro800MidiMessage;

class MidiComponent
{
public:
    MidiComponent(MidiHandler *midiHandler, const juce::Array<Pro800MessageType> messageTypes = juce::Array<Pro800MessageType>());
    virtual ~MidiComponent();

    void handlePro800Message(Pro800MessageType type, std::shared_ptr<Pro800MidiMessage> &settingsMessage);
    virtual void handlePro800SettingsUpdate();
    virtual void handlePro800VersionUpdate();

protected:
    void setupMidiCCComponent(uint8_t midiCC, juce::Component *component);
    void removeMidiCCComponent(uint8_t midiCC, juce::Component *component);

    void setupMidiLogComponent(juce::Component *component);
    void removeMidiLogComponent(juce::Component *component);

    std::shared_ptr<SettingsMessage> &getCurrentSettings();
    void updateSettings(Pro800Settings setting, int value);

    std::shared_ptr<VersionMessage> &getCurrentVersion();
    
private:
    juce::Array<Pro800MessageType> registeredMessageTypes = juce::Array<Pro800MessageType>();
    MidiHandler *midiHandler;

    std::shared_ptr<SettingsMessage> currentSettings = std::shared_ptr<SettingsMessage>();
    std::shared_ptr<VersionMessage> currentVersion = std::shared_ptr<VersionMessage>();
};