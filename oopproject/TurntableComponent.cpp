/*
  ==============================================================================

    TurntableComponent.cpp
    Created: 21 Jan 2025
    Author:  Jules

  ==============================================================================
*/

#include "TurntableComponent.h"

TurntableComponent::TurntableComponent(DJAudioPlayer* _player)
    : player(_player)
{
    startTimer(30); // ~30fps animation
}

TurntableComponent::~TurntableComponent()
{
    stopTimer();
}

void TurntableComponent::paint (Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto center = bounds.getCentre();
    float radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 5.0f;

    // Draw Vinyl Disk
    g.setColour(Colours::black);
    g.fillEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2);

    // Draw Grooves (Texture)
    g.setColour(Colours::darkgrey.withAlpha(0.5f));
    for (float r = radius * 0.4f; r < radius * 0.9f; r += 5.0f)
    {
        g.drawEllipse(center.x - r, center.y - r, r * 2, r * 2, 1.0f);
    }

    // Draw Label
    float labelRadius = radius * 0.35f;
    g.setColour(Colours::red);
    g.fillEllipse(center.x - labelRadius, center.y - labelRadius, labelRadius * 2, labelRadius * 2);

    // Rotate context
    g.saveState();
    AffineTransform transform = AffineTransform::rotation(rotationAngle, center.x, center.y);
    g.addTransform(transform);

    // Text on label
    g.setColour(Colours::white);
    g.setFont(12.0f);
    g.drawText("OTO", center.x - 20, center.y - 10, 40, 20, Justification::centred);

    // Marker
    g.fillRect(center.x - 2, center.y - radius + 5, 4.0f, radius * 0.3f);

    g.restoreState();

    // Spindle
    g.setColour(Colours::silver);
    g.fillEllipse(center.x - 5, center.y - 5, 10, 10);
}

void TurntableComponent::resized()
{
}

void TurntableComponent::timerCallback()
{
    if (player->isPlaying())
    {
        rotationAngle += 0.1;
        if (rotationAngle > MathConstants<double>::twoPi)
            rotationAngle -= MathConstants<double>::twoPi;

        repaint();
    }
}
