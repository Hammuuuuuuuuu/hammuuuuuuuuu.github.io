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
    float radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 10.0f;

    // Draw Vinyl Disk
    g.setColour(Colours::black);
    g.fillEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2);

    // Draw Grooves (Texture)
    g.setColour(Colours::darkgrey.withAlpha(0.5f));
    for (float r = radius * 0.4f; r < radius * 0.9f; r += 5.0f)
    {
        g.drawEllipse(center.x - r, center.y - r, r * 2, r * 2, 1.0f);
    }

    // Draw Label (Center)
    float labelRadius = radius * 0.35f;
    g.setColour(Colours::red);
    g.fillEllipse(center.x - labelRadius, center.y - labelRadius, labelRadius * 2, labelRadius * 2);

    // Draw Spindle
    g.setColour(Colours::silver);
    g.fillEllipse(center.x - 5, center.y - 5, 10, 10);

    // Draw Marker (to visualize rotation)
    // We rotate the context around the center
    g.saveState();

    AffineTransform transform = AffineTransform::rotation(rotationAngle, center.x, center.y);
    g.addTransform(transform);

    g.setColour(Colours::white);
    g.fillRect(center.x - 2, center.y - radius + 10, 4.0f, radius * 0.3f); // White strip on the edge

    // Text on label
    g.setColour(Colours::white);
    g.setFont(12.0f);
    g.drawText("VINYL", center.x - 20, center.y - 10, 40, 20, Justification::centred);

    g.restoreState();
}

void TurntableComponent::resized()
{
}

void TurntableComponent::mouseDown(const MouseEvent& event)
{
    isScratching = true;
    lastMouseAngle = event.position.getAngleToPoint(getLocalBounds().getCentre().toFloat());
}

void TurntableComponent::mouseDrag(const MouseEvent& event)
{
    if (isScratching)
    {
        float currentAngle = event.position.getAngleToPoint(getLocalBounds().getCentre().toFloat());
        float delta = currentAngle - lastMouseAngle;

        // Handle wrap-around (e.g. going from PI to -PI)
        if (delta > MathConstants<float>::pi) delta -= MathConstants<float>::twoPi;
        if (delta < -MathConstants<float>::pi) delta += MathConstants<float>::twoPi;

        rotationAngle += delta;
        lastMouseAngle = currentAngle;

        // Convert rotation to seek
        // One full rotation = 2 seconds? Or arbitrary.
        // Let's say 1 full rotation = 5% of track for sensitivity?
        // Better: Scrubbing.
        double currentPos = player->getPositionRelative();
        double seekAmt = delta / (MathConstants<float>::twoPi * 10.0); // Sensitivity

        double newPos = currentPos + seekAmt;
        if (newPos < 0) newPos = 0;
        if (newPos > 1) newPos = 1;

        player->setPositionRelative(newPos);

        repaint();
    }
}

void TurntableComponent::mouseUp(const MouseEvent& event)
{
    isScratching = false;
}

void TurntableComponent::timerCallback()
{
    // Check if player is playing
    // DJAudioPlayer doesn't expose "isPlaying" state directly, but we can infer or add it.
    // For now, let's just rotate if the position is moving?
    // Or we assume 33 RPM if playing.

    // Actually, we should check with the player.
    // Let's add isPlaying to DJAudioPlayer or just rely on position changes.
    // Checking position change is safer.

    static double lastPos = -1.0;
    double currentPos = player->getPositionRelative();

    if (currentPos != lastPos && !isScratching)
    {
        // It's playing
        // 33 1/3 RPM = ~0.55 rotations per second
        // At 30fps, that's 0.018 revs per frame = 0.11 rads per frame
        rotationAngle += 0.1;
        if (rotationAngle > MathConstants<double>::twoPi)
            rotationAngle -= MathConstants<double>::twoPi;

        repaint();
    }
    lastPos = currentPos;
}
