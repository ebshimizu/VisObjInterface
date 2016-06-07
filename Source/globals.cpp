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

SearchResult::SearchResult() { }

SearchResult::SearchResult(const SearchResult & other) :
  _scene(other._scene), _editHistory(other._editHistory), _objFuncVal(other._objFuncVal),
  _sampleNo(other._sampleNo), _cluster(other._cluster)
{
}

SearchResult::~SearchResult() {
}

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

void setSessionName()
{
  time_t t = time(0);   // get time now
  struct tm * now = localtime(&t);

  char buffer[80];
  strftime(buffer, 80, "%Y-%m-%d-%H%M", now);

  getGlobalSettings()->_sessionName = "search-" + string(buffer);
}

float clamp(float val, float min, float max)
{
  return (val > max) ? max : ((val < min) ? min : val);
}

void GlobalSettings::dumpDiagnosticData()
{
  if (_exportTraces) {
    ofstream file;

    string filename = _traceRootDir + "/" + _sessionName + ".csv";
    
    file.open(filename, ios::trunc);

    // export format:
    // Thread id, sample id, function val, acceptance chance, generating edit name, feature vector
    for (auto& kvp : _samples) {
      for (auto& d : kvp.second) {
        file << kvp.first << "," << d._sampleId << "," << d._f << "," << d._a << "," << d._editName << "," << d._accepted << ",";
        for (int i = 0; i < d._scene.size(); i++) {
          file << d._scene[i];
          if (i <= (d._scene.size() - 2)) {
            file << ",";
          }
          else {
            file << "\n";
          }
        }
      }
    }

    file.close();

    if (_autoRunTraceGraph) {
      // actually just go generate a report now
      string cmd = "python C:/Users/eshimizu/Documents/AttributesInterface/dataviz/tsne2.py " + _traceRootDir + "/" + _sessionName + " 30";
      system(cmd.c_str());
    }
  }

  _samples.clear();
  _clusterCounter = 0;
}

void GlobalSettings::loadDiagnosticData(string filename)
{
  ifstream file(filename);
  string line;

  _loadedTraces.clear();

  getStatusBar()->setStatusMessage("Loading traces...");
  int traceID = 1;

  if (file.is_open()) {
    while (getline(file, line)) {
      stringstream lineStream(line);
      string cell;

      SearchResult* r = new SearchResult();
      vector<double> sceneVals;
      string tooltip;
      DebugData sample;

      int i = 0;
      while (getline(lineStream, cell, ',')) {
        if (i == 0) {
          sample._threadId = stoi(cell);
        }
        else if (i == 1) {
          sample._sampleId = stoi(cell);
        }
        else if (i == 2) {
          sample._f = stod(cell);
        }
        else if (i == 3) {
          sample._a = stod(cell);
        }
        else if (i == 4) {
          sample._editName = cell;
        }
        else if (i == 5) {
          sample._accepted = (stoi(cell) == 1) ? true : false;
        }
        else {
          sceneVals.push_back(stod(cell));
        }
        
        i++;
      }

      // create scene vector
      sample._scene.resize(sceneVals.size());
      for (int i = 0; i < sceneVals.size(); i++) {
        sample._scene[i] = sceneVals[i];
      }

      // sort debug data
      if (sample._threadId == -1) {
        _loadedTraces[-1] = { sample };
      }
      else {
        if (sample._editName == "TERMINAL") {
          // new trace id needed, skip adding the terminal element, it's a duplicate
          traceID++;
        }
        else {
          _loadedTraces[traceID].push_back(sample);
        }
      }
    }
  }

  getStatusBar()->setStatusMessage("Loaded " + String(_loadedTraces.size()) + " traces.");
}

unsigned int GlobalSettings::getSampleID()
{
  unsigned int ret = _clusterCounter;
  _clusterCounter++;
  return ret;
}

Image GlobalSettings::getCachedImage(Snapshot* s, int w, int h, int samples)
{
  if (_cacheUpdated && _renderCache.getWidth() == w && _renderCache.getHeight() == h)
    return _renderCache;

  auto devices = s->getDevices();
  auto p = getAnimationPatch();

  if (p == nullptr)
    return Image();

  // render at 2x res, downsample to canonical resolution
  uint8* bufptr = Image::BitmapData(_renderCache, Image::BitmapData::readWrite).getPixelPointer(0, 0);
  p->setDims(w, h);
  p->setSamples(samples);

  getAnimationPatch()->renderSingleFrameToBuffer(devices, bufptr, w, h);

  _cacheUpdated = true;
  return _renderCache;
}

void GlobalSettings::invalidateCache()
{
  _cacheUpdated = false;
}

void GlobalSettings::clearEdits()
{
  for (auto& e : _edits) {
    delete e;
  }
  _edits.clear();
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
  _renderWidth = 0;
  _renderHeight = 0;
  _thumbnailRenderScale = 0.50;
  _editDepth = 3;
  _clusterDistThreshold = 0.30;
  _editStepSize = 0.1;
  _maxMCMCIters = 10;
  _numDisplayClusters = 1;
  _jndThreshold = 0.5;
  _standardMCMC = false;
  _currentSortMode = "Attribute Default";
  _clusterElemsPerRow = 6;
  _maxReturnedScenes = 50;
  _showThumbnailImg = false;
  _T = 1;
  _exportTraces = false;
  _traceRootDir = "C:/Users/eshimizu/Documents/AttributesInterface/traces";
  _clusterCounter = 0;
  _meanShiftEps = 1e-4;
  _meanShiftBandwidth = 0.005;
  _grayscaleMode = false;
  _searchFailureLimit = 3;
  _searchThreads = thread::hardware_concurrency() / 2;
  _autoRunTraceGraph = false;
  _standardMCMCIters = 1e4;

  if (_searchThreads <= 0)
    _searchThreads = 1;
}

GlobalSettings::~GlobalSettings()
{
  for (auto& e : _edits) {
    delete e;
  }
}
