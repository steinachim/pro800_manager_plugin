#include "ProgramMessage.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

juce::MidiMessage ProgramMessage::request(int programNumber)
{
    uint8_t programLSB = (programNumber & 0x7F);
    uint8_t programMSB = (uint8_t)(programNumber >> 7);

    std::vector<uint8_t> request;
    request.insert(request.end(), std::begin(PRO800_HEADER), std::end(PRO800_HEADER));
    request.insert(request.end(), {REQUEST_ID, programLSB, programMSB});
    return juce::MidiMessage::createSysExMessage(request.data(), (int)request.size());
}

ProgramMessage::ProgramMessage(const juce::MidiMessage &message) : ProgramMessage(message.getRawData(), message.getRawDataSize())
{
}

ProgramMessage::ProgramMessage(const uint8_t *newRawData, int newRawDataSize) : Pro800MidiMessage(newRawData, newRawDataSize)
{
    // if this message is an older version (pre 111), then update version to 111 and reserve memory accordingly
    if ( newRawDataSize < MESSAGE_SIZE_VERSION_111 )
    {   
        // resize data array to new size     
        this->rawData->resize(MESSAGE_SIZE_VERSION_111, 0);

        // move 0xF7 from previous last position to new last position
        this->rawData->at(newRawDataSize-1) = 0x00;
        this->rawData->at(this->rawData->size()-1 ) = 0xF0;

        // update program version info
        this->setValue(PROGRAM_FIELD_VERSION, 111);
    }
}

bool ProgramMessage::isValid() const
{
    // TODO: add length check based on version
    return Pro800MidiMessage::isValid() 
        && getRawDataSize() > POS_MESSAGE_START;
}

uint16_t ProgramMessage::getProgramNumber() const
{
    uint8_t programLSB = getUint8Value(PROGRAM_NUM_LSB);
    uint8_t programMSB = getUint8Value(PROGRAM_NUM_MSB);
    return (uint16_t)((programMSB << 7) | programLSB);
}

std::string ProgramMessage::getProgramBankNumber() const
{
    uint16_t programNumber = getProgramNumber();
    uint8_t bank = (uint8_t)(programNumber / 100); // 0-3 = A-D
    uint8_t program = (uint8_t)(programNumber % 100); // 0-99

    char bankName = 'A' + bank;

    std::stringstream ss;
    ss << bankName;
    ss << std::setfill('0') << std::setw(2) << (int)program;
    return ss.str();
}

void ProgramMessage::setProgramNumber(uint16_t programNumber)
{
    uint8_t programLSB = programNumber & 0x7F;
    uint8_t programMSB = (programNumber >> 7) & 0x7F;
    this->setUint8Value(PROGRAM_NUM_LSB, programLSB);
    this->setUint8Value(PROGRAM_NUM_MSB, programMSB);
}

std::string ProgramMessage::getProgramName() const
{
    int lastCharPos = std::min((int)PROGRAM_NAME_LAST_CHAR, getRawDataSize() - POS_MESSAGE_START - 2); // last character cannot be the final 0xF7
    const char *stringStart = (char*)getRawData() + Pro800MidiMessage::POS_MESSAGE_START + PROGRAM_NAME_FIRST_CHAR;
    const size_t stringLength = lastCharPos - PROGRAM_NAME_FIRST_CHAR + 1;

    std::string value = std::string(stringStart, stringLength);

    // remove unused overflow characters
    const size_t unusedPos1 = PROGRAM_NAME_UNUSED_1 - PROGRAM_NAME_FIRST_CHAR;
    if ( unusedPos1 < value.length() )
    {
        value.erase(unusedPos1, 1);
    }

    const size_t unusedPos2 = PROGRAM_NAME_UNUSED_2 - PROGRAM_NAME_FIRST_CHAR - 1; // moved 1 to the left because of previous deletion
    if ( unusedPos2 < value.length() )
    {
        value.erase(unusedPos2, 1);
    }

    // remove potential trailing null values
    value.erase(std::remove(value.begin(), value.end(), 0x00), value.end());

    return value;
}



