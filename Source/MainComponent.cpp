/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"
#include "globals.h"
#include "SettingsEditor.h"
#include "AttributeSearch.h"
#include "HistoryPanel.h"
#include "ImageAttribute.h"

//==============================================================================
MainContentComponent::MainContentComponent()
{
  Lumiverse::Logger::setLogLevel(INFO);
  addAndMakeVisible(getStatusBar());

  // create log file for this session
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, 80, "actions-%F-%H-%M-%S.log", timeinfo);
  getRecorder()->setFilename(string(buffer));
  
  // Init components and resizers
  addAndMakeVisible(_search = new SearchResultsViewer());
  addAndMakeVisible(_attrs = new AttributeControls());
  addAndMakeVisible(_viewer = new SceneViewer());
  addAndMakeVisible(_exp = new ExplorerPanel());
  _exp->initWithResults(_search->getContainer());
  _search->getContainer()->setExplorerPanel(_exp);

  addAndMakeVisible(_hbar = new StretchableLayoutResizerBar(&_horizResizer, 1, false));
  addAndMakeVisible(_vbar = new StretchableLayoutResizerBar(&_vertResizer, 1, true));
  addAndMakeVisible(_vbar2 = new StretchableLayoutResizerBar(&_viewerSearchResizer, 1, true));

  _searchWorker = new AttributeSearch(_search);

  _horizResizer.setItemLayout(0, -0.05, -0.8, -0.65);
  _horizResizer.setItemLayout(1, 5, 5, 5);
  _horizResizer.setItemLayout(2, -0.05, -0.8, -0.35);

  _vertResizer.setItemLayout(0, -0.05, -1, -0.75);
  _vertResizer.setItemLayout(1, 5, 5, 5);
  _vertResizer.setItemLayout(2, 30, -0.5, -0.25);

  _viewerSearchResizer.setItemLayout(0, -0.05, -0.8, -0.7);
  _viewerSearchResizer.setItemLayout(1, 5, 5, 5);
  _viewerSearchResizer.setItemLayout(2, -0.05, -0.8, -0.3);

  setSize (1600, 900);

  if (getGlobalSettings()->_commandLineArgs.count("preload") > 0) {
    openRig(getGlobalSettings()->_commandLineArgs["preload"]);
  }

	// create log directories if they don't exist
  createLogDirs();

  getGlobalSettings()->_freezeDrawMode = DrawMode::RECT_ADD;

  _autoTimer.setWorker(_searchWorker.get());

  // copy data setup
  _intensCP = shared_ptr<LumiverseFloat>(new LumiverseFloat());
  _colorCP = shared_ptr<LumiverseColor>(new LumiverseColor(BASIC_RGB));
}

MainContentComponent::~MainContentComponent()
{
  getGlobalSettings()->exportSettings();

  _searchWorker->stop();

  getRecorder()->log(SYSTEM, "Interface shutting down.");
}

void MainContentComponent::paint (Graphics& g)
{
  g.fillAll (Colours::black);
}

void MainContentComponent::resized()
{
  // This is called when the MainContentComponent is resized.
  // If you add any child components, this is where you should
  // update their positions.
  auto lbounds = getLocalBounds();

  getStatusBar()->setBounds(lbounds.removeFromBottom(26));

  Component* comps2[] = { nullptr, _vbar, _attrs };
  _vertResizer.layOutComponents(comps2, 3, 0, 0, lbounds.getWidth(), lbounds.getHeight(), false, true);

  Component* comps[] = { _viewer , _hbar, _search };
  _horizResizer.layOutComponents(comps, 3, 0, 0, _vertResizer.getItemCurrentAbsoluteSize(0), lbounds.getHeight(), true, true);

  //Component* comps3[] = { _viewer, _vbar2, _exp };
  //_viewerSearchResizer.layOutComponents(comps3, 3, 0, 0, _vertResizer.getItemCurrentAbsoluteSize(0), _horizResizer.getItemCurrentAbsoluteSize(0), false, true);

}

ApplicationCommandTarget * MainContentComponent::getNextCommandTarget()
{
  return findFirstTargetParentComponent();
}

void MainContentComponent::getAllCommands(Array<CommandID>& commands)
{
  // Add new commands to handle here.
  const CommandID ids[] = {
    command::OPEN, command::REFRESH_PARAMS, command::ARNOLD_RENDER,
    command::SEARCH, command::REFRESH_ATTR, command::SAVE, command::SAVE_AS, command::RECLUSTER,
    command::LOCK_ALL_COLOR,
    command::LOCK_ALL_INTENSITY, command::LOCK_ALL_POSITION, command::UNLOCK_ALL,
    command::LOCK_ALL_AREAS_EXCEPT, command::LOCK_AREA, command::LOCK_SYSTEM, command::LOCK_ALL_SYSTEMS_EXCEPT,
    command::SAVE_RENDER, command::GET_FROM_ARNOLD, command::STOP_SEARCH, command::GET_NEW_RESULTS,
    command::UPDATE_NUM_THREADS, command::SAVE_RESULTS, command::LOAD_RESULTS, command::LOAD_TRACES,
    command::PICK_TRACE, command::OPEN_MASK, command::SAVE_CLUSTERS, command::LOAD_CLUSTERS, command::REFRESH_SETTINGS,
    command::START_AUTO, command::END_AUTO, command::LOCK_ALL_SELECTED,
    command::LOCK_SELECTED_INTENSITY, command::LOCK_SELECTED_COLOR, command::UNLOCK_ALL_SELECTED,
    command::UNLOCK_SELECTED_COLOR, command::UNLOCK_SELECTED_INTENSITY, command::RELOAD_ATTRS, command::LOAD_ATTRS,
    command::RESET_ALL, command::SAVE_IDEAS, command::LOAD_IDEAS, command::DELETE_ALL_PINS,
    command::TOGGLE_SELECT_VIEW, command::INTERFACE_OLD, command::INTERFACE_NEW, command::INTERFACE_ALL,
    command::RESET_TIMER, command::SHOW_PROMPT, command::COPY_DEVICE, command::PASTE_ALL, command::PASTE_COLOR,
    command::PASTE_INTENS, command::SET_TO_FULL, command::SET_TO_OFF, command::SET_TO_WHITE,
    command::SYNC, command::ABOUT, command::SYNC_SELECT
  };

  commands.addArray(ids, numElementsInArray(ids));
}

