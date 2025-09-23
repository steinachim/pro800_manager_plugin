#include "ProgramMessage.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

juce::MidiMessage ProgramMessage::request(int programNumber)
{
    unsigned char programLSB = (programNumber & 0x7F);
    unsigned char programMSB = (programNumber >> 7);

    std::vector<uint8_t> request;
    request.insert(request.end(), std::begin(PRO800_HEADER), std::end(PRO800_HEADER));
    request.insert(request.end(), {REQUEST_ID, programLSB, programMSB});
    return juce::MidiMessage::createSysExMessage(request.data(), (int)request.size());
}

bool ProgramMessage::isValid() const
{
    return Pro800MidiMessage::isValid() && getRawDataSize() >= DUMP_POS_PROGRAM_MSB;
}


ProgramMessage::ProgramMessage(const juce::MidiMessage &message) : Pro800MidiMessage(message)
{
    extractDumpData();
}

short ProgramMessage::getProgramNumber() const
{
    return this->programNumber;
}

void ProgramMessage::setProgramNumber(short programNumber)
{
    this->programNumber = programNumber;

    uint8_t programLSB = programNumber & 0x7F;
    uint8_t programMSB = (programNumber >> 7) & 0x7F;

    this->getRawData()[DUMP_POS_PROGRAM_LSB] = programLSB;
    this->getRawData()[DUMP_POS_PROGRAM_MSB] = programMSB;
}

std::string ProgramMessage::getProgramName() const
{
    return this->programName;
}

std::string ProgramMessage::getProgramBankNumber() const
{
    if (this->programNumber < 0)
    {
        return "XXX";
    }

    int bank = this->programNumber / 100;
    int program = this->programNumber % 100;

    char bankName = 'A' + bank;

    std::stringstream ss;
    ss << bankName;
    ss << std::setfill('0') << std::setw(2) << program;
    return ss.str();
}

void ProgramMessage::setProgramName(const std::string &newName)
{
    /*
      Apparent internal format:
      - needs 0 after first 4 letters (display size-related)
      - needs 0 after further 7 letters..
      - max name length 16 (+ fillter 0s)
      - name structure: XX XX XX XX 00 XX XX XX XX XX XX XX 00 XX XX XX XX XX
     */
    std::vector<unsigned char> formattedProgramName = std::vector<unsigned char>(newName.begin(), newName.end());
    formattedProgramName.resize(MAX_PROGRAM_NAME_LENGTH, 0x00);

    formattedProgramName.insert(formattedProgramName.begin() + 4, 0x00);
    formattedProgramName.insert(formattedProgramName.begin() + 12, 0x00); // account for first inserted 0
    formattedProgramName.push_back(0x00);                                 // add trailing NULL

    std::copy(formattedProgramName.begin(), formattedProgramName.end(), this->getRawData() + DUMP_POS_PROGRAM_NAME_START);

    // set program name from the actual message data
    this->programName = getValueString(DUMP_POS_PROGRAM_NAME_START, DUMP_POS_PROGRAM_NAME_END);
}

juce::String ProgramMessage::toString() const
{
    std::stringstream ss;
    ss << "Pro800 Program Dump: ";
    ss << getProgramBankNumber() << " - " << getProgramName();
    return ss.str();
}

uint32_t ProgramMessage::getValue(RawDumpPosition position, size_t width)
{
    if (!isValid())
    {
        cerr << "Trying to read value of invalid dump report" << endl;
        return 0;
    }

    switch (width)
    {
    case 1:
        return getValue8(position);

    case 2:
        return getValue16(position);

    case 4:
        return getValue32(position);

    default:
        cerr << "Unsupported field width requested - returning 0" << endl;
    }

    return 0;
}

uint8_t ProgramMessage::getValue8(RawDumpPosition position)
{
    return this->getRawData()[position];
}

uint16_t ProgramMessage::getValue16(RawDumpPosition position)
{
    uint16_t msb = this->getRawData()[position];
    uint16_t lsb = this->getRawData()[position + 1];

    return (msb << 8) | lsb;
}

uint32_t ProgramMessage::getValue32(RawDumpPosition position)
{
    uint32_t byte1 = this->getRawData()[position];
    uint32_t byte2 = this->getRawData()[position + 1];
    uint32_t byte3 = this->getRawData()[position + 2];
    uint32_t byte4 = this->getRawData()[position + 3];

    return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
}

std::string ProgramMessage::getValueString(RawDumpPosition start, RawDumpPosition end)
{

    std::string value = std::string(getRawData() + start, getRawData() + end);

    // remove 0x00
    value.erase(std::remove(value.begin(), value.end(), 0x00), value.end());

    return value;
}

void ProgramMessage::extractDumpData()
{
    if (!isValid())
    {
        return;
    }

    // note program LSB is 7-byte value only
    short programLSB = getRawData()[DUMP_POS_PROGRAM_LSB];
    short programMSB = getRawData()[DUMP_POS_PROGRAM_MSB];
    this->programNumber = (programMSB << 7) | programLSB;

    if (this->programNumber == 0x1FE) // global dump
    {
        this->programName = "GLOBAL";
        return;
    }

    if (getRawDataSize() < DUMP_POS_PROGRAM_NAME_START)
    {
        cout << "Dump data truncated or not program dump. No extraction.\n";
        this->programName = "INVALID";
        this->programNumber = -1;
        return;
    }

    this->programName = getValueString(DUMP_POS_PROGRAM_NAME_START, DUMP_POS_PROGRAM_NAME_END);

    //setValid(true);
}

unsigned char ProgramMessage::getResponseType() const
{
    return RESPONSE_ID;
}
