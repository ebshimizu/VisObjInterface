/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "SearchResultsViewer.h"
#include "ParamControls.h"
#include "AttributeControls.h"
#include "SceneViewer.h"


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public Component, public ApplicationCommandTarget
{
public:
  //==============================================================================
  MainContentComponent();
  ~MainContentComponent();

  void paint (Graphics&);
  void resized();

  virtual ApplicationCommandTarget* getNextCommandTarget() override;
  virtual void getAllCommands(Array<CommandID>& commands) override;
  virtual void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;
  virtual bool perform(const InvocationInfo& info) override;

private:
  // Open a Lumiverse file
  void openRig();

  // Initializes the components in the interface after loading a rig
  void loadComponents();

  // Refrehses the parameter controls
  void refreshParams();

  // Opens the settings window
  void openSettings();

  // Private vars
  File _parentDir;
  String _showName;

  // Top/bottom split
  StretchableLayoutManager _horizResizer;

  // Left/center/right split for top
  StretchableLayoutManager _vertResizer;

  // Horizontal bar
  ScopedPointer<StretchableLayoutResizerBar> _hbar;
  
  // Vertical bars
  ScopedPointer<StretchableLayoutResizerBar> _vbar1;
  ScopedPointer<StretchableLayoutResizerBar> _vbar2;

  // Components
  ScopedPointer<SearchResultsViewer> _search;
  ScopedPointer<ParamControls> _params;
  ScopedPointer<AttributeControls> _attrs;
  ScopedPointer<SceneViewer> _viewer;

  SafePointer<DocumentWindow> _settingsWindow;

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