void MainContentComponent::getCommandInfo(CommandID commandID, ApplicationCommandInfo & result)
{
  // Add new command descriptions here
  switch (commandID) {
  case command::OPEN:
    result.setInfo("Open...", "Opens a Lumiverse file", "File", 0);
    result.addDefaultKeypress('o', ModifierKeys::commandModifier);
    break;
  case command::REFRESH_PARAMS:
    result.setInfo("Refresh Parameter Controls", "Internal: refreshes parameter control panel", "Internal", 0);
    break;
  case command::ARNOLD_RENDER:
    result.setInfo("Render", "Renders the current scene with the current settings", "Render", 0);
    result.addDefaultKeypress('r', ModifierKeys::noModifiers);
    break;
  case command::SETTINGS:
    result.setInfo("Settings...", "Opens the application settings window", "Edit", 0);
    break;
  case command::SEARCH:
    result.setInfo("Search", "Performs an exploratory search with the current attribute constraints", "Explore", 0);
    break;
  case command::REFRESH_ATTR:
    result.setInfo("Refresh Attribute Controls", "Internal: refreshes the attribute control panel", "Internal", 0);
    break;
  case command::SAVE:
    result.setInfo("Save", "Save rig file", "File", 0);
    result.addDefaultKeypress('s', ModifierKeys::commandModifier);
    break;
  case command::SAVE_AS:
    result.setInfo("Save As...", "Save rig file using different name", "File", 0);
    result.addDefaultKeypress('s', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
    break;
  case command::RECLUSTER:
    result.setInfo("Cluster", "Cluster results", "Explore", 0);
		result.addDefaultKeypress('c', ModifierKeys::noModifiers);
    break;
  case command::LOCK_ALL_COLOR:
    result.setInfo("Lock All Color", "Lock all color parameters", "Explore", 0);
    result.addDefaultKeypress('c', ModifierKeys::shiftModifier);
    break;
  case command::LOCK_ALL_INTENSITY:
    result.setInfo("Lock All Intensity", "Lock all intensity parameters", "Explore", 0);
    result.addDefaultKeypress('i', ModifierKeys::shiftModifier);
    break;
  case command::LOCK_ALL_POSITION:
    result.setInfo("Lock All Position", "Lock all position parameters", "Explore", 0);
    result.addDefaultKeypress('p', ModifierKeys::shiftModifier);
    break;
  case command::UNLOCK_ALL:
    result.setInfo("Unlock All Parameters", "Unlock all locked parameters", "Explore", 0);
    result.addDefaultKeypress('u', ModifierKeys::shiftModifier);
    break;
  case command::LOCK_ALL_AREAS_EXCEPT:
    result.setInfo("Lock All Areas Except...", "Locks all areas except the specified area.", "Explore", 0);
    result.addDefaultKeypress('1', ModifierKeys::shiftModifier);
    break;
  case command::LOCK_AREA:
    result.setInfo("Lock Area...", "Locks a single area.", "Explore", 0);
    result.addDefaultKeypress('2', ModifierKeys::shiftModifier);
    break;
  case command::LOCK_ALL_SYSTEMS_EXCEPT:
    result.setInfo("Lock All Systems Except...", "Lock all systems except the specified system", "Explore", 0);
    result.addDefaultKeypress('3', ModifierKeys::shiftModifier);
    break;
  case command::LOCK_SYSTEM:
    result.setInfo("Lock System...", "Lock a single system", "Explore", 0);
    result.addDefaultKeypress('4', ModifierKeys::shiftModifier);
    break;
  case command::SAVE_RENDER:
    result.setInfo("Save Render", "Saves the current render of the rig", "File", 0);
    break;
  case command::GET_FROM_ARNOLD:
    result.setInfo("Get Defaults From Arnold", "Get device default values from the Arnold .ass file", "Edit", 0);
    break;
  case command::STOP_SEARCH:
    result.setInfo("Stop Search", "Stops the current search operation", "Explore", 0);
    break;
  case command::GET_NEW_RESULTS:
    result.setInfo("Gets new results to display", "Internal update operation", "Internal", 0);
    break;
  case command::UPDATE_NUM_THREADS:
    result.setInfo("Update Search Threads", "Updates the search worker to use the specified number of threads", "Internal", 0);
    break;
  case command::SAVE_RESULTS:
    result.setInfo("Save Results...", "Saves the current set of results to a file", "Explore", 0);
    break;
  case command::LOAD_RESULTS:
    result.setInfo("Load Results...", "Loads a previously saved set of results back in to the interface", "Explore", 0);
    break;
  case command::LOAD_TRACES:
    result.setInfo("Load Trace...", "Loads a previously saved exported set of samples for viewing", "Explore", 0);
    break;
  case command::PICK_TRACE:
    result.setInfo("Display Trace", "Displays a loaded trace in the results window", "Explore", 0);
    break;
  case command::OPEN_MASK:
    result.setInfo("Open Mask...", "Opens a mask file", "File", 0);
    break;
	case command::SAVE_CLUSTERS:
		result.setInfo("Save Clusters", "Save the current clusters", "Explore", 0);
		break;
	case command::LOAD_CLUSTERS:
		result.setInfo("Load Clusters", "Load a cluster configuration", "Explore", 0);
		break;
	case command::REFRESH_SETTINGS:
		result.setInfo("Refresh Settings", "Refreshes the settings window", "Internal", 0);
		break;
  case command::START_AUTO:
    result.setInfo("Start Automatic Command", "Internal: Start search based on command line arguments", "Internal", 0);
    break;
  case command::END_AUTO:
    result.setInfo("Stop Automatic Command", "Internal: Runs the final code for the automatic search", "Internal", 0);
    break;
  case command::LOCK_SELECTED_COLOR:
    result.setInfo("Lock Color", "Lock the selected devices' color parameter", "Explore", 0);
    result.addDefaultKeypress('j', ModifierKeys::noModifiers);
    break;
  case command::LOCK_SELECTED_INTENSITY:
    result.setInfo("Lock Intensity", "Lock the selected devices' intensity parameter", "Explore", 0);
    result.addDefaultKeypress('k', ModifierKeys::noModifiers);
    break;
  case command::LOCK_ALL_SELECTED:
    result.setInfo("Lock All", "Lock all parameters in the selected devices", "Explore", 0);
    result.addDefaultKeypress('l', ModifierKeys::noModifiers);
    break;
  case command::UNLOCK_SELECTED_COLOR:
    result.setInfo("Unlock Color", "Unlock the selected devices' color parameter", "Explore", 0);
    result.addDefaultKeypress('j', ModifierKeys::commandModifier);
    break;
  case command::UNLOCK_SELECTED_INTENSITY:
    result.setInfo("Unlock Intensity", "Unlock the selected devices' intensity parameter", "Explore", 0);
    result.addDefaultKeypress('k', ModifierKeys::commandModifier);
    break;
  case command::UNLOCK_ALL_SELECTED:
    result.setInfo("Unlock All", "Unlock all parameters in the selected devices", "Explore", 0);
    result.addDefaultKeypress('l', ModifierKeys::commandModifier);
    break;
  case command::RELOAD_ATTRS:
    result.setInfo("Reload Research Images", "Reloads the image attributes.", "File", 0);
    break;
  case command::LOAD_ATTRS:
    result.setInfo("Load Research Images", "Select a folder to load images from", "File", 0);
    break;
  case command::RESET_ALL:
    result.setInfo("Reset Scene", "Resets the current scene to defaults and clears the search interfaces", "Edit", 0);
    break;
  case command::SAVE_IDEAS:
    result.setInfo("Save Ideas", "Save ideas into a specified folder", "File", 0);
    break;
  case command::LOAD_IDEAS:
    result.setInfo("Load Ideas", "Load ideas from a specified folder", "File", 0);
    break;
  case command::DELETE_ALL_PINS:
    result.setInfo("Delete All Pinned Regions", "Deletes all currently pinned regions on the stage", "Explore", 0);
    break;
  case command::TOGGLE_SELECT_VIEW:
    result.setInfo("Toggle Selection View", "Hide or show the current selection of lights", "Render", 0);
    result.addDefaultKeypress('s', ModifierKeys::noModifiers);
    break;
  case command::INTERFACE_NEW:
    result.setInfo("Switch to Concepts Interface", "Switches to the new experimental interface", "Window", 0);
    break;
  case command::INTERFACE_OLD:
    result.setInfo("Switch to Standard Interface", "Switches to the standard interface", "Window", 0);
    break;
  case command::INTERFACE_ALL:
    result.setInfo("Switch to Debug Interface", "Switches to the debug interface", "Window", 0);
    break;
  case command::RESET_TIMER:
    result.setInfo("Reset Log Timer", "resets the log timer", "Edit", 0);
    break;
  case command::SHOW_PROMPT:
    result.setInfo("Show Prompt", "Displays the prompt for this experiment", "Window", 0);
    break;
  case command::COPY_DEVICE:
    result.setInfo("Copy", "Copies the current device paramters", "Edit", 0);
    result.addDefaultKeypress('c', ModifierKeys::commandModifier);
    break;
  case command::PASTE_ALL:
    result.setInfo("Paste", "Pastes the current device parameters", "Edit", 0);
    result.addDefaultKeypress('v', ModifierKeys::commandModifier);
    break;
  case command::PASTE_INTENS:
    result.setInfo("Paste Intensity", "Pastes the intensity only", "Edit", 0);
    result.addDefaultKeypress('v', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
    break;
  case command::PASTE_COLOR:
    result.setInfo("Paste Color", "Pastes the color only", "Edit", 0);
    result.addDefaultKeypress('v', ModifierKeys::commandModifier | ModifierKeys::altModifier);
    break;
  case command::SET_TO_FULL:
    result.setInfo("Set Intensity to Full", "Sets the intensity of the selected lights to 100%", "Edit", 0);
    result.addDefaultKeypress('f', ModifierKeys::noModifiers);
    break;
  case command::SET_TO_OFF:
    result.setInfo("Set Intensity to Off", "Sets the intensity of selected lights to 0", "Edit", 0);
    result.addDefaultKeypress('g', ModifierKeys::noModifiers);
    break;
  case command::SET_TO_WHITE:
    result.setInfo("Set Color to White", "Sets the color of the light to white", "Edit", 0);
    result.addDefaultKeypress('f', ModifierKeys::commandModifier);
    break;
  case command::SYNC:
    result.setInfo("Sync From Eos", "Get all intensity and color information from an ETC Eos Console", "Edit", 0);
    break;
  case command::ABOUT:
    result.setInfo("About", "About the application", "Window", 0);
    break;
  case command::SYNC_SELECT:
    result.setInfo("Get Eos Selection", "Selects the channel numbers selected by Eos", "Edit", 0);
    result.addDefaultKeypress('e', ModifierKeys::noModifiers);
    break;
  default:
    return;
  }
}

bool MainContentComponent::perform(const InvocationInfo & info)
{
  switch (info.commandID) {
  case command::OPEN:
    openRig();
    break;
  case command::REFRESH_PARAMS:
    refreshParams();
    break;
  case command::ARNOLD_RENDER:
    arnoldRender();
    break;
  case command::SEARCH:
    search();
    break;
  case command::REFRESH_ATTR:
    refreshAttr();
    break;
  case command::SAVE:
    saveRig();
    break;
  case command::SAVE_AS:
    saveAs();
    break;
  case command::RECLUSTER:
    _search->cluster();
    break;
  case command::LOCK_ALL_COLOR:
    lockAllColor();
    break;
  case command::LOCK_ALL_INTENSITY:
    lockAllIntensity();
    break;
  case command::LOCK_ALL_POSITION:
    lockAllPosition();
    break;
  case command::UNLOCK_ALL:
    unlockAll();
    break;
  case command::LOCK_ALL_AREAS_EXCEPT:
    selectBox("area", true, "Lock all areas except");
    break;
  case command::LOCK_ALL_SYSTEMS_EXCEPT:
    selectBox("system", true, "Lock all systems except");
    break;
  case command::LOCK_AREA:
    selectBox("area", false, "Lock one area");
    break;
  case command::LOCK_SYSTEM:
    selectBox("system", false, "Lock one system");
    break;
  case command::SAVE_RENDER:
    saveRender();
    break;
  case command::GET_FROM_ARNOLD:
    getDefaultsFromArnold();
    break;
  case command::STOP_SEARCH:
    stopSearch();
    break;
  case command::GET_NEW_RESULTS:
    showNewResults();
    break;
  case command::UPDATE_NUM_THREADS:
    _searchWorker->stop();
    _searchWorker->reinit();
    break;
  case command::SAVE_RESULTS:
    saveResults();
    break;
  case command::LOAD_RESULTS:
    loadResults();
    break;
  case command::LOAD_TRACES:
    loadTraces();
    break;
  case command::PICK_TRACE:
    pickTrace();
    break;
  case command::OPEN_MASK:
    openMask();
    break;
	case command::SAVE_CLUSTERS:
		saveClusters();
		break;
	case command::LOAD_CLUSTERS:
		loadClusters();
		break;
	case command::REFRESH_SETTINGS:
    _attrs->refreshSettings();
    break;
  case command::START_AUTO:
    startAuto();
    break;
  case command::END_AUTO:
    endAuto();
    break;
  case command::LOCK_SELECTED_INTENSITY:
    _attrs->getParamController()->lockSelected({ "intensity" });
    break;
  case command::LOCK_SELECTED_COLOR:
    _attrs->getParamController()->lockSelected({ "color" });
    break;
  case command::LOCK_ALL_SELECTED:
    _attrs->getParamController()->lockSelected({ "intensity", "color" });
    break;
  case command::UNLOCK_SELECTED_INTENSITY:
    _attrs->getParamController()->unlockSelected({ "intensity" });
    break;
  case command::UNLOCK_SELECTED_COLOR:
    _attrs->getParamController()->unlockSelected({ "color" });
    break;
  case command::UNLOCK_ALL_SELECTED:
    _attrs->getParamController()->unlockSelected({ "intensity", "color" });
    break;
  case command::RELOAD_ATTRS:
    reloadImageAttrs();
    break;
  case command::LOAD_ATTRS:
    loadImageAttrsFromDir();
    break;
  case command::RESET_ALL:
    reset();
    break;
  case command::SAVE_IDEAS:
    saveIdeas();
    break;
  case command::LOAD_IDEAS:
    loadIdeas();
    break;
  case command::DELETE_ALL_PINS:
    deleteAllPins();
    break;
  case command::TOGGLE_SELECT_VIEW:
    toggleSelectView();
    break;
  case command::INTERFACE_NEW:
    _attrs->toggleNewInterface();
    break;
  case command::INTERFACE_OLD:
    _attrs->toggleOldInterface();
    break;
  case command::INTERFACE_ALL:
    _attrs->toggleAllInterface();
    break;
  case command::RESET_TIMER:
    getRecorder()->resetTimer();
    break;
  case command::SHOW_PROMPT:
    showPrompt();
    break;
  case command::COPY_DEVICE:
    copyDeviceParams();
    break;
  case command::PASTE_ALL:
    pasteDeviceParams(set<string>({ "intensity", "color" }));
    break;
  case command::PASTE_COLOR:
    pasteDeviceParams(set<string>({ "color" }));
    break;
  case command::PASTE_INTENS:
    pasteDeviceParams(set<string>({ "intensity" }));
    break;
  case command::SET_TO_FULL:
    setSelectedDevicesTo(1.0f);
    break;
  case command::SET_TO_OFF:
    setSelectedDevicesTo(0);
    break;
  case command::SET_TO_WHITE:
    setSelectedColorTo(1, 1, 1);
    break;
  case command::SYNC:
    sync();
    break;
  case command::ABOUT:
    about();
    break;
  case command::SYNC_SELECT:
    getEosSelection();
    break;
  default:
    return false;
  }

  return true;
}

void MainContentComponent::addHistory()
{
  HistoryPanel* h = _attrs->getHistory();

  // take current scene, package as search result, add to history
  shared_ptr<SearchResult> r = shared_ptr<SearchResult>(new SearchResult());

  Snapshot* current = new Snapshot(getRig());
  r->_scene = snapshotToVector(current);
  r->_snapshot = current;
  //delete current;

  // also save to disk in temporary file in case app crashes
  getRig()->save(File::getCurrentWorkingDirectory().getChildFile("backup.rig.json").getFullPathName().toStdString(), true);
  
  // and save the ideas
  saveIdeas(true);

  r->_sampleNo = h->getHistorySize() + 1;

  SearchResultContainer* c = new SearchResultContainer(r, true);
  c->setImage(_viewer->getRender());

  h->addHistoryItem(c);
  _search->resized();
  _search->repaint();

}

void MainContentComponent::sortCluster()
{
  _search->sort();
}

void MainContentComponent::refreshClusterDisplay()
{
  _attrs->refresh();
}

void MainContentComponent::setThumbImage(Image img)
{
  _viewer->setPreview(img);
  _viewer->repaint();
}

void MainContentComponent::repaintRenderArea()
{
  _viewer->repaint();
}

void MainContentComponent::cleanUpResults(int resultsToKeep)
{
  _searchWorker->stop();
  _search->cleanUp(resultsToKeep);

  if (!_searchWasStopped)
    _searchWorker->startThread();
}

void MainContentComponent::showNewResults()
{
  _search->showNewResults();
}

void MainContentComponent::clearClusters()
{
	_search->clearClusters();
}

void MainContentComponent::transferSelected(Snapshot * source)
{
  StringArray ids = _attrs->getParamController()->getSelectedIds();

  for (String s : ids) {
    string id = s.toStdString();
    Device* sourceDevice = source->getRigData()[id];
    Device* targetDevice = getRig()->getDevice(id);

    for (string param : sourceDevice->getParamNames()) {
      LumiverseTypeUtils::copyByVal(sourceDevice->getParam(param), targetDevice->getParam(param));
    }
  }

  getGlobalSettings()->invalidateCache();
  arnoldRender();
}

void MainContentComponent::transferSelected(Snapshot * source, DeviceSet devices, bool transferPinned)
{
  auto ids = devices.getIds();

  for (string id : ids) {
    Device* sourceDevice = source->getRigData()[id];
    Device* targetDevice = getRig()->getDevice(id);

    for (string param : sourceDevice->getParamNames()) {
      if (transferPinned) {
        LumiverseTypeUtils::copyByVal(sourceDevice->getParam(param), targetDevice->getParam(param));
      }
      else {
        // lots of checks, the pin cache doesn't take user actions after the search in to account, but
        // we probably should here
        if (param == "intensity" && getGlobalSettings()->_intensityPins.count(id) == 0 &&
            !isDeviceParamLocked(id, "intensity")) {
          LumiverseTypeUtils::copyByVal(sourceDevice->getParam(param), targetDevice->getParam(param));
        }
        else if (param == "color" && getGlobalSettings()->_colorPins.count(id) == 0 && !isDeviceParamLocked(id, "color")) {
          LumiverseTypeUtils::copyByVal(sourceDevice->getParam(param), targetDevice->getParam(param));
        }
        else if (param != "intensity" && param != "color") {
          LumiverseTypeUtils::copyByVal(sourceDevice->getParam(param), targetDevice->getParam(param));
        }
      }
    }
  }

  getGlobalSettings()->invalidateCache();
  arnoldRender();
}

void MainContentComponent::redrawResults()
{
  _search->updateImages();
}

void MainContentComponent::populateSystemViews()
{
  _exp->populateSystemViews();
}

bool MainContentComponent::isSearchRunning()
{
  return _searchWorker->isThreadRunning();
}

StringArray MainContentComponent::getSelectedDeviceIds()
{
  return _attrs->getParamController()->getSelectedIds();
}

void MainContentComponent::setColors(vector<Eigen::VectorXd> colors, double intens, vector<float> weights)
{
  _attrs->setColors(colors, intens, weights);
}

void MainContentComponent::reset()
{
  AlertWindow alert("Reset Scene",
    "Reset the interface? This will delete all active ideas, views, and search results from the interface.",
    AlertWindow::AlertIconType::WarningIcon);
  alert.addButton("OK", 1);
  alert.addButton("Cancel", 0);
  int result = alert.runModalLoop();

  if (result == 0) {
    return;
  }

  stopSearch();

  // unlock devices
  unlockAll();

  // reset to open white
  auto devices = getRig()->getAllDevices();
  for (auto id : devices.getIds()) {
    getRig()->getDevice(id)->getIntensity()->setValAsPercent(0.15);

    if (getRig()->getDevice(id)->paramExists("color"))
      getRig()->getDevice(id)->setColorRGBRaw("color", 1, 1, 1);
  }

  // clear search panel
  _search->clearContainer();

  // clear explore panels
  _exp->clear();

  // clear the mask?
  _viewer->clearMask();

  // clear selection mode
  _viewer->hideSelection();

  // delete ideas
  _attrs->deleteIdeas();

  getGlobalSettings()->_ideaMap.clear();
  getGlobalSettings()->_pinnedRegions.clear();

  // re-render
  arnoldRender();
}

void MainContentComponent::createIdea(Image i, String name, IdeaType type)
{
  _attrs->addIdea(i, name, type);
  _viewer->repaint();

  getRecorder()->log(ACTION, "Objective created: " + name.toStdString() + " with type " + String(type).toStdString());
}

void MainContentComponent::debugShowAffectedDevices(juce::Rectangle<float> region)
{
  // attribute controls has a function for this
  map<string, double> sens;
  DeviceSet affected = _attrs->computeAffectedDevices(region, sens);

  String data = "";
  int i = 0;
  for (auto d : sens) {
    if (i % 3 == 0)
      data += "\n";
    data += String(d.first) + ": " + String(d.second) + " ";
    i++;
  }
 
  AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "Light Sensitivity within Selected Region", data);
}

DeviceSet MainContentComponent::computeAffectedDevices(juce::Rectangle<float> region)
{
  return _attrs->computeAffectedDevices(region);
}

void MainContentComponent::setSelectedIds(DeviceSet selection)
{
  _attrs->getParamController()->setSelectedIds(selection);
}

void MainContentComponent::toggleSelectView()
{
  if (_viewer->isInSelectionMode()) {
    _viewer->hideSelection();
  }
  else {
    if (getGlobalSettings()->_freezeDrawMode == DrawMode::RECT_ADD) {
      DeviceSet selected = _attrs->computeAffectedDevices(getGlobalSettings()->_activeIdea);
      _viewer->showSelection(selected);
    }
    else if (getGlobalSettings()->_freezeDrawMode == DrawMode::SELECT_ONLY) {
      _viewer->showSelection();
    }
  }

  _viewer->repaint();
}

void MainContentComponent::createView(DeviceSet selection)
{
  _exp->getContainer()->addContainer(selection);
  _exp->resized();
}

void MainContentComponent::deleteAllObjectives()
{
  _attrs->deleteIdeas();
}

void MainContentComponent::copyDeviceParams()
{
  // retrieve selected devices
  auto ids = getSelectedDeviceIds();

  // get average values
  float intens = 0;
  Eigen::Vector3d color(0,0,0);
  int count = 0;

  for (auto id : ids) {
    intens += getRig()->getDevice(id.toStdString())->getIntensity()->asPercent();
    color += getRig()->getDevice(id.toStdString())->getColor()->getRGB();
    count++;
  }

  _intensCP->setValAsPercent(intens / count);
  color /= count;
  _colorCP->setRGBRaw(color[0], color[1], color[2]);
}

void MainContentComponent::pasteDeviceParams(set<string> paramsToPaste)
{
  auto ids = getSelectedDeviceIds();

  for (auto id : ids) {
    string did = id.toStdString();

    if (paramsToPaste.count("intensity") > 0) {
      LumiverseTypeUtils::copyByVal(_intensCP.get(), getRig()->getDevice(did)->getIntensity());
    }
    if (paramsToPaste.count("color") > 0) {
      LumiverseTypeUtils::copyByVal(_colorCP.get(), getRig()->getDevice(did)->getColor());
    }
  }

  _attrs->refresh();
  _attrs->repaint();
}

void MainContentComponent::setSelectedDevicesTo(float val)
{
  for (auto id : getSelectedDeviceIds()) {
    getRig()->getDevice(id.toStdString())->getIntensity()->setValAsPercent(val);
  }

  _attrs->refresh();
  _attrs->repaint();
}

void MainContentComponent::setSelectedColorTo(float r, float g, float b)
{
  for (auto id : getSelectedDeviceIds()) {
    getRig()->getDevice(id.toStdString())->getColor()->setRGB(r, g, b);
  }

  _attrs->refresh();
  _attrs->repaint();
}

void MainContentComponent::getEosSelection()
{
  // find an OSC patch
  auto patches = getRig()->getPatches();

  for (auto p : patches) {
    // use first found OSC patch
    if (p.second->getType() == "osc") {
      set<int> channels = dynamic_cast<OscPatch*>(p.second)->getEosSelection();

      // put channels into device set.
      DeviceSet ds(getRig());
      for (auto chan : channels) {
        ds = ds.add(chan);
      }

      // set selection in params
      _attrs->getParamController()->setSelectedIds(ds);
      break;
    }
  }
}

void MainContentComponent::openRig() {
  FileChooser fc("Load Show (pick a .rig.json or .playback.json file)", File::getCurrentWorkingDirectory(),
    "*.rig.json;*.playback.json", true);

  if (fc.browseForFileToOpen()) {
    File selected = fc.getResult();
    String fileName = selected.getFullPathName();
    openRig(fileName);
  }
  repaint();
}

void MainContentComponent::openRig(String fname)
{
  stopSearch();
  File selected = File(fname);

  try {
    bool res = false;
    res = getRig()->load(selected.getFullPathName().toStdString());

    if (res) {
      getStatusBar()->setStatusMessage("Loaded file \"" + selected.getFullPathName() + "\" successfully.");
      getRecorder()->log(SYSTEM, "Loaded file \"" + selected.getFullPathName().toStdString() + "\" successfully.");

      getRig()->init();

      _exp->clear();
      getGlobalSettings()->_useFGMask = false;
      getGlobalSettings()->_fgMask = Image();
      loadComponents();

      // try to auto load mask
      // looks for mask.png in same folder as loaded .json
      File mask = selected.getParentDirectory();
      File maskFile = mask.getChildFile("mask.png");
      openMask(maskFile);

      _attrs->reload();
      // Also should delete history and clear any displayed clusters
      _search->clearContainer();
      _attrs->getHistory()->clearAllHistory();
      _showName = selected.getFileName();
      getGlobalSettings()->_showName = _showName;

      getGlobalSettings()->_constraints.clear();
      getGlobalSettings()->generateDefaultConstraints();
      getGlobalSettings()->_showThumbnailImg = false;
      getGlobalSettings()->_freeze = Image(Image::PixelFormat::ARGB, getGlobalSettings()->_renderWidth, getGlobalSettings()->_renderHeight, true);
      getGlobalSettings()->_freeze.clear(getGlobalSettings()->_freeze.getBounds(), Colour(0xff000000));
      getGlobalSettings()->_ideaMap.clear();
      getGlobalSettings()->_rigState = new Snapshot(getRig());

      // compute sensitivity
      computeLightSensitivity();

      getRig()->updateOnce();
      getAppTopLevelWindow()->setName("Lighting Attributes Interface - " + _showName);
    }
    else {
      getStatusBar()->setStatusMessage("Error loading \"" + selected.getFullPathName() + "\"", true);
      getRecorder()->log(SYSTEM, "Failed to load \"" + selected.getFullPathName().toStdString() + "\"");
    }

    arnoldRender(false);
    _attrs->refreshSettings();
  }
  catch (exception e) {
    getStatusBar()->setStatusMessage("Failed to load " + fname, true);

    return;
  }
}

void MainContentComponent::openMask()
{
  FileChooser fc("Load Mask", File::getCurrentWorkingDirectory(),
    "*.png", true);

  if (fc.browseForFileToOpen()) {
    openMask(fc.getResult());
  }

  repaint();
}

void MainContentComponent::openMask(File mask)
{
  FileInputStream in(mask);

  if (in.openedOk()) {
    // load image
    PNGImageFormat pngReader;
    getGlobalSettings()->_fgMask = pngReader.decodeImage(in);

    // assert dimensions are the same, otherwise what exactly is being masked?
    if (getGlobalSettings()->_fgMask.getWidth() != getGlobalSettings()->_renderWidth ||
      getGlobalSettings()->_fgMask.getHeight() != getGlobalSettings()->_renderHeight) {
      getStatusBar()->setStatusMessage("Error loading mask: Dimension mismatch", true, false);
      getGlobalSettings()->_fgMask = Image();
      return;
    }

    getGlobalSettings()->_useFGMask = true;

    _attrs->reload();
    getStatusBar()->setStatusMessage("Loaded mask.");
  }
}

void MainContentComponent::saveRig()
{
  if (_showName == "") {
    // Redirect to save as if we don't have a show name...
    saveAs();
    return;
  }

  File rig = _parentDir.getChildFile(String(_showName));

  bool rigRes = getRig()->save(rig.getFullPathName().toStdString(), true);

  if (rigRes)
    getStatusBar()->setStatusMessage("Wrote show " + _showName + " to disk.");
  else
    getStatusBar()->setStatusMessage("Error saving rig file for show.");
}

void MainContentComponent::saveAs()
{
  FileChooser fc("Save as...",
    File::getCurrentWorkingDirectory(),
    "*.rig.json;*.playback.json",
    true);

  if (fc.browseForFileToSave(true))
  {
    File selected = fc.getResult();
    String fileName = selected.getFileName();
    fileName = fileName.upToFirstOccurrenceOf(".", false, false);

    _parentDir = selected.getParentDirectory();

    File rig = _parentDir.getChildFile(fileName + ".rig.json");

    bool rigRes = getRig()->save(rig.getFullPathName().toStdString(), true);

    if (rigRes) {
      _showName = selected.getFileName();
      getStatusBar()->setStatusMessage("Wrote show " + _showName + " to disk.");
      getAppTopLevelWindow()->setName("Lighting Attributes Interface - " + _showName);
    }
    else {
      getStatusBar()->setStatusMessage("Error saving rig file for show.");
      getRecorder()->log(SYSTEM, "Failed to save show.");
    }
  }
}

void MainContentComponent::saveRender() {
  FileChooser fc("Save Render",
    File::getCurrentWorkingDirectory(),
    "*.png",
    true);

  if (fc.browseForFileToSave(true))
  {
    File selected = fc.getResult();
    String fileName = selected.getFileName();
    fileName = fileName.upToFirstOccurrenceOf(".", false, false);

    _parentDir = selected.getParentDirectory();

    File img = _parentDir.getChildFile(fileName + ".png");
    FileOutputStream os(img);
    PNGImageFormat pngif;

    // TEMP: RENDER DOWNSCALED IMAGE FOR A TEST TARGET
    auto p = getAnimationPatch();
    Image highRes = Image(Image::ARGB, getGlobalSettings()->_renderWidth, getGlobalSettings()->_renderHeight, true);
    uint8* bufptr = Image::BitmapData(highRes, Image::BitmapData::readWrite).getPixelPointer(0, 0);
    p->setDims(getGlobalSettings()->_renderWidth, getGlobalSettings()->_renderHeight);

    getAnimationPatch()->renderSingleFrameToBuffer(getRig()->getDeviceRaw(), bufptr, getGlobalSettings()->_renderWidth, getGlobalSettings()->_renderHeight);

    pngif.writeImageToStream(highRes, os);

    File description = _parentDir.getChildFile(fileName + ".csv");
    FileOutputStream dos(description);

    Snapshot s(getRig());
    dos.writeString(vectorToString(snapshotToVector(&s)));
  }
}

void MainContentComponent::saveResults()
{
  stopSearch();

  FileChooser fc("Save Results",
    File::getCurrentWorkingDirectory(),
    "*.csv",
    true);

  if (fc.browseForFileToSave(true))
  {
    File selected = fc.getResult();
    String fileName = selected.getFullPathName();

    // export results to a csv flie
    _search->saveResults(fileName.toStdString());
  }
}

void MainContentComponent::loadResults()
{
  FileChooser fc("Load Results", File::getCurrentWorkingDirectory(),
    "*.csv", true);

  if (fc.browseForFileToOpen()) {
    File selected = fc.getResult();
    String fileName = selected.getFullPathName();
    _search->loadResults(fileName.toStdString());
  }
}

void MainContentComponent::loadTraces()
{
  FileChooser fc("Load Traces", File::getCurrentWorkingDirectory(),
    "*.csv", true);

  if (fc.browseForFileToOpen()) {
    File selected = fc.getResult();
    String fileName = selected.getFullPathName();
    getGlobalSettings()->loadDiagnosticData(fileName.toStdString());
  }
}

void MainContentComponent::pickTrace()
{
  stopSearch();

  if (getGlobalSettings()->_loadedTraces.size() == 0) {
    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "No Traces Loaded", "Load traces first with the Load Traces menu option");
    return;
  }

  AlertWindow w("Select trace",
    "Select a search trace to display in the results section.",
    AlertWindow::QuestionIcon);

  // get list of available traces
  StringArray comboVals;
  for (auto t : getGlobalSettings()->_loadedTraces) {
    if (t.first == -1)
      continue;
    
    comboVals.add(String(t.first));
  }

  w.addComboBox("Trace ID", comboVals);

  w.addButton("Show", 1, KeyPress(KeyPress::returnKey, 0, 0));
  w.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));

  if (w.runModalLoop() != 0) // is they picked 'show'
  {
    // this is the item they selected
    auto box = w.getComboBoxComponent("Trace ID");
    int selected = comboVals[box->getSelectedItemIndex()].getIntValue();

    // pass off selected trace to the results viewer
    _search->loadTrace(selected);
  }
}

