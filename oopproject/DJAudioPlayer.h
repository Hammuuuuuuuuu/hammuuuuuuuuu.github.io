/*
  ==============================================================================

    DJAudioPlayer.h
    Created: 13 Mar 2020 4:22:22pm
    Author:  matthew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class DJAudioPlayer : public AudioSource {
  public:

    DJAudioPlayer(AudioFormatManager& _formatManager);
    ~DJAudioPlayer();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void loadURL(URL audioURL);
    void setGain(double gain);
    void setSpeed(double ratio);
    void setPosition(double posInSecs);
    void setPositionRelative(double pos);


    void start();
    void stop();
    bool isPlaying();

    /** get the relative position of the playhead */
    double getPositionRelative();

    /** set the filter (low pass) cutoff */
    void setFilter(double cutoff);

    /** set EQ gains (0.1 to 4.0 typically, 1.0 is flat) */
    void setLow(double gain);
    void setMid(double gain);
    void setHigh(double gain);

    /** toggle looping */
    void setLooping(bool shouldLoop);
    bool isLooping();

    /** Hot Cues */
    void setCue(int index);
    void jumpToCue(int index);
    bool hasCue(int index);
    void clearCue(int index);

    /** Beat Loop: Set loop range starting now with duration */
    void setBeatLoop(double durationSeconds);

private:
    AudioFormatManager& formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    ResamplingAudioSource resampleSource{&transportSource, false, 2};

    // EQ Chain using IIRFilter directly (Stereo)
    IIRFilter lowFilters[2];
    IIRFilter midFilters[2];
    IIRFilter highFilters[2];

    double currentSampleRate = 0;
    double currentFilterCutoff = 1.0;

    bool isLoopingState = false;

    double hotCues[3] = {-1.0, -1.0, -1.0};

    // Manual Looping
    bool isBeatLooping = false;
    double loopStart = 0.0;
    double loopDuration = 0.0;
};
