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
#include "ConstraintEditor.h"
#include "SystemExplorer.h"

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
  void arnoldRender(bool add = true);
  void arnoldRenderNoPopup();

  // The big search function. This function does the following:
  // - Performs a number of searches using particular sets of constraints (edits)
  // - Groups results (if any) and presents them to the user
  void search();

  // Stops an active search
  void stopSearch();

  // Adds the current state of the rig and rendered image to the history panel.
  void addHistory();

  // Tells the currently displayed cluster, if it exists, to sort itself
  void sortCluster();

  // Updates the cluster display gui element
  void refreshClusterDisplay();

  void setThumbImage(Image img);
  void repaintRenderArea();

  // Removes duplicate results from the results display section
  void cleanUpResults(int resultsToKeep);

  void showNewResults();

	void clearClusters();

  // Transfers the currently selected devices from the source snapshot to the
  // stage. Selected devices are as stored in the param controls
  void transferSelected(Snapshot* source);
  void transferSelected(Snapshot* source, DeviceSet devices, bool transferPinned = true);

  void redrawResults();

  // fills the bottom area of the screen with views based off of default systems
  void populateSystemViews();

  bool isSearchRunning();

  // Checks the parameter component and retrieves selected devices
  StringArray getSelectedDeviceIds();

  void setColors(vector<Eigen::VectorXd> colors, double intens, vector<float> weights);

  // resets the interface
  // Which is the following: unlock all lights, reset to open white, clear all
  // explore panels, empty the results panel
  void reset();

  // adds an idea to the ideas panel based on the specified image
  void createIdea(Image i, String name, IdeaType type);

  // shows a dialog box containing the devices affected by the selected region
  void debugShowAffectedDevices(juce::Rectangle<float> region);
  DeviceSet computeAffectedDevices(juce::Rectangle<float> region);

  void setSelectedIds(DeviceSet selection);

  // toggles if the selection view is being shown
  void toggleSelectView();

  // creates view in the explorer panel from the selected devices
  void createView(DeviceSet selection);

  // removes all objectives from the list
  void deleteAllObjectives();

  // copy and paste operations. Works on currently selected devices.
  void copyDeviceParams();
  void pasteDeviceParams(set<string> paramsToPaste);
  void setSelectedDevicesTo(float val);
  void setSelectedColorTo(float r, float g, float b);

  void getEosSelection();

private:
  // Open a Lumiverse file
  void openRig();
  void openRig(String fname);
  void openMask();
  void openMask(File mask);

  void saveRig();
  void saveAs();
  void saveRender();

  void saveResults();
  void loadResults();

  void loadTraces();
  void pickTrace();

  // Initializes the components in the interface after loading a rig
  void loadComponents();

  // parameter locking
  void lockAllColor();
  void lockAllIntensity();
  void lockAllPosition();
  void unlockAll();
  void lockDevice(Device* d);
  void getDefaultsFromArnold();

	void saveClusters();
	void loadClusters();

  // Starts automatic processing based on command line args.
  void startAuto();
  void endAuto();

  void createLogDirs();

  // Reloads the image attributes
  void reloadImageAttrs();
  void loadImageAttrsFromDir();

  // idea serialization
  void saveIdeas(bool backup = false);
  void loadIdeas();

  // region management
  void deleteAllPins();

  // Opens a selection box presenting a dropdown consisting of the selected
  // metadata field
  void selectBox(string metadataKey, bool inv, string title);

  void showPrompt();

  // runs a sync from an osc patch, if it exists
  void sync();

  // opens a dialog box with some info in it
  void about();

  // Private vars
  File _parentDir;
  String _showName;

  // Top/bottom split
  StretchableLayoutManager _horizResizer;

  // Left/right split
  StretchableLayoutManager _vertResizer;

  // view/results split
  StretchableLayoutManager _viewerSearchResizer;

  // Horizontal bar
  ScopedPointer<StretchableLayoutResizerBar> _hbar;
  
  // Vertical bars
  ScopedPointer<StretchableLayoutResizerBar> _vbar;
  ScopedPointer<StretchableLayoutResizerBar> _vbar2;

  // Components
  ScopedPointer<SearchResultsViewer> _search;
  ScopedPointer<AttributeControls> _attrs;
  ScopedPointer<SceneViewer> _viewer;
  ScopedPointer<ExplorerPanel> _exp;

  // Search object
  ScopedPointer<AttributeSearch> _searchWorker;

  TooltipWindow _tips;

  // copypasta data
  shared_ptr<LumiverseFloat> _intensCP;
  shared_ptr<LumiverseColor> _colorCP;

  class AutoTimer : public Timer
  {
  public:
    AutoTimer();
    ~AutoTimer();

    void setWorker(AttributeSearch* worker);
    virtual void timerCallback();
  private:
    AttributeSearch* _worker;
  };
  AutoTimer _autoTimer;

  // Indicates if the user manually stopped the search.
  bool _searchWasStopped;

  class SyncBackgroundThread : public ThreadWithProgressWindow {
  public:
    SyncBackgroundThread();
    ~SyncBackgroundThread();

    void run();
    void threadComplete(bool userPressedCancel);
  };

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
