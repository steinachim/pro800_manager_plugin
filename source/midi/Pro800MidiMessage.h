#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../Pro800Constants.h"

/**
 * Known request types:
 * 
 * 02 - Unknown (responds with 03 00)
 * 03 - Unknown (responds with Status OK)
 * 04 - Unknown (responds with 05 50 30 45 39 49 00)
 * 06 - Request device name (responds with 07 50 52 4f 2d 38 30 30 00 - string: PRO-800)
 * 08 00 - Request Version info (responds with VersionMessage)
 * 0E - Unknown (responds with Status OK)
 * 0F [00-18] - Unknown (responds with Status OK in that range, Status Error otherwise)
 * 11 [00-1F] - Unknown (responds with Status OK in that range, Status Error otherwise)
 * 12 [00-02] - Unknown (responds with Status OK in that range, Status Error otherwise)
 * 13 [00-07] - Unknown (responds with Status OK in that range, Status Error otherwise)
 * 17 [00-01] - Unknown (responds with Status OK in that range, Status Error otherwise)
 * 19 [00-02] - Unknown (responds with Status OK in that range, Status Error otherwise)
 * 1A [00-04] - Unknown (responds with Status OK in that range, Status Error otherwise)
 * 1B [00-03] - Unknown (responds with Status OK in that range, Status Error otherwise)
 * 1C [00-05] - Unknown (responds with Status OK in that range, Status Error otherwise)
 * 1D - Unknown (responds with Status OK)
 * 32 [00-7F] - parameter 00: Reset default mode and to saved preset settings (?)
 *            - any other parameter: display 8888, then keeps dots enabled in display and responds strangely to controls. MIDI still seems to work. Some debug mode maybe?
 *            - always responds with Status OK
 * 35 - Unknown (responds with Status OK)
 * 36 [00-7F] - parameter 00: appears to lock synth
 *            - any other parameter: unlocks it again
 *            - always responds with Status OK
 * 37 [00-7F] - parameter 00: Turn off all button lights
 *            - any other parameter: Lights up all buttons (
 *            - always responds with Status OK
 * 50 [00-7F] [00-01] -  (responds with Status OK in that range, Status Error otherwise)
 * 71 [00-17] - "push" buttons:
 *            - 00-09 = numpad
 *            - 0A/0B = ??? - disable performace / settings if active
 *            - 0C = preset
 *            - 0D = rec
 *            - 0e = perf
 *            - 0f = settings
 *            - 10 = seq 1
 *            - 11 = seq 2
 *            - 12 = ?? (in performance mode, disable perf mode; in settings mode, go to Tune)
 *            - 13 = sync clock
 *            - 14 = sync source
 *            - 15 = ??
 *            - 16 = Next Setting (value wheel?)
 *            - 17 - Prev Setting (value wheel?)
 * 72 [00-19] - Unknown (get value of param 1 - responds with 73 [param 1] [00-ff])
 *            - Example values (param 1, param2)
 *              00, 3e (62)
 *              01, 42 (66)
 *              02, 50 (80)
 *              03, 42 (66)
 *              04, 3e (62)
 *              05, 40 (64)
 *              06, 6c (108)
 *              07, 39 (57)
 *              08, 7f (127)
 *              09, 7f (127)
 *              0a, 26 (38)
 *              0b, 45 (117)
 *              0c, 5d (93)
 *              0d, 41 (65)
 *              0e, 50 (80)
 *              0f, 32 (50)
 *              10, 58 (88)
 *              11, 3a (58)
 *              12, 37 (55)
 *              13, 3f (63)
 *              14, 68 (104)
 *              15, 00 (0)
 *              16, 7e (126)
 *              17, 51 (81)
 *              18, 53 (83)
 *              19, 7f (127)
 * 73 [00-19] [00-7F] - Unknown (set value of param 1, responds with status OK if values in range)
 * 77 [00-7F] [00-03] - Request program dump (param1 = program lsb, param2 = program msb) - response with 73 (see ProgramMessage) - note: 7e 03 = SettingsMessage
 * 78 [00-7F] [00-03] [...] - Set program dump (param1 = program lsb, param2 = program msb), following bytes = program data (may be empty -> set to "uninitialized")
 * 7D - Factory Reset (no confirmation!)
 * 
 */

class Pro800MidiMessage
{
public:
    const uint8_t POS_OFFSET = 10; // TODO: cleanup and remove

    const static inline std::vector<uint8_t> PRO800_HEADER = {
        0x00, 0x20, 0x32,      // Brand ID (Behringer)
        0x00, 0x01, 0x24,      // Product ID (Pro-800)
        0x00                   // CPU ID
    };

    static const int POS_MESSAGE_TYPE = 0x08;
    static const int POS_MESSAGE_START = 0x09;

    static const uint8_t RESPONSE_UNINIT = 0xFF;

    Pro800MidiMessage(const juce::MidiMessage &message);
    Pro800MidiMessage(const uint8_t *newRawData, int newRawDataSize);
    virtual ~Pro800MidiMessage();

    virtual MessageType getMessageType() const { return MessageType::PRO800_UNKNOWN_MESSAGE;}

    virtual juce::String toString() const;
    std::shared_ptr<juce::MidiMessage> toMidiMessage() const;

    virtual bool isValid() const;

    std::shared_ptr<std::vector<uint8_t>> &getRawData();
    size_t getRawDataSize() const;

protected:
    uint8_t getUint8Value(size_t position) const;
    void setUint8Value(size_t position, uint8_t value);

    int getValue(int firstByte, int numBytes, bool isSigned) const;
    void setValue(int firstByte, int numBytes, int value);

    std::string getStringValue(int firstByte, int lastByte) const;
    void setStringValue(int firstByte, int lastByte, const std::string &newValue);

    virtual unsigned char getResponseType() const;

private:
    bool isCorrectResponse() const;

    std::shared_ptr<std::vector<uint8_t>> rawData;
};