void MainContentComponent::loadComponents()
{
  _attrs->getParamController()->initProperties();
  _attrs->getParamController()->refreshParams();
  auto p = getAnimationPatch();
  getGlobalSettings()->_renderWidth = p->getWidth();
  getGlobalSettings()->_renderHeight = p->getHeight();
  getGlobalSettings()->_stageRenderSamples = p->getSamples();
}

void MainContentComponent::refreshParams()
{
  _attrs->getParamController()->refreshParams();
  _attrs->getParamController()->repaint();
}

void MainContentComponent::refreshAttr()
{
  _attrs->repaint();
}

void MainContentComponent::arnoldRender(bool add)
{
  _viewer->renderScene();
  redrawResults();

  // sometimes add history item
  if (add)
    addHistory();

  // update the saved rig state
  delete getGlobalSettings()->_rigState;
  getGlobalSettings()->_rigState = new Snapshot(getRig());

  getRig()->updateOnce();
}

void MainContentComponent::arnoldRenderNoPopup()
{
  _viewer->renderSceneNoPopup();

  // may want to clean this part up a bit, maybe trigger on mouse up
  addHistory();
}

void MainContentComponent::lockAllColor()
{
  auto devices = getRig()->getAllDevices();
  for (auto d : devices.getDevices()) {
    lockDeviceParam(d->getId(), "color");
  }
  repaint();
}

