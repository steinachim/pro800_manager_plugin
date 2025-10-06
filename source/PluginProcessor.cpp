/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "midi/MidiCallbackMessage.h"

//==============================================================================
Pro800ManagerAudioProcessor::Pro800ManagerAudioProcessor() : AudioProcessor(BusesProperties())
{
    this->midiHandler = new MidiHandler(this);
}

Pro800ManagerAudioProcessor::~Pro800ManagerAudioProcessor()
{
    delete this->midiHandler;
}

//==============================================================================
const juce::String Pro800ManagerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Pro800ManagerAudioProcessor::acceptsMidi() const
{
    return true;
}

bool Pro800ManagerAudioProcessor::producesMidi() const
{
    return true;
}

bool Pro800ManagerAudioProcessor::isMidiEffect() const
{
    return true;
}

double Pro800ManagerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Pro800ManagerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Pro800ManagerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Pro800ManagerAudioProcessor::setCurrentProgram (int /*index*/)
{
}

const juce::String Pro800ManagerAudioProcessor::getProgramName (int /*index*/)
{
    return {};
}

void Pro800ManagerAudioProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/)
{
}

//==============================================================================
void Pro800ManagerAudioProcessor::prepareToPlay (double /*sampleRate*/, int /*samplesPerBlock*/)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void Pro800ManagerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool Pro800ManagerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    juce::ignoreUnused (layouts);
    return true;
}

void Pro800ManagerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        //const auto time = metadata.samplePosition;

        (new MidiCallbackMessage(this->midiHandler, message))->post();
    }
    midiMessages.clear();
    
    if ( !this->sendBuffer.isEmpty() )
    {
        midiMessages.swapWith(this->sendBuffer);
        this->sendBuffer.clear();
    }
}

void Pro800ManagerAudioProcessor::sendMidiMessage(const juce::MidiMessage& message)
{
    this->sendBuffer.addEvent(message, 0);
}   

//==============================================================================
bool Pro800ManagerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* Pro800ManagerAudioProcessor::createEditor()
{
    return new Pro800ManagerEditor (midiHandler, *this);
}

//==============================================================================
void Pro800ManagerAudioProcessor::getStateInformation (juce::MemoryBlock& /*destData*/)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Pro800ManagerAudioProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Pro800ManagerAudioProcessor();
}

