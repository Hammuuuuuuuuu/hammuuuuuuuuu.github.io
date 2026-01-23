/*
  ==============================================================================

    MixerComponent.h
    Created: 21 Jan 2025
    Author:  Jules

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"

//==============================================================================
/*
*/
class MixerComponent : public Component,
                       public Slider::Listener
{
public:
    MixerComponent(DJAudioPlayer* player1, DJAudioPlayer* player2);
    ~MixerComponent();

    void paint (Graphics&) override;
    void resized() override;

    void sliderValueChanged(Slider* slider) override;

private:
    DJAudioPlayer* player1;
    DJAudioPlayer* player2;

    // Channel 1 EQs
    Slider highEQ1, midEQ1, lowEQ1;

    // Channel 2 EQs
    Slider highEQ2, midEQ2, lowEQ2;

    // Master
    Slider crossfader;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerComponent)
};
