/*
  ==============================================================================

    PlaylistComponent.cpp
    Created: 21 Jan 2025
    Author:  Jules

  ==============================================================================
*/

#include "PlaylistComponent.h"

PlaylistComponent::PlaylistComponent(AudioFormatManager& _formatManager)
    : formatManager(_formatManager)
{
    addAndMakeVisible(tableComponent);
    addAndMakeVisible(importButton);
    addAndMakeVisible(searchBox);

    importButton.addListener(this);
    searchBox.addListener(this);
    searchBox.setTextToShowWhenEmpty("Search...", Colours::grey);

    tableComponent.getHeader().addColumn("Track Title", 1, 400);
    tableComponent.setModel(this);

    // Allow dragging rows
    // tableComponent.setMultipleSelectionEnabled(true);
}

PlaylistComponent::~PlaylistComponent()
{
}

void PlaylistComponent::paint (Graphics& g)
{
    g.fillAll (Colours::black);
    g.setColour(Colours::grey);
    g.drawRect(getLocalBounds(), 1);
}

void PlaylistComponent::resized()
{
    // Search bar top right?
    // Controls: "Load" buttons (User spec says "Load" buttons... maybe "Load" button opens chooser?)
    // "Load" buttons usually load to deck. But drag and drop is preferred in spec "Drag-and-Drop functionality to the decks above".

    double rowH = 30;
    importButton.setBounds(0, 0, 100, rowH);
    searchBox.setBounds(100, 0, getWidth() - 100, rowH);
    tableComponent.setBounds(0, rowH, getWidth(), getHeight() - rowH);
}

int PlaylistComponent::getNumRows()
{
    return filteredIndices.size();
}

void PlaylistComponent::paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected) g.fillAll(Colours::orange);
    else g.fillAll(Colours::darkgrey);
}

void PlaylistComponent::paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (rowNumber < filteredIndices.size())
    {
        int index = filteredIndices[rowNumber];
        if (index < trackFiles.size()) {
            g.setColour(Colours::white);
            g.drawText(trackFiles[index].getFileName(), 2, 0, width - 4, height, Justification::centredLeft, true);
        }
    }
}

void PlaylistComponent::buttonClicked(Button* button)
{
    if (button == &importButton)
    {
         auto fileChooserFlags = FileBrowserComponent::canSelectFiles | FileBrowserComponent::canSelectMultipleItems;
        fChooser.launchAsync(fileChooserFlags, [this](const FileChooser& chooser)
        {
            Array<File> files = chooser.getResults();
            for (auto f : files) {
                trackFiles.push_back(f);
            }
            textEditorTextChanged(searchBox); // Refresh
        });
    }
}

void PlaylistComponent::textEditorTextChanged(TextEditor& editor)
{
    String text = editor.getText();
    filteredIndices.clear();
    for (int i=0; i<trackFiles.size(); ++i)
    {
        if (text.isEmpty() || trackFiles[i].getFileName().containsIgnoreCase(text))
        {
            filteredIndices.push_back(i);
        }
    }
    tableComponent.updateContent();
}

var PlaylistComponent::getDragSourceDescription(const SparseSet<int>& selectedRows)
{
    // Build a StringArray of paths or just one file?
    // Simpler to drag one file for now.
    if (selectedRows.size() > 0)
    {
        int row = selectedRows[0];
        if (row < filteredIndices.size()) {
             int index = filteredIndices[row];
             if (index < trackFiles.size())
                return trackFiles[index].getFullPathName();
        }
    }
    return "";
}

bool PlaylistComponent::isInterestedInFileDrag (const StringArray &files)
{
    return true;
}
void PlaylistComponent::filesDropped (const StringArray &files, int x, int y)
{
    for (auto f : files) {
        trackFiles.push_back(File{f});
    }
    textEditorTextChanged(searchBox);
}
