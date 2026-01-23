/*
  ==============================================================================

    PlaylistComponent.h
    Created: 21 Jan 2025
    Author:  Jules

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>
#include <string>

//==============================================================================
/*
*/
class PlaylistComponent : public Component,
                          public TableListBoxModel,
                          public Button::Listener,
                          public TextEditor::Listener,
                          public FileDragAndDropTarget
{
public:
    PlaylistComponent(AudioFormatManager& formatManager);
    ~PlaylistComponent();

    void paint (Graphics&) override;
    void resized() override;

    // TableListBoxModel
    int getNumRows() override;
    void paintRowBackground (Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell (Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    // Button::Listener
    void buttonClicked(Button* button) override;

    // TextEditor::Listener
    void textEditorTextChanged(TextEditor& editor) override;

    // DragAndDrop
    var getDragSourceDescription(const SparseSet<int>& selectedRows) override;
    bool isInterestedInFileDrag (const StringArray &files) override;
    void filesDropped (const StringArray &files, int x, int y) override;

private:
    TableListBox tableComponent;
    std::vector<File> trackFiles;
    std::vector<int> filteredIndices;

    TextButton importButton{"Load Library"};
    TextEditor searchBox;

    AudioFormatManager& formatManager;
    juce::FileChooser fChooser{"Select audio files..."};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaylistComponent)
};