void ProgramMessage::setProgramName(const std::string &newName)
{
    /*
      - overflow bytes in the name must be filled with their correct values
      - name structure: XX XX XX XX 00 XX XX XX XX XX XX XX 00 XX XX XX XX XX
     */
    std::vector<unsigned char> formattedProgramName = std::vector<unsigned char>(newName.begin(), newName.end());
    formattedProgramName.resize(PROGRAM_NAME_LAST_CHAR-PROGRAM_NAME_FIRST_CHAR+1, 0x00);

    // 
    int overflowPos1 = PROGRAM_NAME_UNUSED_1 - PROGRAM_NAME_FIRST_CHAR;
    formattedProgramName.insert(formattedProgramName.begin() + overflowPos1, getUint8Value(PROGRAM_NAME_UNUSED_1));

    int overflowPos2 = PROGRAM_NAME_UNUSED_2 - PROGRAM_NAME_FIRST_CHAR;
    formattedProgramName.insert(formattedProgramName.begin() + overflowPos2, getUint8Value(PROGRAM_NAME_UNUSED_2));

    std::copy(formattedProgramName.begin(), formattedProgramName.end(), this->getRawData() + POS_MESSAGE_START + PROGRAM_NAME_FIRST_CHAR);
}

bool ProgramMessage::isLfoDestinationEnabled(Pro800ProgramLfoDestination destination) const
{
    const uint8_t lfoDestinations = (uint8_t)getValue(PROGRAM_FIELD_LFO_DEST);
    return lfoDestinations & destination;
}

void ProgramMessage::setLfoDestinationEnabled(Pro800ProgramLfoDestination destination, bool enabled)
{
    uint8_t lfoDestinations = (uint8_t)getValue(PROGRAM_FIELD_LFO_DEST);
    uint8_t targetValue = (enabled ? destination : 0);

    lfoDestinations = (lfoDestinations & ~destination) | targetValue;
    setValue(PROGRAM_FIELD_LFO_DEST, lfoDestinations);
}

juce::String ProgramMessage::toString() const
{
    std::stringstream ss;
    ss << "Pro800 Program Dump: "
       << getProgramBankNumber() << " - '" << getProgramName() << "'\n";

    for ( auto param : PRO800_PROGRAM_FIELDS )
    {
        int value = getValue(param.first);
        int maxValue = (1 << param.second.numBytes*8) - 1;

        ss << param.second.name << ": " << getValue(param.first) << " (display: " << value * 999 / maxValue << ")\n";
    }
       
    ss << "raw: " << juce::String::toHexString(getRawData(), getRawDataSize());
    return ss.str();
}

int ProgramMessage::getValue(Pro800ProgramField field) const
{
    if ( PRO800_PROGRAM_FIELDS.contains(field) )
    {
        Pro800Parameter param = PRO800_PROGRAM_FIELDS.at(field);
        return Pro800MidiMessage::getValue(param.firstByte, param.numBytes, param.isSigned);
    }
    else if ( field == PROGRAM_FIELD_NUM )
    {
        return (int)getProgramNumber();
    }
    else
    {
        std::cerr << "ProgramMessage::getValue(): No getter for field defined: " << field << std::endl;
    }

    return 0;
}

void ProgramMessage::setValue(Pro800ProgramField field, int value)
{
    if ( PRO800_PROGRAM_FIELDS.contains(field) )
    {
        Pro800Parameter param = PRO800_PROGRAM_FIELDS.at(field);
        Pro800MidiMessage::setValue(param.firstByte, param.numBytes, value);
    }
    else if ( field == PROGRAM_FIELD_NUM )
    {
        setProgramNumber((uint16_t)value);
    }
    else
    {
        std::cerr << "ProgramMessage::setValue(): No setter for field defined: " << field << std::endl;
    }    
}

unsigned char ProgramMessage::getResponseType() const
{
    return RESPONSE_ID;
}