void MainContentComponent::lockAllIntensity()
{
  auto devices = getRig()->getAllDevices();
  for (auto d : devices.getDevices()) {
    lockDeviceParam(d->getId(), "intensity");
  }
  repaint();
}

void MainContentComponent::lockAllPosition()
{
  auto devices = getRig()->getAllDevices();
  for (auto d : devices.getDevices()) {
    lockDeviceParam(d->getId(), "azimuth");
    lockDeviceParam(d->getId(), "polar");
  }
  repaint();
}

void MainContentComponent::unlockAll()
{
  auto devices = getRig()->getAllDevices();
  for (auto d : devices.getDevices()) {
    for (auto p : d->getParamNames()) {
      unlockDeviceParam(d->getId(), p);
    }
  }
  repaint();
}

void MainContentComponent::lockDevice(Device * d)
{
  string id = d->getId();
  for (auto p : d->getParamNames()) {
    lockDeviceParam(d->getId(), p);
  }
  repaint();
}

void MainContentComponent::getDefaultsFromArnold()
{
  auto arnold = getAnimationPatch();
  //arnold->getPositionFromAss(getRig()->getDeviceRaw());
  arnold->getBeamPropsFromAss(getRig()->getDeviceRaw());
  _attrs->getParamController()->initProperties();
}

