/*
  ==============================================================================

    DeckGUI.h
    Created: 13 Mar 2020 6:44:48pm
    Author:  matthew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"
#include "TurntableComponent.h"

//==============================================================================
/*
*/
class DeckGUI    : public Component,
                   public Button::Listener,
                   public Slider::Listener,
                   public FileDragAndDropTarget,
                   public Timer
{
public:
    DeckGUI(DJAudioPlayer* player,
           AudioFormatManager & 	formatManagerToUse,
           AudioThumbnailCache & 	cacheToUse );
    ~DeckGUI();

    void paint (Graphics&) override;
    void resized() override;

     /** implement Button::Listener */
    void buttonClicked (Button *) override;

    /** implement Slider::Listener */
    void sliderValueChanged (Slider *slider) override;

    bool isInterestedInFileDrag (const StringArray &files) override;
    void filesDropped (const StringArray &files, int x, int y) override;

    void timerCallback() override;

    void loadFile(File f);

private:
    juce::FileChooser fChooser{"Select a file..."};

    TextButton playButton{"PLAY"};
    TextButton stopButton{"STOP"};
    TextButton loadButton{"LOAD"};
    ToggleButton loopButton{"LOOP"};

    // Beat Loop Buttons
    TextButton loop1Btn{"1/2"}; // 1/2 Beat (assuming 120bpm = 0.25s)
    TextButton loop2Btn{"1"};   // 1 Beat (0.5s)
    TextButton loop3Btn{"4"};   // 4 Beats (2.0s)

    // Cue Buttons
    TextButton cue1Btn{"1"};
    TextButton cue2Btn{"2"};
    TextButton cue3Btn{"3"};

    // Sliders
    Slider volSlider;
    Slider speedSlider;
    Slider posSlider;

    // EQ Sliders
    Slider lowSlider;
    Slider midSlider;
    Slider highSlider;

    WaveformDisplay waveformDisplay;
    TurntableComponent turntableComponent;

    DJAudioPlayer* player;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeckGUI)
};
