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
        transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset (newSource.release());
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