void MainContentComponent::saveClusters()
{
	_search->saveClusters();
	getStatusBar()->setStatusMessage("Saved current cluster configuration.");
}

void MainContentComponent::loadClusters()
{
	if (_search->numSavedClusters() <= 0) {
		getStatusBar()->setStatusMessage("Can't load clusters. No clusters have been saved.", true);
		return;
	}

	AlertWindow w("Load Clusters",
		"Select Cluster Set",
		AlertWindow::QuestionIcon);

	// construct list of available clusterings
	StringArray options;
	for (int i = 0; i < _search->numSavedClusters(); i++) {
		options.add(String(i));
	}

	w.addComboBox("Cluster ID", options);

	w.addButton("Load", 1, KeyPress(KeyPress::returnKey, 0, 0));
	w.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));

	if (w.runModalLoop() != 0) // is they picked 'go'
	{
		// this is the item they selected
		auto box = w.getComboBoxComponent("Cluster ID");
		int selected = options[box->getSelectedItemIndex()].getIntValue();
		_search->loadClusters(selected);
		getStatusBar()->setStatusMessage("Loaded cluster set " + String(selected));
	}
}

void MainContentComponent::startAuto()
{
  getStatusBar()->setStatusMessage("Running auto program");

  // assert all required params are present
  auto& args = getGlobalSettings()->_commandLineArgs;
  if (args.count("auto") == 0 || args.count("img-attr") == 0 ||
      args.count("samples") == 0 || args.count("out") == 0) {
    // kill the app otherwise
    JUCEApplicationBase::quit();
  }

  // settings
  getGlobalSettings()->_searchMode = (SearchMode)args["auto"].getIntValue();
  getGlobalSettings()->_maxReturnedScenes = args["samples"].getIntValue();
  getGlobalSettings()->_logRootDir = args["out"].toStdString();
  getGlobalSettings()->_exportTraces = true;
  getGlobalSettings()->_autoRunTraceGraph = false;
  
  if (args.count("timeout") > 0)
    getGlobalSettings()->_autoTimeout = args["timeout"].getIntValue();

  if (args.count("jnd") > 0)
    getGlobalSettings()->_jndThreshold = args["jnd"].getFloatValue();

  if (args.count("evWeight") > 0)
    getGlobalSettings()->_evWeight = args["evWeight"].getFloatValue();

  if (args.count("T") > 0)
    getGlobalSettings()->_T = args["T"].getFloatValue();

  if (args.count("stepSize") > 0)
    getGlobalSettings()->_editStepSize = args["stepSize"].getFloatValue();

  if (args.count("uniformEdits") > 0)
    getGlobalSettings()->_uniformEditWeights = true;

  if (args.count("chainLength") > 0)
    getGlobalSettings()->_startChainLength = args["chainLength"].getIntValue();

  if (args.count("randomInit") > 0) {
    // signals that each thread should do a random initialization of its starting
    // configuration before running the search
  }

  if (args.count("resampleTime") > 0) {
    getGlobalSettings()->_resampleTime = args["resampleTime"].getIntValue();
  }

  if (args.count("resampleThreads") > 0) {
    getGlobalSettings()->_resampleThreads = args["resampleThreads"].getIntValue();
  }

  if (args.count("editMode") > 0) {
    getGlobalSettings()->_editSelectMode = (EditSelectMode)args["editMode"].getIntValue();
  }

  File logFolder;
  if (getGlobalSettings()->_commandLineArgs.count("outputFolderName") > 0) {
    getGlobalSettings()->_logRootDir = logFolder.getCurrentWorkingDirectory().
      getChildFile(String(getGlobalSettings()->_logRootDir + "/" + getGlobalSettings()->_commandLineArgs["outputFolderName"].toStdString())).
      getFullPathName().toStdString();
  }
  else {
    getGlobalSettings()->_logRootDir = logFolder.getCurrentWorkingDirectory().
      getChildFile(String(getGlobalSettings()->_logRootDir) + "/" + String(getGlobalSettings()->_searchMode)).
      getFullPathName().toStdString();
  }

  createLogDirs();

  // replace all attributes with our specific attribute
  _attrs->deleteAllAttributes();
  ImageAttribute* imgAttr;

  if (args["img-attr"] == "auto") {
    // generate image
    // okay to keep things consistent, we'll just take the start scene and do a bunch of random edits
    // to it to maintain some semblance of consistency.
    _searchWorker->setState(new Snapshot(getRig()), _attrs->getActiveAttributes());
    _searchWorker->generateEdits(false);

    default_random_engine gen(std::random_device{}());
    uniform_real_distribution<float> udist(50, 500);
    uniform_real_distribution<float> edist(0, (float)getGlobalSettings()->_edits.size());

    Snapshot* target = new Snapshot(getRig());

    // conveniently edits are just in the globals so
    for (int i = 0; i < (int)udist(gen); i++) {
      int id = (int)edist(gen);
      getGlobalSettings()->_edits[id]->performEdit(target, 0.2); // go nuts
    }

    // create attribute controller
    imgAttr = new ImageAttribute("auto", target);
    _attrs->addAttributeController(imgAttr);
    delete target;
  }
  else {
    // load image
    File img(args["img-attr"]);
    imgAttr = new ImageAttribute(img.getFileNameWithoutExtension().toStdString(), args["img-attr"].toStdString());
    _attrs->addAttributeController(imgAttr);
  }

  // set attribute status
  if (args.count("less") > 0)
    imgAttr->setStatus(A_LESS);
  else
    imgAttr->setStatus(A_MORE);

  // search start timer init here for timer
  getGlobalSettings()->_searchStartTime = chrono::high_resolution_clock::now();
  search();

  // search starts. when finished, aplication will quit after all data is gathered.
  _autoTimer.startTimer(100);
}

