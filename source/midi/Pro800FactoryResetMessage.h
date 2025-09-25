

#include "StatusMessage.h"

class Pro800FactoryResetMessage : public Pro800MidiMessage
{
public:
    static const unsigned char REQUEST_ID = 0x7D;
    static const unsigned char RESPONSE_ID = StatusMessage::RESPONSE_ID;

    static juce::MidiMessage request()
    {
        std::vector<uint8_t> request;
        request.insert(request.end(), std::begin(PRO800_HEADER), std::end(PRO800_HEADER));
        request.insert(request.end(), {REQUEST_ID, 0x00});
        return juce::MidiMessage::createSysExMessage(request.data(), (int)request.size());
    }

private:
    Pro800FactoryResetMessage(const juce::MidiMessage &message) : Pro800MidiMessage(message)
    {        
        // is request only, no dedicated response
    }
};