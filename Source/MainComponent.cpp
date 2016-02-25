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
  addAndMakeVisible(getStatusBar());
  
  // Init components and resizers
  addAndMakeVisible(_search = new SearchResultsViewer());
  addAndMakeVisible(_params = new ParamControls());
  addAndMakeVisible(_attrs = new AttributeControls());
  addAndMakeVisible(_viewer = new SceneViewer());

  addAndMakeVisible(_hbar = new StretchableLayoutResizerBar(&_horizResizer, 1, false));
  addAndMakeVisible(_vbar1 = new StretchableLayoutResizerBar(&_vertResizer, 1, true));
  addAndMakeVisible(_vbar2 = new StretchableLayoutResizerBar(&_vertResizer, 3, true));

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
  if (_clusterWindow != nullptr)
    delete _clusterWindow;

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
    command::VIEW_CLUSTERS, command::UNDO, command::REDO, command::LOCK_ALL_COLOR,
    command::LOCK_ALL_INTENSITY, command::LOCK_ALL_POSITION, command::UNLOCK_ALL,
    command::LOCK_KEY, command::LOCK_FILL, command::LOCK_RIM
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
    result.setInfo("Recluster", "Recluster results based on user settings", "Explore", 0);
    break;
  case command::VIEW_CLUSTERS:
    result.setInfo("Show Clusters", "Open window showing all clusters at once", "Explore", 0);
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
  case command::LOCK_KEY:
    result.setInfo("Lock Key", "Lock all parameters on the key light", "Explore", 0);
    result.addDefaultKeypress('1', ModifierKeys::shiftModifier);
    break;
  case command::LOCK_FILL:
    result.setInfo("Lock Fill", "Lock all parameters on the fill light", "Explore", 0);
    result.addDefaultKeypress('2', ModifierKeys::shiftModifier);
    break;
  case command::LOCK_RIM:
    result.setInfo("Lock Rim", "Lock all parameters on the rim light", "Explore", 0);
    result.addDefaultKeypress('3', ModifierKeys::shiftModifier);
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
    _search->redisplay();
    break;
  case command::VIEW_CLUSTERS:
    openClusters();
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
  case command::LOCK_KEY:
  {
    Snapshot* s = new Snapshot(getRig(), nullptr);
    lockDevice(getSpecifiedDevice(L_KEY, s));
    delete s;
    break;
  }
  case command::LOCK_FILL:
  {
    Snapshot* s = new Snapshot(getRig(), nullptr);
    lockDevice(getSpecifiedDevice(L_FILL, s));
    delete s;
    break;
  }
  case command::LOCK_RIM:
  {
    Snapshot* s = new Snapshot(getRig(), nullptr);
    lockDevice(getSpecifiedDevice(L_RIM, s));
    delete s;
    break;
  }
  default:
    return false;
  }

  return true;
}

void MainContentComponent::setBottomSearchComponent(Component* c, Component* source)
{
  _search->setBotComponent(c, source);
}

void MainContentComponent::addHistory()
{
  HistoryPanel* h = _search->getHistory();
  h->clearRedo();
  h->addHistoryItem(new HistoryEntry(new Snapshot(getRig(), nullptr), "History", _viewer->getRender()));
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

    h->addHistoryItem(e);
    getRecorder()->log(ACTION, "Redo Called");
  }
}

void MainContentComponent::sortCluster()
{
  _search->sortDisplayedCluster();
}

void MainContentComponent::refreshClusterDisplay()
{
  _attrs->refresh();
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
  String fileName = fname.upToFirstOccurrenceOf(".", false, false);

  _parentDir = selected.getParentDirectory();

  File rig = _parentDir.getChildFile(fileName + ".rig.json");
  File playback = _parentDir.getChildFile(fileName + ".playback.json");

  bool res = getRig()->load(rig.getFullPathName().toStdString());

  if (res) {
    getStatusBar()->setStatusMessage("Loaded file \"" + rig.getFullPathName() + "\" successfully.");
    getRecorder()->log(SYSTEM, "Loaded file \"" + rig.getFullPathName().toStdString() + "\" successfully.");

    getRig()->init();

    loadComponents();

    _showName = rig.getFileName();
    getAppTopLevelWindow()->setName("Lighting Attributes Interface - " + _showName);
  }
  else {
    getStatusBar()->setStatusMessage("Error loading \"" + rig.getFullPathName() + "\"");
    getRecorder()->log(SYSTEM, "Failed to load \"" + rig.getFullPathName().toStdString() + "\"");
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

void MainContentComponent::openClusters()
{
  if (_clusterWindow != nullptr)
    return;

  _clusterWindow = new ClusterBusterWindow(_search->getResults());
  juce::Rectangle<int> area(50, 50, 1280, 720);
  _clusterWindow->setBounds(area);
  _clusterWindow->setResizable(true, false);
  _clusterWindow->setUsingNativeTitleBar(true);
  _clusterWindow->setVisible(true);

  getRecorder()->log(ACTION, "All Clusters window opened");
}

void MainContentComponent::lockAllColor()
{
  auto devices = getRig()->getAllDevices();
  for (auto d : devices.getDevices()) {
    lockDeviceParam(d->getId(), "colorRed");
    lockDeviceParam(d->getId(), "colorGreen");
    lockDeviceParam(d->getId(), "colorBlue");
    lockDeviceParam(d->getId(), "colorH");
    lockDeviceParam(d->getId(), "colorS");
    lockDeviceParam(d->getId(), "colorV");
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
      if (p == "color") {
        unlockDeviceParam(d->getId(), "colorRed");
        unlockDeviceParam(d->getId(), "colorGreen");
        unlockDeviceParam(d->getId(), "colorBlue");
        unlockDeviceParam(d->getId(), "colorH");
        unlockDeviceParam(d->getId(), "colorS");
        unlockDeviceParam(d->getId(), "colorV");
      }
      else {
        unlockDeviceParam(d->getId(), p);
      }
    }
  }
  repaint();
}

void MainContentComponent::lockDevice(Device * d)
{
  string id = d->getId();
  for (auto p : d->getParamNames()) {
    if (p == "color") {
      lockDeviceParam(d->getId(), "colorRed");
      lockDeviceParam(d->getId(), "colorGreen");
      lockDeviceParam(d->getId(), "colorBlue");
      lockDeviceParam(d->getId(), "colorH");
      lockDeviceParam(d->getId(), "colorS");
      lockDeviceParam(d->getId(), "colorV");
    }
    else {
      lockDeviceParam(d->getId(), p);
    }
  }
  repaint();
}

void MainContentComponent::search()
{
  getRecorder()->log(ACTION, "Exploratory search started.");

  // Break out actual search alg into different function to prevent main from getting too cluttered.
  list<SearchResult*> results = attributeSearch(_attrs->getActiveAttributes(), getGlobalSettings()->_editDepth);

  if (results.size() > 0) {
    getStatusBar()->setStatusMessage("Search finished with " + String(results.size()) + " results.");
    // display results in the explorer (handles rendering and clustering)
    _search->display(results);
  }
  else {
    getStatusBar()->setStatusMessage("WARNING: No results displayed because search yielded no results.");
  }

  getRecorder()->log(ACTION, (String("Exploratory search ended. Returned ") + String(results.size()) + String(" results.")).toStdString());
}
