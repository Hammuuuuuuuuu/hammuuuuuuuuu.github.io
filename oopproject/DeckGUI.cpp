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

    addAndMakeVisible(loop1Btn);
    addAndMakeVisible(loop2Btn);
    addAndMakeVisible(loop3Btn);

    addAndMakeVisible(cue1Btn);
    addAndMakeVisible(cue2Btn);
    addAndMakeVisible(cue3Btn);

    addAndMakeVisible(volSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(posSlider);

    addAndMakeVisible(lowSlider);
    addAndMakeVisible(midSlider);
    addAndMakeVisible(highSlider);

    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(turntableComponent);


    playButton.addListener(this);
    stopButton.addListener(this);
    loadButton.addListener(this);
    loopButton.addListener(this);

    loop1Btn.addListener(this);
    loop2Btn.addListener(this);
    loop3Btn.addListener(this);

    cue1Btn.addListener(this);
    cue2Btn.addListener(this);
    cue3Btn.addListener(this);

    volSlider.addListener(this);
    speedSlider.addListener(this);
    posSlider.addListener(this);

    lowSlider.addListener(this);
    midSlider.addListener(this);
    highSlider.addListener(this);


    volSlider.setRange(0.0, 1.0);
    volSlider.setValue(1.0);

    speedSlider.setRange(0.1, 5.0);
    speedSlider.setValue(1.0);

    posSlider.setRange(0.0, 1.0);

    // EQ Ranges: 0.1 to 4.0, skew for center
    lowSlider.setRange(0.1, 4.0);
    lowSlider.setValue(1.0);
    lowSlider.setSliderStyle(Slider::Rotary);
    lowSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    midSlider.setRange(0.1, 4.0);
    midSlider.setValue(1.0);
    midSlider.setSliderStyle(Slider::Rotary);
    midSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    highSlider.setRange(0.1, 4.0);
    highSlider.setValue(1.0);
    highSlider.setSliderStyle(Slider::Rotary);
    highSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    volSlider.setSliderStyle(Slider::LinearBarVertical);
    volSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    speedSlider.setSliderStyle(Slider::Rotary);
    speedSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

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

    g.setColour(Colours::white);
    g.setFont(10.0f);

    // Draw labels for EQ
    auto bounds = getLocalBounds();
    int h = getHeight();
    int w = getWidth();
    int eqY = h * 0.5 + 20;

    // Crude labelling
    // g.drawText("L", 0, eqY, w/3, 20, Justification::centredTop);
    // g.drawText("M", w/3, eqY, w/3, 20, Justification::centredTop);
    // g.drawText("H", w*2/3, eqY, w/3, 20, Justification::centredTop);
}

void DeckGUI::resized()
{
    // Layout:
    // Row 1: Waveform (top, 15%)
    // Row 2: Turntable (35%)
    // Row 3: EQ Knobs (20%)
    // Row 4: Controls (Speed, Vol) + Buttons (30%)

    double h = getHeight();
    double w = getWidth();

    double waveformH = h * 0.15;
    double turntableH = h * 0.35;
    double eqH = h * 0.20;
    double controlsH = h * 0.30;

    waveformDisplay.setBounds(0, 0, w, waveformH);
    posSlider.setBounds(0, waveformH, w, 20); // Overlay or just below

    // Turntable
    turntableComponent.setBounds(0, waveformH + 20, w, turntableH - 20);

    // EQ
    double eqW = w / 3;
    double eqY = waveformH + turntableH;
    lowSlider.setBounds(0, eqY, eqW, eqH);
    midSlider.setBounds(eqW, eqY, eqW, eqH);
    highSlider.setBounds(eqW * 2, eqY, eqW, eqH);

    // Controls & Buttons
    double controlsY = eqY + eqH;

    // Sub-row 1: Speed, Vol
    double subRowH = controlsH * 0.4;
    speedSlider.setBounds(0, controlsY, w/2, subRowH);
    volSlider.setBounds(w/2, controlsY, w/2, subRowH);

    // Sub-row 2: Transport & Utility
    double btnY = controlsY + subRowH;
    double btnH = controlsH * 0.3;
    double btnW = w / 4;

    playButton.setBounds(0, btnY, btnW, btnH);
    stopButton.setBounds(btnW, btnY, btnW, btnH);
    loadButton.setBounds(btnW*2, btnY, btnW, btnH);
    loopButton.setBounds(btnW*3, btnY, btnW, btnH);

    // Sub-row 3: Cues & Loops
    double row3Y = btnY + btnH;
    double row3H = controlsH * 0.3;
    double smallBtnW = w / 6;

    cue1Btn.setBounds(0, row3Y, smallBtnW, row3H);
    cue2Btn.setBounds(smallBtnW, row3Y, smallBtnW, row3H);
    cue3Btn.setBounds(smallBtnW*2, row3Y, smallBtnW, row3H);

    loop1Btn.setBounds(smallBtnW*3, row3Y, smallBtnW, row3H);
    loop2Btn.setBounds(smallBtnW*4, row3Y, smallBtnW, row3H);
    loop3Btn.setBounds(smallBtnW*5, row3Y, smallBtnW, row3H);
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

    if (button == &cue1Btn)
    {
        if (player->hasCue(0)) player->jumpToCue(0);
        else player->setCue(0);
    }
    if (button == &cue2Btn)
    {
        if (player->hasCue(1)) player->jumpToCue(1);
        else player->setCue(1);
    }
    if (button == &cue3Btn)
    {
        if (player->hasCue(2)) player->jumpToCue(2);
        else player->setCue(2);
    }

    // Beat Loops (Assuming 120 BPM = 0.5s per beat)
    // 1/2 Beat = 0.25s
    // 1 Beat = 0.5s
    // 4 Beats = 2.0s
    if (button == &loop1Btn) player->setBeatLoop(0.25);
    if (button == &loop2Btn) player->setBeatLoop(0.5);
    if (button == &loop3Btn) player->setBeatLoop(2.0);
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

    if (slider == &lowSlider) player->setLow(slider->getValue());
    if (slider == &midSlider) player->setMid(slider->getValue());
    if (slider == &highSlider) player->setHigh(slider->getValue());
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

    // Update Cue Button colours to indicate if set
    cue1Btn.setColour(TextButton::buttonColourId, player->hasCue(0) ? Colours::orange : getLookAndFeel().findColour(TextButton::buttonColourId));
    cue2Btn.setColour(TextButton::buttonColourId, player->hasCue(1) ? Colours::orange : getLookAndFeel().findColour(TextButton::buttonColourId));
    cue3Btn.setColour(TextButton::buttonColourId, player->hasCue(2) ? Colours::orange : getLookAndFeel().findColour(TextButton::buttonColourId));
}

void DeckGUI::loadFile(File f)
{
    URL audioURL{f};
    player->loadURL(audioURL);
    waveformDisplay.loadURL(audioURL);

    // Try to find album art
    // Strategy: Look for "cover.jpg" or same filename with .jpg/.png in same dir.
    File parent = f.getParentDirectory();
    File coverArt = parent.getChildFile("cover.jpg");
    if (!coverArt.existsAsFile())
    {
        coverArt = f.withFileExtension("jpg");
    }
    if (!coverArt.existsAsFile())
    {
        coverArt = f.withFileExtension("png");
    }

    turntableComponent.loadAlbumArt(coverArt);
}
