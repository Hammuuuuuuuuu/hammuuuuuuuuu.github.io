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
    return filteredFiles.size();
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
    if (rowNumber < filteredFiles.size())
    {
        if (columnId == 1)
        {
            g.drawText(filteredFiles[rowNumber].getFileName(),
                       2, 0, width - 4, height,
                       Justification::centredLeft, true);
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
                    trackFiles.push_back(file);
                }
                textEditorTextChanged(searchBox);
                tableComponent.updateContent();
            }
        });
    }
    else
    {
        String id = button->getComponentID();
        if (id.startsWith("2_"))
        {
            int row = id.substring(2).getIntValue();
            if (row < filteredFiles.size())
                deck1->loadFile(filteredFiles[row]);
        }
        else if (id.startsWith("3_"))
        {
            int row = id.substring(2).getIntValue();
             if (row < filteredFiles.size())
                deck2->loadFile(filteredFiles[row]);
        }
    }
}

void PlaylistComponent::textEditorTextChanged(TextEditor& editor)
{
    String searchText = editor.getText();
    filteredFiles.clear();

    if (searchText.isEmpty())
    {
        filteredFiles = trackFiles;
    }
    else
    {
        for (auto& file : trackFiles)
        {
            if (file.getFileName().containsIgnoreCase(searchText))
            {
                filteredFiles.push_back(file);
            }
        }
    }
    tableComponent.updateContent();
}

void PlaylistComponent::loadLibrary()
{
    // Load from text file
    std::ifstream file("playlist_library.txt");
    std::string str;
    if (file.is_open())
    {
        while (std::getline(file, str))
        {
            File f{String(str)};
            if (f.existsAsFile())
            {
                trackFiles.push_back(f);
            }
        }
    }
    // Initialize filtered list
    filteredFiles = trackFiles;
}

void PlaylistComponent::saveLibrary()
{
    // Save to text file
    std::ofstream file("playlist_library.txt");
    if (file.is_open())
    {
        for (auto& f : trackFiles)
        {
            file << f.getFullPathName().toStdString() << "\n";
        }
    }
}
