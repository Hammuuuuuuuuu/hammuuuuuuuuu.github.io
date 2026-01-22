/*
==============================================================================

DJAudioPlayer.cpp
Created: 13 Mar 2020 4:22:22pm
Author:  matthew

==============================================================================
*/

#include "DJAudioPlayer.h"

DJAudioPlayer::DJAudioPlayer(AudioFormatManager& _formatManager)
: formatManager(_formatManager)
{

}
DJAudioPlayer::~DJAudioPlayer()
{

}

void DJAudioPlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

    for (int i=0; i<2; ++i)
    {
        lowFilters[i].reset();
        midFilters[i].reset();
        highFilters[i].reset();
    }

    // Initialize EQ to flat
    setLow(1.0);
    setMid(1.0);
    setHigh(1.0);
}
void DJAudioPlayer::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if (isBeatLooping)
    {
        double currentPos = transportSource.getCurrentPosition();
        if (currentPos >= loopStart + loopDuration)
        {
            transportSource.setPosition(loopStart);
        }
    }

    // Get audio from source
    resampleSource.getNextAudioBlock(bufferToFill);

    // Process with EQ filters
    // Ensure we don't process more channels than we have filters for (Stereo = 2)
    int channelsToProcess = jmin(bufferToFill.buffer->getNumChannels(), 2);

    for (int channel = 0; channel < channelsToProcess; ++channel)
    {
        float* channelData = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

        lowFilters[channel].processSamples(channelData, bufferToFill.numSamples);
        midFilters[channel].processSamples(channelData, bufferToFill.numSamples);
        highFilters[channel].processSamples(channelData, bufferToFill.numSamples);
    }
}
void DJAudioPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
}

void DJAudioPlayer::loadURL(URL audioURL)
{
    auto* reader = formatManager.createReaderFor(audioURL.createInputStream(false));
    if (reader != nullptr) // good file!
    {
        std::unique_ptr<AudioFormatReaderSource> newSource (new AudioFormatReaderSource (reader,
true));
        // Important: Preserve loop state if already set, or re-apply it.
        if (isLoopingState) newSource->setLooping(true);

        transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset (newSource.release());

        // Reset Cues on load
        for(int i=0; i<3; ++i) hotCues[i] = -1.0;
    }
}
void DJAudioPlayer::setGain(double gain)
{
    if (gain < 0 || gain > 1.0)
    {
        std::cout << "DJAudioPlayer::setGain gain should be between 0 and 1" << std::endl;
    }
    else {
        transportSource.setGain(gain);
    }

}
void DJAudioPlayer::setSpeed(double ratio)
{
  if (ratio < 0 || ratio > 100.0)
    {
        std::cout << "DJAudioPlayer::setSpeed ratio should be between 0 and 100" << std::endl;
    }
    else {
        resampleSource.setResamplingRatio(ratio);
    }
}
void DJAudioPlayer::setPosition(double posInSecs)
{
    transportSource.setPosition(posInSecs);
}

void DJAudioPlayer::setPositionRelative(double pos)
{
     if (pos < 0 || pos > 1.0)
    {
        std::cout << "DJAudioPlayer::setPositionRelative pos should be between 0 and 1" << std::endl;
    }
    else {
        double posInSecs = transportSource.getLengthInSeconds() * pos;
        setPosition(posInSecs);
    }
}


void DJAudioPlayer::start()
{
    transportSource.start();
}
void DJAudioPlayer::stop()
{
  transportSource.stop();
}

bool DJAudioPlayer::isPlaying()
{
    return transportSource.isPlaying();
}

double DJAudioPlayer::getPositionRelative()
{
    if (transportSource.getLengthInSeconds() > 0)
        return transportSource.getCurrentPosition() / transportSource.getLengthInSeconds();
    return 0;
}

void DJAudioPlayer::setFilter(double cutoff)
{
    // Deprecated in favor of EQ
}

void DJAudioPlayer::setLow(double gain)
{
    if (currentSampleRate > 0)
    {
        auto coeffs = IIRCoefficients::makeLowShelf(currentSampleRate, 250.0, 1.0, gain);
        lowFilters[0].setCoefficients(coeffs);
        lowFilters[1].setCoefficients(coeffs);
    }
}

void DJAudioPlayer::setMid(double gain)
{
    if (currentSampleRate > 0)
    {
        auto coeffs = IIRCoefficients::makePeakFilter(currentSampleRate, 1000.0, 1.0, gain);
        midFilters[0].setCoefficients(coeffs);
        midFilters[1].setCoefficients(coeffs);
    }
}

void DJAudioPlayer::setHigh(double gain)
{
    if (currentSampleRate > 0)
    {
        auto coeffs = IIRCoefficients::makeHighShelf(currentSampleRate, 4000.0, 1.0, gain);
        highFilters[0].setCoefficients(coeffs);
        highFilters[1].setCoefficients(coeffs);
    }
}

void DJAudioPlayer::setLooping(bool shouldLoop)
{
    isLoopingState = shouldLoop;
    if (readerSource)
    {
        readerSource->setLooping(shouldLoop);
    }
}

bool DJAudioPlayer::isLooping()
{
    return isLoopingState;
}

void DJAudioPlayer::setCue(int index)
{
    if (index >= 0 && index < 3)
    {
        hotCues[index] = transportSource.getCurrentPosition();
    }
}

void DJAudioPlayer::jumpToCue(int index)
{
    if (index >= 0 && index < 3 && hotCues[index] >= 0)
    {
        transportSource.setPosition(hotCues[index]);
    }
}

bool DJAudioPlayer::hasCue(int index)
{
    return (index >= 0 && index < 3 && hotCues[index] >= 0);
}

void DJAudioPlayer::clearCue(int index)
{
    if (index >= 0 && index < 3)
    {
        hotCues[index] = -1.0;
    }
}

void DJAudioPlayer::setBeatLoop(double durationSeconds)
{
    if (durationSeconds <= 0)
    {
        isBeatLooping = false;
        return;
    }

    loopStart = transportSource.getCurrentPosition();
    loopDuration = durationSeconds;
    isBeatLooping = true;

    std::cout << "Looping: " << durationSeconds << "s" << std::endl;
}
