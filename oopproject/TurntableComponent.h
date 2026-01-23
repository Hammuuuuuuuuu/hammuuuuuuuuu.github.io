/*
  ==============================================================================

    TurntableComponent.h
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
class TurntableComponent : public Component,
                           public Timer
{
public:
    TurntableComponent(DJAudioPlayer* player);
    ~TurntableComponent();

    void paint (Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    DJAudioPlayer* player;
    double rotationAngle = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TurntableComponent)
};
