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

//==============================================================================
MainContentComponent::MainContentComponent()
{
  Lumiverse::Logger::setLogLevel(INFO);
  addAndMakeVisible(getStatusBar());
  
  // Init components and resizers
  addAndMakeVisible(_search = new SearchResultsViewer());
  addAndMakeVisible(_params = new ParamControls());
  addAndMakeVisible(_attrs = new AttributeControls());
  addAndMakeVisible(_viewer = new SceneViewer());

  addAndMakeVisible(_hbar = new StretchableLayoutResizerBar(&_horizResizer, 1, false));
  addAndMakeVisible(_vbar1 = new StretchableLayoutResizerBar(&_vertResizer, 1, true));
  addAndMakeVisible(_vbar2 = new StretchableLayoutResizerBar(&_vertResizer, 3, true));

  _searchWorker = new AttributeSearch(_search);

  _horizResizer.setItemLayout(0, -0.2, -0.8, -0.5);
  _horizResizer.setItemLayout(1, 5, 5, 5);
  _horizResizer.setItemLayout(2, -0.2, -0.8, -0.5);

  _vertResizer.setItemLayout(0, -0.2, -0.5, -0.25);
  _vertResizer.setItemLayout(1, 5, 5, 5);
  _vertResizer.setItemLayout(2, -0.2, -0.8, -0.5);
  _vertResizer.setItemLayout(3, 5, 5, 5);
  _vertResizer.setItemLayout(4, -0.2, -0.5, -0.25);

  setSize (1280, 720);

  if (getGlobalSettings()->_commandLineArgs.count("preload") > 0) {
    openRig(getGlobalSettings()->_commandLineArgs["preload"]);
  }
}

MainContentComponent::~MainContentComponent()
{
  if (_settingsWindow != nullptr)
    delete _settingsWindow;

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

  Component* comps[] = { nullptr, _hbar, _search };
  _horizResizer.layOutComponents(comps, 3, 0, 0, lbounds.getWidth(), lbounds.getHeight(), true, true);

  Component* comps2[] = { _params, _vbar1, _viewer, _vbar2, _attrs };
  _vertResizer.layOutComponents(comps2, 5, 0, 0, lbounds.getWidth(), _horizResizer.getItemCurrentAbsoluteSize(0), false, true);
}

ApplicationCommandTarget * MainContentComponent::getNextCommandTarget()
{
  return findFirstTargetParentComponent();
}

void MainContentComponent::getAllCommands(Array<CommandID>& commands)
{
  // Add new commands to handle here.
  const CommandID ids[] = {
    command::OPEN, command::REFRESH_PARAMS, command::ARNOLD_RENDER, command::SETTINGS,
    command::SEARCH, command::REFRESH_ATTR, command::SAVE, command::SAVE_AS, command::RECLUSTER,
    command::UNDO, command::REDO, command::LOCK_ALL_COLOR,
    command::LOCK_ALL_INTENSITY, command::LOCK_ALL_POSITION, command::UNLOCK_ALL,
    command::LOCK_ALL_AREAS_EXCEPT, command::LOCK_AREA, command::LOCK_SYSTEM, command::LOCK_ALL_SYSTEMS_EXCEPT,
    command::SAVE_RENDER, command::GET_FROM_ARNOLD, command::STOP_SEARCH, command::GET_NEW_RESULTS,
    command::UPDATE_NUM_THREADS, command::SAVE_RESULTS, command::LOAD_RESULTS, command::LOAD_TRACES,
    command::PICK_TRACE, command::OPEN_MASK, command::SAVE_CLUSTERS, command::LOAD_CLUSTERS, command::REFRESH_SETTINGS
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
    result.addDefaultKeypress('f', ModifierKeys::commandModifier);
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
  case command::UNDO:
    result.setInfo("Undo", "Undo", "Edit", 0);
    result.addDefaultKeypress('z', ModifierKeys::commandModifier);
    break;
  case command::REDO:
    result.setInfo("Redo", "Redo", "Edit", 0);
    result.addDefaultKeypress('y', ModifierKeys::commandModifier);
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
  case command::SETTINGS:
    openSettings();
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
  case command::UNDO:
    undo();
    break;
  case command::REDO:
    redo();
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
	{
		if (_settingsWindow != nullptr) {
			_settingsWindow->refresh();
		}
	}
  default:
    return false;
  }

  return true;
}

void MainContentComponent::addHistory()
{
  HistoryPanel* h = _search->getHistory();
  h->clearRedo();
  h->addHistoryItem(new HistoryEntry(new Snapshot(getRig()), "History", _viewer->getRender()));
  _search->resized();
  _search->repaint();
}

