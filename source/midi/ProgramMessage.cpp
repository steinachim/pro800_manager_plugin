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

ProgramMessage::ProgramMessage(const juce::MidiMessage &message) : Pro800MidiMessage(message)
{
}

bool ProgramMessage::isValid() const
{
    // TODO: add length check based on version
    return Pro800MidiMessage::isValid() 
        && getRawDataSize() > POS_MESSAGE_START + PROGRAM_VERSION;
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
    return getValueString(PROGRAM_NAME_FIRST_CHAR, PROGRAM_NAME_LAST_CHAR);
}



void ProgramMessage::setProgramName(const std::string &newName)
{
    /*
      Apparent internal format:
      - needs 0 after first 4 letters (display size-related)
      - needs 0 after further 7 letters..
      - max name length 15 (+ fillter 0s)
      - name structure: XX XX XX XX 00 XX XX XX XX XX XX XX 00 XX XX XX XX 00
     */
    std::vector<unsigned char> formattedProgramName = std::vector<unsigned char>(newName.begin(), newName.end());
    formattedProgramName.resize(MAX_PROGRAM_NAME_LENGTH, 0x00);

    formattedProgramName.insert(formattedProgramName.begin() + 4, 0x00);
    formattedProgramName.insert(formattedProgramName.begin() + 12, 0x00); // account for first inserted 0
    formattedProgramName.push_back(0x00);                                 // add trailing NULL

    std::copy(formattedProgramName.begin(), formattedProgramName.end(), this->getRawData() + POS_MESSAGE_START + PROGRAM_NAME_FIRST_CHAR);
}

juce::String ProgramMessage::toString() const
{
    std::stringstream ss;
    ss << "Pro800 Program Dump: "
       << getProgramBankNumber() << " - " << getProgramName() << "\n"
       << "Version:      " << getValue(PROGRAM_FIELD_VERSION) << "\n";

    for ( auto param : PRO800_PROGRAM_FIELDS )
    {
        ss << param.second.name << ": " << getValue(param.first) << "\n";
    }
       
    ss << "raw: " << juce::String::toHexString(getRawData(), getRawDataSize());
    return ss.str();
}

int ProgramMessage::getValue(Pro800ProgramField field) const
{
    if ( PRO800_PROGRAM_FIELDS.contains(field) )
    {
        Parameter16Bit parameterBlock = PRO800_PROGRAM_FIELDS.at(field);
        return (int) getUint16Value(parameterBlock);
    }
    switch(field)
    {
        case PROGRAM_FIELD_NUM:
            return (int)getProgramNumber();
        
        case PROGRAM_FIELD_VERSION:
            return (int)getUint8Value(PROGRAM_VERSION);

        case PROGRAM_FIELD_NAME:
        default:
            std::cerr << "ProgramMessage::getValue(): No getter for field defined: " << field << std::endl;
            return 0;
    }
}

void ProgramMessage::setValue(Pro800ProgramField field, int value)
{
    if ( PRO800_PROGRAM_FIELDS.contains(field) )
    {
        Parameter16Bit parameterBlock = PRO800_PROGRAM_FIELDS.at(field);
        setUint16Value(parameterBlock, (uint16_t)value);
    }

    switch(field)
    {
        case PROGRAM_FIELD_NUM:
            setProgramNumber((uint16_t)value);
            break;
        
        case PROGRAM_FIELD_VERSION:
            setUint8Value(PROGRAM_VERSION, (uint8_t)value);
            break;

        case PROGRAM_FIELD_NAME:
        default:
            std::cerr << "ProgramMessage::setValue(): No setter for field defined: " << field << std::endl;
    }

}


std::string ProgramMessage::getValueString(uint16_t firstCharPos, uint16_t lastCharPos) const
{
    lastCharPos = (uint16_t)std::min((int)(lastCharPos), getRawDataSize() - POS_MESSAGE_START - 2); // last character cannot be the final 0xF7
    const char *stringStart = (char*)getRawData() + Pro800MidiMessage::POS_MESSAGE_START + firstCharPos;
    const size_t stringLength = lastCharPos - firstCharPos + 1;

    std::string value = std::string(stringStart, stringLength);

    // remove 0x00
    value.erase(std::remove(value.begin(), value.end(), 0x00), value.end());

    return value;
}

unsigned char ProgramMessage::getResponseType() const
{
    return RESPONSE_ID;
}
