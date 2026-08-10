/*
  ==============================================================================

    MixerComponent.cpp
    Created: 21 Jan 2025
    Author:  Jules

  ==============================================================================
*/

#include "MixerComponent.h"

MixerComponent::MixerComponent(DJAudioPlayer* _player1, DJAudioPlayer* _player2)
    : player1(_player1), player2(_player2)
{
    // Setup sliders
    auto setupSlider = [this](Slider& slider) {
        slider.setSliderStyle(Slider::Rotary);
        slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
        slider.setRange(0.2, 5.0); // Requirement R4
        slider.setValue(1.0);
        slider.addListener(this);
        addAndMakeVisible(slider);
    };

    setupSlider(highEQ1);
    setupSlider(midEQ1);
    setupSlider(lowEQ1);

    setupSlider(highEQ2);
    setupSlider(midEQ2);
    setupSlider(lowEQ2);

    // Crossfader
    crossfader.setSliderStyle(Slider::LinearHorizontal);
    crossfader.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    crossfader.setRange(-1.0, 1.0);
    crossfader.setValue(0.0);
    crossfader.addListener(this);
    addAndMakeVisible(crossfader);
}

MixerComponent::~MixerComponent()
{
}

void MixerComponent::paint (Graphics& g)
{
    g.fillAll (Colours::darkgrey);
    g.setColour (Colours::black);
    g.drawRect (getLocalBounds(), 2);

    // Labels
    g.setColour(Colours::white);
    g.setFont(10.0f);
    // Draw labels logic here or just rely on layout
}

void MixerComponent::resized()
{
    // 2 Columns for EQ, Crossfader at bottom
    double w = getWidth();
    double h = getHeight();
    double colW = w / 2;
    double rowH = (h - 40) / 3; // 3 EQ rows

    // Col 1
    highEQ1.setBounds(0, 0, colW, rowH);
    midEQ1.setBounds(0, rowH, colW, rowH);
    lowEQ1.setBounds(0, rowH * 2, colW, rowH);

    // Col 2
    highEQ2.setBounds(colW, 0, colW, rowH);
    midEQ2.setBounds(colW, rowH, colW, rowH);
    lowEQ2.setBounds(colW, rowH * 2, colW, rowH);

    // Crossfader
    crossfader.setBounds(10, h - 40, w - 20, 30);
}

void MixerComponent::sliderValueChanged(Slider* slider)
{
    // EQ Logic
    if (slider == &highEQ1 || slider == &midEQ1 || slider == &lowEQ1)
    {
        player1->setEQ(highEQ1.getValue(), midEQ1.getValue(), lowEQ1.getValue());
    }

    if (slider == &highEQ2 || slider == &midEQ2 || slider == &lowEQ2)
    {
        player2->setEQ(highEQ2.getValue(), midEQ2.getValue(), lowEQ2.getValue());
    }

    // Crossfader Logic
    if (slider == &crossfader)
    {
        double fade = slider->getValue(); // -1 to 1
        // Simple linear fade:
        // -1: Deck 1 = 1.0, Deck 2 = 0.0
        //  0: Deck 1 = 0.5, Deck 2 = 0.5 (or 1.0 depending on curve)
        //  1: Deck 1 = 0.0, Deck 2 = 1.0

        // Linear mapping:
        double vol1 = 0.5 * (1.0 - fade);
        double vol2 = 0.5 * (1.0 + fade);

        // Apply to players gain? No, players have their own volume sliders.
        // The spec says "calculate inverse volume".
        // Usually crossfader scales the existing channel volume.
        // We probably need a method `setCrossfadeGain` on players?
        // Or we just abuse `setGain`?
        // But `DeckGUI` also has volume sliders.
        // If we abuse `setGain`, the deck sliders will fight with mixer.
        // The clean way: `DJAudioPlayer` should have `setGain` (channel fader) AND `setMasterGain` (crossfader)?
        // Or `DeckGUI` updates `DJAudioPlayer`'s volume, and `Mixer` updates... ?
        // Let's assume `DJAudioPlayer::setGain` is the final volume.
        // Wait, `DeckGUI` volume slider is likely the Channel Fader.
        // So `MixerComponent` crossfader should ideally multiply that.
        // But `MixerComponent` doesn't know the `DeckGUI` volume.

        // Hack: `DJAudioPlayer` has `setGain` (channel) and `setCrossfade`?
        // Let's verify `DJAudioPlayer` later.

        // For now, I'll assume we can call `setGain` but that overrides DeckGUI.
        // Spec says: "Middle Mixer... Contains EQ knobs and Faders".
        // Wait, spec section 1B says "Left Deck... Contains Turntable, Transport, and Pitch".
        // Spec section 1B also says "Center Mixer... Contains EQ knobs and Faders".
        // FADERS usually means Channel Volume Faders.
        // BUT section 2 DeckGUI says "juce::Slider volSlider".

        // CONTRADICTION in SPEC:
        // 1B: Mixer has Faders. Deck has Pitch.
        // 2: DeckGUI has volSlider.

        // I will follow Section 2 Class Breakdown as primary for members.
        // DeckGUI has volSlider. Mixer has EQ + Crossfader (from Section 3).
        // Section 3 Master Control says "juce::Slider crossfader".

        // So how does crossfader work?
        // I will add `setCrossfadeFactor(double)` to `DJAudioPlayer`.
        player1->setCrossfadeFactor(vol1);
        player2->setCrossfadeFactor(vol2);
    }
}
