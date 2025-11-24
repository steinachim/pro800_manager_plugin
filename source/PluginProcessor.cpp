/** 
 * Pro800 Manager Plugin
 * Copyright (C) 2025 Achim Stein
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "midi/MidiCallbackMessage.h"

Pro800ManagerAudioProcessor::Pro800ManagerAudioProcessor() 
     : AudioProcessor(BusesProperties()
                      // workaround: not really used, but added to allow plugin in Audio FX section in Logic
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), false)
                     )
{
    this->midiHandler = new MidiHandler(this);
}

Pro800ManagerAudioProcessor::~Pro800ManagerAudioProcessor()
{
    delete this->midiHandler;
}

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
    return 1;
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

void Pro800ManagerAudioProcessor::prepareToPlay (double /*sampleRate*/, int /*samplesPerBlock*/)
{
}

void Pro800ManagerAudioProcessor::releaseResources()
{
}

bool Pro800ManagerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    juce::ignoreUnused (layouts);
    return true;
}

void Pro800ManagerAudioProcessor::processBlock (juce::AudioBuffer<float>& /*buffer*/, juce::MidiBuffer& /*midiMessages*/)
{
}

bool Pro800ManagerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* Pro800ManagerAudioProcessor::createEditor()
{
    return new Pro800ManagerEditor (midiHandler, *this);
}


void Pro800ManagerAudioProcessor::getStateInformation (juce::MemoryBlock& /*destData*/)
{
}

void Pro800ManagerAudioProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Pro800ManagerAudioProcessor();
}

