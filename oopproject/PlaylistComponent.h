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
#include "DeckGUI.h"

//==============================================================================
/*
*/
class PlaylistComponent : public Component,
                          public TableListBoxModel,
                          public Button::Listener,
                          public TextEditor::Listener
{
public:
    PlaylistComponent(DeckGUI* _deck1, DeckGUI* _deck2, AudioFormatManager& formatManager);
    ~PlaylistComponent();

    void paint (Graphics&) override;
    void resized() override;

    int getNumRows() override;
    void paintRowBackground (Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell (Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

    void buttonClicked(Button* button) override;
    void textEditorTextChanged(TextEditor& editor) override;

    void loadLibrary();
    void saveLibrary();

private:
    TableListBox tableComponent;
    std::vector<File> trackFiles;
    std::vector<File> filteredFiles; // For search

    TextButton importButton{"IMPORT"};
    TextEditor searchBox;

    juce::FileChooser fChooser{"Select files..."};

    DeckGUI* deck1;
    DeckGUI* deck2;
    AudioFormatManager& formatManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaylistComponent)
};
