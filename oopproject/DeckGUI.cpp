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
               turntable(_player)
{

    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(loadButton);

    addAndMakeVisible(volSlider);
    addAndMakeVisible(speedSlider);

    addAndMakeVisible(trackNameLabel);
    addAndMakeVisible(turntable);

    playButton.addListener(this);
    stopButton.addListener(this);
    loadButton.addListener(this);

    volSlider.addListener(this);
    speedSlider.addListener(this);

    volSlider.setRange(0.0, 1.0);
    volSlider.setValue(1.0);

    speedSlider.setRange(0.1, 5.0);
    speedSlider.setValue(1.0);

    // Styles
    volSlider.setSliderStyle(Slider::LinearBarVertical);
    volSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    speedSlider.setSliderStyle(Slider::Rotary);
    speedSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    trackNameLabel.setText("No Track Loaded", dontSendNotification);
    trackNameLabel.setJustificationType(Justification::centred);
}

DeckGUI::~DeckGUI()
{
}

void DeckGUI::paint (Graphics& g)
{
    g.fillAll (Colours::lightgrey);   // clear the background
    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
}

void DeckGUI::resized()
{
    // Layout logic for Region B (DeckGUI)
    // Needs Turntable, Transport, Pitch, Vol

    double w = getWidth();
    double h = getHeight();

    // Top 60%: Turntable
    turntable.setBounds(0, 0, w, h * 0.6);

    // Remaining 40%
    double controlsY = h * 0.6;
    double controlsH = h * 0.4;

    // Track Label
    trackNameLabel.setBounds(0, controlsY, w, 20);

    // Sliders: Speed (Left), Vol (Right)
    double sliderY = controlsY + 20;
    double sliderH = controlsH - 50; // Leave space for buttons
    speedSlider.setBounds(0, sliderY, w/2, sliderH);
    volSlider.setBounds(w/2, sliderY, w/2, sliderH);

    // Buttons
    double btnY = h - 30;
    double btnW = w / 3;
    playButton.setBounds(0, btnY, btnW, 30);
    stopButton.setBounds(btnW, btnY, btnW, 30);
    loadButton.setBounds(btnW*2, btnY, btnW, 30);
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

bool DeckGUI::isInterestedInDragSource (const SourceDetails& dragSourceDetails)
{
    // Check if it's a file path string?
    return true;
}

void DeckGUI::itemDropped (const SourceDetails& dragSourceDetails)
{
    String path = dragSourceDetails.description.toString();
    File f{path};
    if (f.existsAsFile())
    {
        loadFile(f);
    }
}

void DeckGUI::loadFile(File f)
{
    URL url{f};
    player->loadURL(url);
    trackNameLabel.setText(f.getFileNameWithoutExtension(), dontSendNotification);

    // Notify listeners
    listeners.call([this, url](Listener& l){ l.fileLoaded(this, url); });
}
