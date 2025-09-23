#include "Pro800MessageFactory.h"


#include "ProgramMessage.h"
#include "SettingsMessage.h"
#include "VersionMessage.h"
#include "StatusMessage.h"

std::shared_ptr<Pro800MidiMessage>Pro800MessageFactory::createMidiMessage(const juce::MidiMessage &midiMessage)
{
    std::shared_ptr<Pro800MidiMessage> pro800Message(new Pro800MidiMessage(midiMessage));
    if ( !pro800Message->isValid() )
    {
        return std::shared_ptr<Pro800MidiMessage>();
    }

    unsigned char messageType = midiMessage.getRawData()[Pro800MidiMessage::POS_MESSAGE_TYPE];

    switch (messageType)
    {
    case SettingsMessage::RESPONSE_ID: {
        uint8_t addressLow = midiMessage.getRawData()[Pro800MidiMessage::POS_MESSAGE_TYPE + 1];
        uint8_t addressHigh = midiMessage.getRawData()[Pro800MidiMessage::POS_MESSAGE_TYPE + 2];

        // note: SettingsMessage is a program message with specific format
        if ( addressLow == SettingsMessage::ADDRESS_LOW && addressHigh == SettingsMessage::ADDRESS_HIGH )
        {
            return std::shared_ptr<Pro800MidiMessage>(new SettingsMessage(midiMessage));
        }
        else
        {       
            return std::shared_ptr<ProgramMessage>(new ProgramMessage(midiMessage));
        }
    }

    case VersionMessage::RESPONSE_ID:
        return std::shared_ptr<VersionMessage>(new VersionMessage(midiMessage));

    case StatusMessage::RESPONSE_ID:
        return std::shared_ptr<StatusMessage>(new StatusMessage(midiMessage));

    default:
        return pro800Message;
    }
}