void MainContentComponent::undo()
{
  HistoryPanel* h = _search->getHistory();
  HistoryEntry* e = h->removeHistoryItem();
  if (e != nullptr) {
    e->_sceneState->loadRig(getRig());
    _viewer->setRender(e->_thumb);
    refreshAttr();
    refreshParams();

    h->addRedoItem(e);
    getRecorder()->log(ACTION, "Undo Called");
  }
}

void MainContentComponent::redo()
{
  HistoryPanel* h = _search->getHistory();
  HistoryEntry* e = h->removeRedoItem();
  if (e != nullptr) {
    e->_sceneState->loadRig(getRig());
    _viewer->setRender(e->_thumb);

    refreshAttr();
    refreshParams();

    h->addHistoryItem(e);
    getRecorder()->log(ACTION, "Redo Called");
  }
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
  File selected = File(fname);

  bool res = getRig()->load(selected.getFullPathName().toStdString());

  if (res) {
    getStatusBar()->setStatusMessage("Loaded file \"" + selected.getFullPathName() + "\" successfully.");
    getRecorder()->log(SYSTEM, "Loaded file \"" + selected.getFullPathName().toStdString() + "\" successfully.");

    getRig()->init();

    loadComponents();
    _attrs->reload();
    // Also should delete history and clear any displayed clusters

    _showName = selected.getFileName();
    getAppTopLevelWindow()->setName("Lighting Attributes Interface - " + _showName);
  }
  else {
    getStatusBar()->setStatusMessage("Error loading \"" + selected.getFullPathName() + "\"");
    getRecorder()->log(SYSTEM, "Failed to load \"" + selected.getFullPathName().toStdString() + "\"");
  }

  arnoldRender();
}

void MainContentComponent::openMask()
{
  FileChooser fc("Load Mask", File::getCurrentWorkingDirectory(),
    "*.png", true);

  if (fc.browseForFileToOpen()) {
    File selected = fc.getResult();
    FileInputStream in(selected);

    if (in.openedOk()) {
      // load image
      PNGImageFormat pngReader;
      getGlobalSettings()->_fgMask = pngReader.decodeImage(in);
      getGlobalSettings()->_useFGMask = true;

      getStatusBar()->setStatusMessage("Loaded mask.");
    }
  }
  repaint();
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
    pngif.writeImageToStream(_viewer->getRender(), os);

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
  _params->initProperties();
  auto p = getAnimationPatch();
  getGlobalSettings()->_renderWidth = p->getWidth();
  getGlobalSettings()->_renderHeight = p->getHeight();
  getGlobalSettings()->_stageRenderSamples = p->getSamples();
}

void MainContentComponent::refreshParams()
{
  _params->refreshParams();
  _params->repaint();
}

void MainContentComponent::refreshAttr()
{
  _attrs->repaint();
}

void MainContentComponent::arnoldRender()
{
  _viewer->renderScene();
  // always add history element after a render
  addHistory();
}

void MainContentComponent::arnoldRenderNoPopup()
{
  _viewer->renderSceneNoPopup();

  // may want to clean this part up a bit, maybe trigger on mouse up
  addHistory();
}

void MainContentComponent::openSettings()
{
  if (_settingsWindow != nullptr)
    return;

  _settingsWindow = new SettingsWindow();
  juce::Rectangle<int> area(50, 50, 600, 400);

  _settingsWindow->setBounds(area);

  _settingsWindow->setResizable(true, false);
  _settingsWindow->setUsingNativeTitleBar(true);
  _settingsWindow->setVisible(true);
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
  _params->initProperties();
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

void MainContentComponent::search()
{
  stopSearch();
  _search->clearContainer();
  getGlobalSettings()->clearEdits();

  // Do not start search if no attributes are selected
  if (_attrs->getActiveAttributes().size() == 0) {
    getStatusBar()->setStatusMessage("ERROR: Search cannot start if all attributes are set to Ignore.", true);
    return;
  }

  _searchWorker->setState(new Snapshot(getRig()), _attrs->getActiveAttributes());
  _searchWorker->startThread(9);
  _searchWasStopped = false;

  getStatusBar()->setStatusMessage("Attribute Search Started");
  getRecorder()->log(ACTION, "Exploratory search started.");
}

void MainContentComponent::stopSearch()
{
  getStatusBar()->setStatusMessage("Stopping current search operation...");
  _searchWorker->stop();
  getStatusBar()->setStatusMessage("Search stopped.");

  if (getGlobalSettings()->_exportTraces) {
    getGlobalSettings()->dumpDiagnosticData();
  }

  _searchWasStopped = true;
}