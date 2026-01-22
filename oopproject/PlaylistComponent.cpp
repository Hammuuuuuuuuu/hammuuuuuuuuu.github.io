/*
  ==============================================================================

    PlaylistComponent.cpp
    Created: 21 Jan 2025
    Author:  Jules

  ==============================================================================
*/

#include "PlaylistComponent.h"
#include <iostream>
#include <fstream>

PlaylistComponent::PlaylistComponent(DeckGUI* _deck1, DeckGUI* _deck2, AudioFormatManager& _formatManager)
    : deck1(_deck1), deck2(_deck2), formatManager(_formatManager)
{
    addAndMakeVisible(tableComponent);
    addAndMakeVisible(importButton);
    addAndMakeVisible(searchBox);

    importButton.addListener(this);
    searchBox.addListener(this);

    tableComponent.getHeader().addColumn("Track title", 1, 200);
    tableComponent.getHeader().addColumn("Artist", 4, 150);
    tableComponent.getHeader().addColumn("", 2, 100); // Load Deck 1
    tableComponent.getHeader().addColumn("", 3, 100); // Load Deck 2
    tableComponent.setModel(this);

    loadLibrary();
}

PlaylistComponent::~PlaylistComponent()
{
    saveLibrary();
}

void PlaylistComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);

    g.setColour (Colours::white);
    g.setFont (14.0f);
    // g.drawText ("PlaylistComponent", getLocalBounds(), Justification::centred, true);
}

void PlaylistComponent::resized()
{
    tableComponent.setBounds(0, 40, getWidth(), getHeight() - 40);
    importButton.setBounds(0, 0, getWidth() / 2, 30);
    searchBox.setBounds(getWidth() / 2, 0, getWidth() / 2, 30);
}

int PlaylistComponent::getNumRows()
{
    return filteredIndices.size();
}

void PlaylistComponent::paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
    {
        g.fillAll(Colours::orange);
    }
    else
    {
        g.fillAll(Colours::darkgrey);
    }
}

void PlaylistComponent::paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (rowNumber < filteredIndices.size())
    {
        int index = filteredIndices[rowNumber];
        TrackInfo& track = tracks[index];

        if (columnId == 1) // Title
        {
            g.drawText(track.title,
                       2, 0, width - 4, height,
                       Justification::centredLeft, true);
        }
        else if (columnId == 4) // Artist
        {
             g.drawText(track.artist, 2, 0, width - 4, height, Justification::centredLeft, true);
        }
    }
}

Component* PlaylistComponent::refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate)
{
    if (columnId == 2)
    {
        if (existingComponentToUpdate == nullptr)
        {
            TextButton* btn = new TextButton("Deck 1");
            btn->setComponentID("2_" + String(rowNumber));
            btn->addListener(this);
            existingComponentToUpdate = btn;
        }
        else
        {
            existingComponentToUpdate->setComponentID("2_" + String(rowNumber));
        }
    }
    else if (columnId == 3)
    {
        if (existingComponentToUpdate == nullptr)
        {
            TextButton* btn = new TextButton("Deck 2");
            btn->setComponentID("3_" + String(rowNumber));
            btn->addListener(this);
            existingComponentToUpdate = btn;
        }
        else
        {
            existingComponentToUpdate->setComponentID("3_" + String(rowNumber));
        }
    }
    return existingComponentToUpdate;
}

void PlaylistComponent::buttonClicked(Button* button)
{
    if (button == &importButton)
    {
        auto fileChooserFlags = FileBrowserComponent::canSelectFiles | FileBrowserComponent::canSelectMultipleItems;
        fChooser.launchAsync(fileChooserFlags, [this](const FileChooser& chooser)
        {
            Array<File> files = chooser.getResults();
            if (files.size() > 0)
            {
                for (auto file : files)
                {
                    TrackInfo track{file};

                    // Parse Metadata
                    std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(file));
                    if (reader)
                    {
                        String artist = reader->metadataValues["artist"];
                        String title = reader->metadataValues["title"];

                        if (artist.isNotEmpty()) track.artist = artist;
                        if (title.isNotEmpty()) track.title = title;

                        // Fallback for ID3 if standard keys fail
                        if (track.artist == "Unknown" && reader->metadataValues.containsKey("id3artist"))
                             track.artist = reader->metadataValues["id3artist"];
                        if (track.title == track.file.getFileNameWithoutExtension() && reader->metadataValues.containsKey("id3title"))
                             track.title = reader->metadataValues["id3title"];
                    }

                    tracks.push_back(track);
                }
                textEditorTextChanged(searchBox);
                tableComponent.updateContent();
                saveLibrary();
            }
        });
    }
    else
    {
        String id = button->getComponentID();
        if (id.startsWith("2_"))
        {
            int row = id.substring(2).getIntValue();
            if (row < filteredIndices.size()){
                deck1->loadFile(tracks[filteredIndices[row]].file);
            }
        }
        else if (id.startsWith("3_"))
        {
            int row = id.substring(2).getIntValue();
             if (row < filteredIndices.size()){
                deck2->loadFile(tracks[filteredIndices[row]].file);
             }
        }
    }
}

void PlaylistComponent::textEditorTextChanged(TextEditor& editor)
{
    String searchText = editor.getText();
    filteredIndices.clear();

    for (int i=0; i < tracks.size(); ++i)
    {
        if (searchText.isEmpty() || tracks[i].title.containsIgnoreCase(searchText) || tracks[i].artist.containsIgnoreCase(searchText))
        {
            filteredIndices.push_back(i);
        }
    }
    tableComponent.updateContent();
}

void PlaylistComponent::loadLibrary()
{
    // Load from text file
    std::ifstream file("playlist_library.txt");
    std::string str;
    tracks.clear();

    if (file.is_open())
    {
        while (std::getline(file, str))
        {
            File f{String(str)};
            if (f.existsAsFile())
            {
                tracks.push_back(TrackInfo{f});
            }
        }
    }
    // Refresh indices
    textEditorTextChanged(searchBox);
}

void PlaylistComponent::saveLibrary()
{
    // Save to text file
    std::ofstream file("playlist_library.txt");
    if (file.is_open())
    {
        for (auto& t : tracks)
        {
            file << t.file.getFullPathName().toStdString() << "\n";
        }
    }
}
