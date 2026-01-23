/*
  ==============================================================================

    MainComponent.h
    Created: 21 Jan 2025
    Author:  Jules

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include "MixerComponent.h"
#include "PlaylistComponent.h"
#include "WaveformDisplay.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public AudioAppComponent,
                        public DeckGUI::Listener,
                        public Timer,
                        public DragAndDropContainer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

    // DeckGUI::Listener
    void fileLoaded(DeckGUI* deck, URL audioURL) override;

    // Timer
    void timerCallback() override;

private:
    //==============================================================================
    AudioFormatManager formatManager;
    AudioThumbnailCache thumbCache{100};

    DJAudioPlayer player1{formatManager};
    DJAudioPlayer player2{formatManager};

    MixerAudioSource mixerSource;

    // Components
    WaveformDisplay waveform1{formatManager, thumbCache};
    WaveformDisplay waveform2{formatManager, thumbCache};

    DeckGUI deckGUI1{&player1, formatManager, thumbCache};
    DeckGUI deckGUI2{&player2, formatManager, thumbCache};

    MixerComponent mixer{&player1, &player2};

    PlaylistComponent playlistComponent{formatManager};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
