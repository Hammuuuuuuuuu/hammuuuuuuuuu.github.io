/*
  ==============================================================================

    MainComponent.cpp
    Created: 21 Jan 2025
    Author:  Jules

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (1024, 768);

    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { if (granted)  setAudioChannels (2, 2); });
    }
    else
    {
        setAudioChannels (0, 2);
    }

    // Region A: Visualisation
    addAndMakeVisible(waveform1);
    addAndMakeVisible(waveform2);

    waveform1.setColour(Colours::cyan);
    waveform2.setColour(Colours::orange);

    // Region B: Mixing Console
    addAndMakeVisible(deckGUI1);
    addAndMakeVisible(mixer);
    addAndMakeVisible(deckGUI2);

    // Region C: Library
    addAndMakeVisible(playlistComponent);

    formatManager.registerBasicFormats();

    deckGUI1.addListener(this);
    deckGUI2.addListener(this);

    startTimer(50);

    // Connect waveform interactions
    waveform1.onPositionChanged = [this](double pos) { player1.setPositionRelative(pos); };
    waveform2.onPositionChanged = [this](double pos) { player2.setPositionRelative(pos); };
}

MainComponent::~MainComponent()
{
    shutdownAudio();
    stopTimer();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);

    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);

 }
void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    mixerSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    player1.releaseResources();
    player2.releaseResources();
    mixerSource.releaseResources();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    double w = getWidth();
    double h = getHeight();

    // A. Top Region (20%)
    double topH = h * 0.2;
    waveform1.setBounds(0, 0, w/2, topH);
    waveform2.setBounds(w/2, 0, w/2, topH);

    // B. Middle Region (40%)
    double midH = h * 0.4;
    double midY = topH;

    // Layout: Deck 1 | Mixer | Deck 2
    double deckW = w * 0.35;
    double mixerW = w * 0.30;

    deckGUI1.setBounds(0, midY, deckW, midH);
    mixer.setBounds(deckW, midY, mixerW, midH);
    deckGUI2.setBounds(deckW + mixerW, midY, deckW, midH);

    // C. Bottom Region (40%)
    double botH = h * 0.4;
    double botY = midY + midH;
    playlistComponent.setBounds(0, botY, w, botH);
}

void MainComponent::fileLoaded(DeckGUI* deck, URL audioURL)
{
    if (deck == &deckGUI1)
    {
        waveform1.loadURL(audioURL);
    }
    if (deck == &deckGUI2)
    {
        waveform2.loadURL(audioURL);
    }
}

void MainComponent::timerCallback()
{
    waveform1.setPositionRelative(player1.getPositionRelative());
    waveform2.setPositionRelative(player2.getPositionRelative());
}
