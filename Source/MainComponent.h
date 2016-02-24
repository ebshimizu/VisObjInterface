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
#include "SettingsEditor.h"
#include "ClusterBuster.h"


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

  // Refrehses the parameter controls
  void refreshParams();

  void refreshAttr();
  void arnoldRender();

  // The big search function. This function does the following:
  // - Performs a number of searches using particular sets of constraints (edits)
  // - Groups results (if any) and presents them to the user
  void search();

  // Typically called from an AttributeSearchResult to place results in the bottom part
  // of the search results viewer
  void setBottomSearchComponent(Component* c, Component* source);

  // Adds the current state of the rig and rendered image to the history panel.
  void addHistory();

  // Undoes the previous operation (pops off the history stack)
  void undo();

  // Redoes the previous operation (returns history item to stack)
  void redo();

  // Tells the currently displayed cluster, if it exists, to sort itself
  void sortCluster();

  // Updates the cluster display gui element
  void refreshClusterDisplay();
private:
  // Open a Lumiverse file
  void openRig();
  void openRig(String fname);

  void saveRig();
  void saveAs();

  // Initializes the components in the interface after loading a rig
  void loadComponents();

  // Opens the settings window
  void openSettings();

  // Opens the clusters window
  void openClusters();

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

  SafePointer<SettingsWindow> _settingsWindow;
  SafePointer<ClusterBusterWindow> _clusterWindow;

  TooltipWindow _tips;

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