void MainContentComponent::endAuto()
{
  getStatusBar()->setStatusMessage("Writing search data...");

  // back in the main thread
  // we want to pull out all the relevant data and put it in a folder in the log root dir
  // first, create the folder
  File resultsFolder = File::getCurrentWorkingDirectory().getChildFile(
    String(getGlobalSettings()->_logRootDir) + "/" + String(getGlobalSettings()->_sessionName));
  string filename = resultsFolder.getChildFile("search.meta").getFullPathName().toStdString();

  if (!resultsFolder.exists()) {
    resultsFolder.createDirectory();
  }

  // write metadata file
  ofstream file;
  file.open(filename, ios::trunc);
  file << getGlobalSettings()->_sessionSearchSettings;

  time_t now = chrono::system_clock::to_time_t(getGlobalSettings()->_searchAbsStartTime);
  Array<shared_ptr<SearchResultContainer> > results = _search->getAllResults();
  ImageAttribute* attr = (ImageAttribute*)(_attrs->getActiveAttributes().begin()->second);

  file << "Search Start Time: " << ctime(&now);
  file << "Search Duration: " << chrono::duration<float>(getGlobalSettings()->_searchEndTime - getGlobalSettings()->_searchStartTime).count() << "s\n\n";

  file << "Search Mode: " << getGlobalSettings()->_searchMode << "\n";
  file << "Starting Edit Depth: " << getGlobalSettings()->_startChainLength << "\n";
  file << "Edit Step Size: " << getGlobalSettings()->_editStepSize << "\n";
  file << "Max MCMC Iterations: " << getGlobalSettings()->_maxMCMCIters << "\n";
  file << "Max Displayed Results: " << getGlobalSettings()->_maxReturnedScenes << "\n";
  file << "Required Distance from Other Results: " << getGlobalSettings()->_jndThreshold << "\n";
  file << "Search Failure Limit: " << getGlobalSettings()->_searchFailureLimit << "\n";
  file << "Search Threads: " << getGlobalSettings()->_searchThreads << "\n";
  file << "Temperature: " << getGlobalSettings()->_T << "\n";

  file << "\nEdit Weight Table\n";

  for (auto e : getGlobalSettings()->_globalEditWeights) {
    file << e.first << " : " << e.second->_name << "\n";
  }

  file.close();

  // save the target image
  File img = resultsFolder.getChildFile("target.png");
  FileOutputStream os(img);
  PNGImageFormat pngif;

  // get the attribute image (the original)
  // the attribute should be the only one in the controls
  Image target = attr->getOriginalImage();
  pngif.writeImageToStream(target, os);

  // save results
  // for all results in the results container we want to export:
  // - thread, sample id, time from start it was generated, attr value
  //   avg lab distance to target, edit history, diameter, variance, feature vector in that order as a csv file
  filename = resultsFolder.getChildFile("results.csv").getFullPathName().toStdString();
  file.open(filename, ios::trunc);

  // collect some data about the results while we're at it
  double avgAttrVal = 0;
  double minAttrVal = DBL_MAX;
  double avgLab = 0;
  double minLab = DBL_MAX;

  // capture variance and diameter at specific points in time
  map<double, double> diamAtN;
  map<double, double> varAtN;
  diamAtN[0] = 0;
  varAtN[0] = 0;

  // compute variance from mean position and diameter of cluster over time
  Eigen::VectorXd runningTotal, variance, diameter;
  Eigen::MatrixXd distances;
  runningTotal.resize(results[0]->getSearchResult()->_scene.size());
  runningTotal.setZero();
  variance.resize(results.size());
  variance.setZero();
  diameter.resize(results.size());
  distances.resize(results.size(), results.size());
  distances.setZero();

  // compute distance matrix and compute related values
  for (int r = 0; r < results.size(); r++) {
    for (int c = r; c < results.size(); c++) {
      distances(r, c) = (results[r]->getSearchResult()->_scene - results[c]->getSearchResult()->_scene).norm();
    }

    runningTotal += results[r]->getSearchResult()->_scene;
    Eigen::VectorXd mean = runningTotal / (r + 1);

    for (int i = 0; i < r; i++) {
      variance[r] += (results[i]->getSearchResult()->_scene - mean).squaredNorm();
    }
    variance[r] /= (r + 1);

    // diameter is maximum pairwise distance
    diameter[r] = 0;
    for (int i = 0; i < r; i++) {
      for (int j = i; j < r; j++) {
        if (distances(i, j) > diameter[r])
          diameter[r] = distances(i, j);
      }
    }
  }

  Snapshot* s = new Snapshot(getRig());
  file << "-1,0,0," << getGlobalSettings()->_samples[-1][0]._f << "," << attr->avgLabDistance(s) << ",START,0,0,0\n";
  delete s;

  // export results here
  int index = 0; // man im lazy
  double threshold = 25;
  map<double, int> sortedVals;
  for (auto r : results) {
    // creation time
    float timeSinceStart = chrono::duration<float>(r->getSearchResult()->_creationTime - getGlobalSettings()->_searchStartTime).count();
    
    // lab distance to target
    s = vectorToSnapshot(r->getSearchResult()->_scene);
    double labDist = attr->avgLabDistance(s);
    delete s;

    file << r->getSearchResult()->_extraData["Thread"] << "," << r->getSearchResult()->_extraData["Sample"] << ",";
    file << timeSinceStart << "," << r->getSearchResult()->_objFuncVal << "," << labDist << ",";

    string editHist = "";
    for (int i = 0; i < r->getSearchResult()->_editHistory.size(); i++) {
      editHist = editHist + r->getSearchResult()->_editHistory[i]->_name + " -> ";
    }

    file << editHist << "END,";
    file << diameter[index] << "," << variance[index] << ",";

    // feature vector (scene vector)
    for (int i = 0; i < r->getSearchResult()->_scene.size(); i++) {
      file << r->getSearchResult()->_scene[i];
      if (i <= (r->getSearchResult()->_scene.size() - 2)) {
        file << ",";
      }
      else {
        file << "\n";
      }
    }

    // stat collection
    avgAttrVal += r->getSearchResult()->_objFuncVal;
    avgLab += labDist;

    if (r->getSearchResult()->_objFuncVal < minAttrVal)
      minAttrVal = r->getSearchResult()->_objFuncVal;

    if (labDist < minLab)
      minLab = labDist;

    if (timeSinceStart > threshold) {
      diamAtN[threshold] = diameter[index];
      varAtN[threshold] = variance[index];
      threshold += 25;
    }

    sortedVals[r->getSearchResult()->_objFuncVal] = index;
    index++;
  }

  // export the top 10 results

  int i = 0;
  for (auto it = sortedVals.begin(); it != sortedVals.end(); it++) {
    if (i > 10)
      break;

    s = vectorToSnapshot(results[it->second]->getSearchResult()->_scene);
    auto p = getAnimationPatch();

    // with caching we can render at full and then scale down
    Image render = Image(Image::ARGB, getGlobalSettings()->_renderWidth, getGlobalSettings()->_renderHeight, true);
    uint8* bufptr = Image::BitmapData(render, Image::BitmapData::readWrite).getPixelPointer(0, 0);
    p->setDims(getGlobalSettings()->_renderWidth, getGlobalSettings()->_renderHeight);

    getAnimationPatch()->renderSingleFrameToBuffer(s->getDevices(), bufptr, render.getWidth(), render.getHeight());
    File imgf = resultsFolder.getChildFile(String(i) + "-" + String(results[it->second]->getSearchResult()->_sampleNo) + ".png");
    FileOutputStream os2(imgf);

    // get the attribute image (the original)
    // the attribute should be the only one in the controls
    pngif.writeImageToStream(render, os2);
    i++;
  }

  file.close();

  filename = resultsFolder.getChildFile("stats.csv").getFullPathName().toStdString();
  file.open(filename, ios::trunc);

  file << "Start Attribute Value," << getGlobalSettings()->_samples[-1][0]._f << "\n";
  file << "Average Lab," << avgLab / results.size() << "\n";
  file << "Average Attr," << avgAttrVal / results.size() << "\n";
  file << "Min Lab," << minLab << "\n";
  file << "Min Attr," << minAttrVal << "\n";
  file << "Min To N Lab\n";

  for (auto kvp : diamAtN) {
    file << kvp.first << "," << kvp.second << "," << varAtN[kvp.first] << "\n";
  }

  file.close();

  // we'll also just straight up save the results in case we want to look at them in viewer
  // at some point in the future
  _search->saveResults(resultsFolder.getChildFile("raw_results.csv").getFullPathName().toStdString());

  // close the app we're done with this run
  JUCEApplicationBase::quit();
}

