/*
  ==============================================================================

    DeckGUI.cpp
    Created: 13 Mar 2020 6:44:48pm
    Author:  matthew

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "DeckGUI.h"

//==============================================================================
DeckGUI::DeckGUI(DJAudioPlayer* _player,
                AudioFormatManager & 	formatManagerToUse,
                AudioThumbnailCache & 	cacheToUse
           ) : player(_player),
               waveformDisplay(formatManagerToUse, cacheToUse),
               turntableComponent(_player)
{

    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(loopButton);

    addAndMakeVisible(volSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(posSlider);
    addAndMakeVisible(filterSlider);

    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(turntableComponent);


    playButton.addListener(this);
    stopButton.addListener(this);
    loadButton.addListener(this);
    loopButton.addListener(this);

    volSlider.addListener(this);
    speedSlider.addListener(this);
    posSlider.addListener(this);
    filterSlider.addListener(this);


    volSlider.setRange(0.0, 1.0);
    volSlider.setValue(1.0);

    speedSlider.setRange(0.0, 10.0); // Assuming 1.0 is normal speed?
    // Original code had 0-100.
    // Let's check MainComponent or DJAudioPlayer.
    // DJAudioPlayer: setSpeed calls resampleSource.setResamplingRatio.
    // 1.0 is normal speed. 100.0 is crazy fast.
    speedSlider.setRange(0.1, 5.0);
    speedSlider.setValue(1.0);

    posSlider.setRange(0.0, 1.0);

    filterSlider.setRange(0.0, 1.0);
    filterSlider.setValue(1.0); // Open

    // Customizing styles
    volSlider.setSliderStyle(Slider::LinearBarVertical);
    volSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    speedSlider.setSliderStyle(Slider::Rotary);
    speedSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    filterSlider.setSliderStyle(Slider::Rotary);
    filterSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    startTimer(500);


}

DeckGUI::~DeckGUI()
{
    stopTimer();
}

void DeckGUI::paint (Graphics& g)
{
    g.fillAll (Colours::black);   // clear the background

    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (Colours::cyan);
    g.setFont (14.0f);
    // Draw labels manually since we hid text boxes or just to be fancy
    // g.drawText ("Deck", getLocalBounds().removeFromTop(20), Justification::centred, true);
}

void DeckGUI::resized()
{
    // Layout:
    // Row 1: Waveform (top, 20%)
    // Row 2: Turntable (Large central feature, 50%)
    // Row 3: Controls (Filter, Speed, Vol)
    // Row 4: Buttons

    double h = getHeight();
    double w = getWidth();

    double waveformH = h * 0.15;
    double turntableH = h * 0.45;
    double controlsH = h * 0.25;
    double buttonsH = h * 0.15;

    waveformDisplay.setBounds(0, 0, w, waveformH);
    posSlider.setBounds(0, waveformH, w, 20); // Overlay or just below

    // Turntable
    turntableComponent.setBounds(0, waveformH + 20, w, turntableH - 20);

    // Controls
    double colW = w / 3;
    double controlsY = waveformH + turntableH;
    filterSlider.setBounds(0, controlsY, colW, controlsH);
    speedSlider.setBounds(colW, controlsY, colW, controlsH);
    volSlider.setBounds(colW * 2, controlsY, colW, controlsH);

    // Buttons
    double buttonsY = controlsY + controlsH;
    double btnW = w / 4;
    playButton.setBounds(0, buttonsY, btnW, buttonsH);
    stopButton.setBounds(btnW, buttonsY, btnW, buttonsH);
    loopButton.setBounds(btnW * 2, buttonsY, btnW, buttonsH);
    loadButton.setBounds(btnW * 3, buttonsY, btnW, buttonsH);
}

void DeckGUI::buttonClicked(Button* button)
{
    if (button == &playButton)
    {
        player->start();
    }
     if (button == &stopButton)
    {
        player->stop();

    }
    if (button == &loopButton)
    {
        player->setLooping(loopButton.getToggleState());
    }
    if (button == &loadButton)
    {
       auto fileChooserFlags =
        FileBrowserComponent::canSelectFiles;
        fChooser.launchAsync(fileChooserFlags, [this](const FileChooser& chooser)
        {
            File chosenFile = chooser.getResult();
            if (chosenFile.exists()){
                loadFile(chosenFile);
            }
        });
    }
}

void DeckGUI::sliderValueChanged (Slider *slider)
{
    if (slider == &volSlider)
    {
        player->setGain(slider->getValue());
    }

    if (slider == &speedSlider)
    {
        player->setSpeed(slider->getValue());
    }

    if (slider == &posSlider)
    {
        player->setPositionRelative(slider->getValue());
    }

    if (slider == &filterSlider)
    {
        player->setFilter(slider->getValue());
    }

}

bool DeckGUI::isInterestedInFileDrag (const StringArray &files)
{
  return true;
}

void DeckGUI::filesDropped (const StringArray &files, int x, int y)
{
  if (files.size() == 1)
  {
      loadFile(File{files[0]});
  }
}

void DeckGUI::timerCallback()
{
    waveformDisplay.setPositionRelative(
            player->getPositionRelative());
}

void DeckGUI::loadFile(File f)
{
    URL audioURL{f};
    player->loadURL(audioURL);
    waveformDisplay.loadURL(audioURL);
}
