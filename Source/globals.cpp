/*
  ==============================================================================

    globals.cpp
    Created: 14 Dec 2015 1:54:11pm
    Author:  falindrith

  ==============================================================================
*/

#include "globals.h"

static ApplicationCommandManager* _manager;
static Rig* _rig;
static StatusBar* _status;
static Recorder* _recorder;
static GlobalSettings* _globalSettings;

ApplicationCommandManager* getApplicationCommandManager() {
  if (_manager == nullptr) {
    _manager = new ApplicationCommandManager();
  }

  return _manager;
}

void cleanUpGlobals() {
  if (_manager != nullptr)
    delete _manager;

  if (_rig != nullptr)
    delete _rig;

  if (_status != nullptr)
    delete _status;

  if (_recorder != nullptr)
    delete _recorder;

  if (_globalSettings != nullptr)
    delete _globalSettings;
}

Rig* getRig() {
  if (_rig == nullptr) {
    _rig = new Rig();
  }

  return _rig;
}

DocumentWindow* getAppTopLevelWindow()
{
  for (int i = TopLevelWindow::getNumTopLevelWindows(); --i >= 0;)
    if (DocumentWindow* win = dynamic_cast<DocumentWindow*> (TopLevelWindow::getTopLevelWindow(i)))
      return win;

  return nullptr;
}

ArnoldAnimationPatch * getAnimationPatch()
{
  // Find the patch, we search for the first ArnoldAnimationPatch we can find.
  Rig* rig = getRig();

  ArnoldAnimationPatch* p = nullptr;
  for (const auto& kvp : rig->getPatches()) {
    if (kvp.second->getType() == "ArnoldAnimationPatch") {
      p = (ArnoldAnimationPatch*)kvp.second;
    }
  }

  return p;
}

StatusBar* getStatusBar() {
  if (_status == nullptr) {
    _status = new StatusBar();
  }

  return _status;
}

GlobalSettings * getGlobalSettings()
{
  if (_globalSettings == nullptr) {
    _globalSettings = new GlobalSettings();
  }

  return _globalSettings;
}

Recorder* getRecorder() {
  if (_recorder == nullptr)
    _recorder = new  Recorder();

  return _recorder;
}

GlobalSettings::GlobalSettings()
{
  _thumbnailRenderSamples = -1;
  _stageRenderSamples = 1;
  _searchDerivDelta = 1e-4;
  _minEditDist = 1;
  _numEditScenes = 3;
  _searchGDTol = 1e-6;
  _searchGDGamma = 1;
  _renderWidth = 0;
  _renderHeight = 0;
  _thumbnailRenderScale = 0.25;
  _editDepth = 1;
}

GlobalSettings::~GlobalSettings()
{
}