void MainContentComponent::createLogDirs()
{
	File clusterFolder;
	clusterFolder = clusterFolder.getCurrentWorkingDirectory().getChildFile(String(getGlobalSettings()->_logRootDir + "/clusters/"));
	if (!clusterFolder.exists()) {
		clusterFolder.createDirectory();
	}

	File logFolder;
	logFolder = logFolder.getCurrentWorkingDirectory().getChildFile(String(getGlobalSettings()->_logRootDir + "/traces/"));
	if (!logFolder.exists()) {
		logFolder.createDirectory();
	}
}

void MainContentComponent::reloadImageAttrs()
{
  _attrs->reload();
  resized();
  repaint();
}

void MainContentComponent::loadImageAttrsFromDir()
{
  FileChooser fc("Choose Image Attributes Folder", File::getCurrentWorkingDirectory());

  if (fc.browseForDirectory())
  {
    getGlobalSettings()->_imageAttrLoc = fc.getResult();
    reloadImageAttrs();
  }
}

void MainContentComponent::saveIdeas(bool backup)
{
  if (backup) {
    _attrs->saveIdeas(File::getCurrentWorkingDirectory().getChildFile("idea_backup"));
  }
  else {
    FileChooser fc("Select Folder to Save Ideas",
      File::getCurrentWorkingDirectory(),
      "", true);

    if (fc.browseForDirectory())
    {
      File selected = fc.getResult();

      if (selected.getNumberOfChildFiles(File::TypesOfFileToFind::findFilesAndDirectories) > 0) {
        // warn user
        AlertWindow alert("Existing Files May Be Overwritten",
          "Files contained within the selected directory may be overwirrten or deleted by saving the current ideas. Is this OK?",
          AlertWindow::AlertIconType::WarningIcon);
        alert.addButton("OK", 1);
        alert.addButton("Cancel", 0);
        int result = alert.runModalLoop();

        if (result == 0) {
          return;
        }
      }

      _attrs->saveIdeas(selected);
    }
  }
}

