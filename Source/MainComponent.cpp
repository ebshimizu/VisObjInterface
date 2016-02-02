/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"
#include "globals.h"
#include "SettingsEditor.h"
#include "AttributeSearch.h"

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
    command::SEARCH, command::REFRESH_ATTR, command::SAVE, command::SAVE_AS, command::RECLUSTER
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
    result.setInfo("Recluster", "Recluster results based on uuser settings", "Explore", 0);
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
  default:
    return false;
  }

  return true;
}

void MainContentComponent::setBottomSearchComponent(Component* c, Component* source)
{
  _search->setBotComponent(c, source);
}

void MainContentComponent::openRig() {
  FileChooser fc("Load Show (pick a .rig.json or .playback.json file)", File::getCurrentWorkingDirectory(),
    "*.rig.json;*.playback.json", true);

  if (fc.browseForFileToOpen()) {
    File selected = fc.getResult();
    String fileName = selected.getFullPathName();
    openRig(fileName);
  }
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

  File rig = _parentDir.getChildFile(String(_showName) + ".rig.json");

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

void MainContentComponent::search()
{
  getRecorder()->log(ACTION, "Exploratory search started.");

  // Break out actual search alg into different function to prevent main from getting too cluttered.
  vector<SearchResult*> results = attributeSearch(_attrs->getActiveAttributes(), getGlobalSettings()->_editDepth);

  if (results.size() > 0) {
    // display results in the explorer (handles rendering and clustering)
    _search->display(results);
  }

  getRecorder()->log(ACTION, "Exploratory search ended.");
}
