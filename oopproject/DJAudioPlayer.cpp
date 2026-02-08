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

    // Init filters
    for (int i=0; i<2; ++i) {
        lowFilters[i].reset();
        midFilters[i].reset();
        highFilters[i].reset();
    }
    setEQ(eqGainHigh, eqGainMid, eqGainLow);
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

    // Get samples
    resampleSource.getNextAudioBlock(bufferToFill);

    // Apply EQ
    int channels = jmin(bufferToFill.buffer->getNumChannels(), 2);
    for (int ch = 0; ch < channels; ++ch)
    {
        float* data = bufferToFill.buffer->getWritePointer(ch, bufferToFill.startSample);
        lowFilters[ch].processSamples(data, bufferToFill.numSamples);
        midFilters[ch].processSamples(data, bufferToFill.numSamples);
        highFilters[ch].processSamples(data, bufferToFill.numSamples);
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

        // Try to parse BPM from metadata
        String bpmStr = reader->metadataValues["bpm"];
        if (bpmStr.isEmpty()) bpmStr = reader->metadataValues["tempo"];
        if (bpmStr.isEmpty() && reader->metadataValues.containsKey("id3bpm")) bpmStr = reader->metadataValues["id3bpm"];

        if (bpmStr.isNotEmpty())
        {
            baseBPM = bpmStr.getDoubleValue();
            if (baseBPM <= 0) baseBPM = 120.0; // Fallback
        }
        else
        {
            baseBPM = 120.0; // Default
        }

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
    channelGain = gain;
    transportSource.setGain(channelGain * crossfadeGain);
}

void DJAudioPlayer::setCrossfadeFactor(double factor)
{
    crossfadeGain = factor;
    transportSource.setGain(channelGain * crossfadeGain);
}

void DJAudioPlayer::setSpeed(double ratio)
{
    currentSpeedRatio = ratio;
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

void DJAudioPlayer::setEQ(double high, double mid, double low)
{
    eqGainHigh = high;
    eqGainMid = mid;
    eqGainLow = low;

    if (currentSampleRate > 0)
    {
        auto lowCoeffs = IIRCoefficients::makeLowShelf(currentSampleRate, 250.0, 1.0, low);
        auto midCoeffs = IIRCoefficients::makePeakFilter(currentSampleRate, 1000.0, 1.0, mid);
        auto highCoeffs = IIRCoefficients::makeHighShelf(currentSampleRate, 4000.0, 1.0, high);

        for (int i=0; i<2; ++i) {
            lowFilters[i].setCoefficients(lowCoeffs);
            midFilters[i].setCoefficients(midCoeffs);
            highFilters[i].setCoefficients(highCoeffs);
        }
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

double DJAudioPlayer::getBPM()
{
    return baseBPM * currentSpeedRatio;
}

void DJAudioPlayer::setBaseBPM(double bpm)
{
    if (bpm > 0) baseBPM = bpm;
}