void MainContentComponent::loadIdeas()
{
  FileChooser fc("Select Folder Containing Ideas",
    File::getCurrentWorkingDirectory(),
    "", true);

  if (fc.browseForDirectory())
  {
    File selected = fc.getResult();

    // data json file must exist for load to proceed
    if (selected.getChildFile("data.ilib").exists()) {
      _attrs->loadIdeas(selected);
    }
    else {
      AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Data file not found", "Cannot find required data.ilib file in selected directory. Check that the file exists in the specified folder and try again.", "OK");
    }
  }
}

void MainContentComponent::deleteAllPins()
{
  getGlobalSettings()->_pinnedRegions.clear();
  _viewer->repaint();
}

void MainContentComponent::selectBox(string metadataKey, bool inv, string title)
{
#if JUCE_MODAL_LOOPS_PERMITTED
  AlertWindow w(title,
    "",
    AlertWindow::QuestionIcon);

  // get list of metadata values
  auto vals = getRig()->getMetadataValues(metadataKey);
  StringArray comboVals;
  for (auto v : vals) {
    comboVals.add(v);
  }

  w.addComboBox("metadataKey", comboVals);

  w.addButton("Go", 1, KeyPress(KeyPress::returnKey, 0, 0));
  w.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));

  if (w.runModalLoop() != 0) // is they picked 'go'
  {
    // this is the item they selected
    auto box = w.getComboBoxComponent("metadataKey");
    string selected = comboVals[box->getSelectedItemIndex()].toStdString();

    string query = "$" + metadataKey + (inv ? "!" : "") + "=" + selected;
    DeviceSet affectedDevices = getRig()->select(query);
    auto devices = affectedDevices.getDevices();
    for (auto& d : devices) {
      lockDevice(d);
    }
  }
#endif
}

void MainContentComponent::showPrompt()
{
  Device* d = getRig()->getDevice("par_H1_2");

  if (d == nullptr) {
    getStatusBar()->setStatusMessage("No prompt found", false, true);
  }
  else {
    string prompt = d->getMetadata("prompt");
    AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "Prompt", prompt);
  }
}

void MainContentComponent::sync()
{
  (new SyncBackgroundThread())->runThread();
}

void MainContentComponent::about()
{
  String aboutStr = "Application version " + String(ProjectInfo::versionString) + "\n";
  aboutStr += "(c) Evan Shimizu 2015-17\nThird-party libraries used by this project include Lumiverse, JUCE, OpenEXR, zlib, oscpack, clp, and others. Contact the author of this program for full license details.";
  AlertWindow::showMessageBox(AlertWindow::AlertIconType::NoIcon, "About", aboutStr);
}

void MainContentComponent::search()
{
  stopSearch();
  _search->clearContainer();
  getGlobalSettings()->clearEdits();
  _exp->getContainer()->empty();

  if (getGlobalSettings()->_searchMode == GIBBS_SAMPLING) {
    GibbsSchedule* state = _attrs->getGibbsSchedule();
    state->addConstraints(_attrs->getConstraintData());
    state->init();
    _attrs->updateSortMenu();

    _searchWorker->setState(new Snapshot(getRig()), state);
    _searchWorker->startThread(9);
    _searchWasStopped = false;
  }
  else {
    // Do not start search if no attributes are selected
    if (_attrs->getActiveAttributes().size() == 0) {
      getStatusBar()->setStatusMessage("ERROR: Search cannot start if all attributes are set to Ignore.", true);
      return;
    }

    _attrs->lockAttributeModes();
    _searchWorker->setState(new Snapshot(getRig()), _attrs->getActiveAttributes());
    _searchWorker->startThread(9);
    _searchWasStopped = false;
  }

  getStatusBar()->setStatusMessage("Attribute Search Started");
  getRecorder()->log(ACTION, "Exploratory search started.");
}

void MainContentComponent::stopSearch()
{
  getStatusBar()->setStatusMessage("Stopping current search operation...");
  _searchWorker->stop();
  _search->showNewResults();
  _attrs->unlockAttributeModes();
  getStatusBar()->setStatusMessage("Search stopped.");

  // calculate per-thread stats, ignore thread id -1
  float avgSamples = 0;
  int count = 0;
  int totalSamples = 0;
  for (auto threadData : getGlobalSettings()->_timings) {
    if (threadData.first < 0)
      continue;

    string logMsg = "Thread: " + String(threadData.first).toStdString() + "\n";
    float avg = (threadData.second._sampleTime / threadData.second._numSamples) * 1000;
    avgSamples += avg;

    logMsg += "Avg Sample Time: " + String(avg).toStdString() + "ms\n";
    logMsg += "Avg Sample Render Time: " + String((threadData.second._sampleRenderTime / threadData.second._numSamples) * 1000).toStdString() + "ms\n";
    logMsg += "Avg Sample Eval Time: " + String((threadData.second._sampleEvalTime / threadData.second._numSamples) * 1000).toStdString() + "ms\n";
    logMsg += "Avg Result Time: " + String((threadData.second._addResultTime / threadData.second._numResults) * 1000).toStdString() + "ms\n";
    logMsg += "Avg Result Render Time: " + String((threadData.second._addResultRenderTime / threadData.second._numResults) * 1000).toStdString() + "ms\n";
    logMsg += "Avg Result Eval Time: " + String((threadData.second._addResultEvalTime / threadData.second._numResults) * 1000).toStdString() + "ms\n";

    getRecorder()->log(SYSTEM, logMsg);
    count++;
    totalSamples += threadData.second._numSamples;
  }

  avgSamples /= count;
  float sps = totalSamples / chrono::duration<float>(getGlobalSettings()->_searchEndTime - getGlobalSettings()->_searchStartTime).count();
  String status = "Avg. Sample Run Time: " + String(avgSamples) + "ms, Samples Per Second: " + String(sps);
  getStatusBar()->setStatusMessage(status);
  getRecorder()->log(SYSTEM, status.toStdString());

  if (getGlobalSettings()->_exportTraces) {
    getGlobalSettings()->dumpDiagnosticData();
  }

  _searchWasStopped = true;
}

MainContentComponent::AutoTimer::AutoTimer()
{
}

MainContentComponent::AutoTimer::~AutoTimer()
{
}

void MainContentComponent::AutoTimer::setWorker(AttributeSearch* worker) {
  _worker = worker;
}

void MainContentComponent::AutoTimer::timerCallback()
{
  // check timeout conditions here
  chrono::time_point<chrono::high_resolution_clock> now = chrono::high_resolution_clock::now();
  double elapsed = chrono::duration<float>(now - getGlobalSettings()->_searchStartTime).count();

  if (!_worker->isThreadRunning() || elapsed > getGlobalSettings()->_autoTimeout * 60) {
    // run exit code.
    stopTimer();

    // reach in to main component directly in case app doesn't have focus
    MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
    mc->stopSearch();
    mc->endAuto();
  }
}

MainContentComponent::SyncBackgroundThread::SyncBackgroundThread() :
  ThreadWithProgressWindow("Syncing from Eos", true, false)
{
  setProgress(-1);
}

MainContentComponent::SyncBackgroundThread::~SyncBackgroundThread()
{
}

void MainContentComponent::SyncBackgroundThread::run()
{
  for (auto p : getRig()->getPatches()) {
    OscPatch* osc = dynamic_cast<OscPatch*>(p.second);
    if (osc != nullptr) {
      // we got an osc patch lets sync
      bool res = osc->sync(getRig()->getDeviceRaw());

      // refresh params and render
      getApplicationCommandManager()->invokeDirectly(command::REFRESH_PARAMS, true);
      getApplicationCommandManager()->invokeDirectly(command::ARNOLD_RENDER, true);

      if (res) {
        getStatusBar()->setStatusMessage("Sync Complete");
      }
      else {
        getStatusBar()->setStatusMessage("Sync Failed", true, false);
      }

      // return, if there are two osc patches (?) ignore the second
      return;
    }
  }
}

void MainContentComponent::SyncBackgroundThread::threadComplete(bool userPressedCancel)
{
  delete this;
}
