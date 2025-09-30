#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../Pro800Constants.h"

class MidiHandler;
class SettingsMessage;
class VersionMessage;
class ProgramMessage;
class Pro800MidiMessage;

class MidiComponent
{
public:
    const juce::String RADIO_VALUE_PROPERTY {"radioValue"};

    MidiComponent(MidiHandler *midiHandler, bool registerMidiCC = false, const juce::Array<MessageType> messageTypes = juce::Array<MessageType>());
    virtual ~MidiComponent();

    void handlePro800Message(MessageType type, std::shared_ptr<Pro800MidiMessage> &settingsMessage);
    void handleMidiCCMessage(uint8_t midiCC, uint8_t value);

    virtual void handlePro800SettingsUpdate();
    virtual void handlePro800VersionUpdate();
    virtual void handlePro800ProgramDump(std::shared_ptr<ProgramMessage> &programMessage);
    virtual void handleMidiLog(const juce::MidiMessage &message, const juce::String &logPrefix);

    void requestFactoryReset();
    void requestProgramDump(int first, int last = -1);

protected:
    void setupMidiCCComponent(uint8_t midiCC, juce::Component *component);
    void removeMidiCCComponent(uint8_t midiCC, juce::Component *component);

    std::shared_ptr<SettingsMessage> &getCurrentSettings();
    void updateSettings(Pro800Settings setting, int value);

    std::shared_ptr<VersionMessage> &getCurrentVersion();

    virtual void setComponentValue(juce::Component *component, int identifier, int value, int maxValue = -1);
    
private:
    juce::Array<MessageType> registeredMessageTypes = juce::Array<MessageType>();
    juce::HashMap<uint8_t, juce::Array<juce::Component*>> registeredCCComponents;

    MidiHandler *midiHandler;

    std::shared_ptr<SettingsMessage> currentSettings = std::shared_ptr<SettingsMessage>();
    std::shared_ptr<VersionMessage> currentVersion = std::shared_ptr<VersionMessage>();
};