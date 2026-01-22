/*
  ==============================================================================

    TrackInfo.h
    Created: 21 Jan 2025
    Author:  Jules

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

struct TrackInfo {
    File file;
    String title;
    String artist;
    String duration; // optional

    TrackInfo(File f) : file(f) {
        title = f.getFileNameWithoutExtension();
        artist = "Unknown";
    }
};
