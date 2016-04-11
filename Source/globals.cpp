/*
  ==============================================================================

    globals.cpp
    Created: 14 Dec 2015 1:54:11pm
    Author:  falindrith

  ==============================================================================
*/

#include "globals.h"
#include "MainComponent.h"
#include "AttributeSearch.h"

static ApplicationCommandManager* _manager;
static Rig* _rig;
static StatusBar* _status;
static Recorder* _recorder;
static GlobalSettings* _globalSettings;
static map<int, DeviceSet> _bins;

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

bool isDeviceParamLocked(string id, string param)
{
  string metadataId = param + "_lock";
  Device* d = getRig()->getDevice(id);

  if (d == nullptr)
    return false;

  if (d->getMetadata(metadataId) == "y")
    return true;
  else
    return false;
}

void lockDeviceParam(string id, string param)
{
  Device* d = getRig()->getDevice(id);

  if (d == nullptr)
    return;

  d->setMetadata(param + "_lock", "y");
}

void unlockDeviceParam(string id, string param)
{
  Device* d = getRig()->getDevice(id);

  if (d == nullptr)
    return;

  d->setMetadata(param + "_lock", "n");
}

void exportSearchResults(list<SearchResult*>& results, int depth, string desc)
{
  // three files exported here:
  // vector values
  // objective function values
  // Sample IDs for each row of the vectors

  ofstream xfile;
  ofstream valfile;
  ofstream key;

  time_t t = time(0);   // get time now
  struct tm * now = localtime(&t);

  char buffer[80];
  strftime(buffer, 80, "%Y-%m-%d-%H%M", now);

  string xfname = getGlobalSettings()->_traceRootDir + "/search-vectors-" + string(buffer) + "-depth-" + String(depth).toStdString() + "-" + desc + ".txt";
  string valfname = getGlobalSettings()->_traceRootDir + "/search-vals-" + string(buffer) + "-depth-" + String(depth).toStdString() + "-" + desc + ".txt";
  string keyname = getGlobalSettings()->_traceRootDir + "/search-ids-" + string(buffer) + "-depth-" + String(depth).toStdString() + "-" + desc + ".txt";

  // export vectors and values
  xfile.open(xfname, ios::trunc);
  valfile.open(valfname, ios::trunc);
  key.open(keyname, ios::trunc);

  for (const auto& r : results) {
    for (int i = 0; i < r->_scene.size(); i++) {
      xfile << r->_scene[i] << "\t";
    }
    xfile << "\n";
    valfile << r->_objFuncVal << "\n";
    key << r->_sampleNo << "\n";
  }

  // automatically run t-sne?
  
  xfile.close();
  valfile.close();
  key.close();
}

void GlobalSettings::dumpDiagnosticData()
{
  if (_exportTraces) {
    ofstream file;
    ofstream indexFile;

    time_t t = time(0);   // get time now
    struct tm * now = localtime(&t);

    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d-%H%M", now);

    string filename = _traceRootDir + "/search-" + string(buffer) + ".csv";
    string indexFilename = _traceRootDir + "/search-selectedIds-" + string(buffer) + ".csv";
    
    file.open(filename, ios::trunc);
    indexFile.open(indexFilename, ios::trunc);

    for (int i = 0; i < _fxs.size(); i++) {
      file << _fxs[i] << "," << _as[i] << "," << _editNames[i] << "\n";
    }

    for (const auto& s : _selectedSamples) {
      indexFile << s << "\n";
    }

    file.close();
    indexFile.close();

    // actually just go generate a report now
    string cmd = "python C:/Users/falindrith/OneDrive/Documents/research/attributes_project/app/AttributesInterface/dataviz/plotSearchTrace.py " + filename;
    system(cmd.c_str());
  }

  _as.clear();
  _fxs.clear();
  _editNames.clear();
  _selectedSamples.clear();
  _clusterCounter = 0;
}

unsigned int GlobalSettings::getSampleID()
{
  unsigned int ret = _clusterCounter;
  _clusterCounter++;
  return ret;
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
    if (DocumentWindow* win = dynamic_cast<DocumentWindow*> (TopLevelWindow::getTopLevelWindow(i))) {
      return win;
    }

  return nullptr;
}

DocumentWindow* getAppMainContentWindow()
{
  for (int i = TopLevelWindow::getNumTopLevelWindows(); --i >= 0;)
    if (DocumentWindow* win = dynamic_cast<DocumentWindow*> (TopLevelWindow::getTopLevelWindow(i))) {
      if (dynamic_cast<MainContentComponent*>(win->getContentComponent()) != nullptr)
        return win;
    }

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
  _minEditDist = 0.5;
  _numEditScenes = 15;
  _renderWidth = 0;
  _renderHeight = 0;
  _thumbnailRenderScale = 0.25;
  _editDepth = 3;
  _clusterDistThreshold = 0.30;
  _editStepSize = 0.02;
  _maxMCMCIters = 25;
  _numDisplayClusters = 3;
  _jndThreshold = 0.25;
  _randomMode = false;
  _currentSortMode = "Attribute Default";
  _clusterElemsPerRow = 6;
  _accceptBandwidth = 0.05;
  _maxReturnedScenes = 100;
  _jndInc = 0.01;
  _showThumbnailImg = false;
  _explorationTolerance = 8;
  _T = 1;
  _exportTraces = true;
  _traceRootDir = "C:/Users/falindrith/OneDrive/Documents/research/attributes_project/app/AttributesInterface/traces";
  _clusterCounter = 0;
}

GlobalSettings::~GlobalSettings()
{
}
