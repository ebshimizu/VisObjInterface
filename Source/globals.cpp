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
    return true;

  if (!d->paramExists(param))
    return true;

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

void toggleDeviceParamLock(string id, string param)
{
  Device* d = getRig()->getDevice(id);

  if (d->getMetadata(param + "_lock") == "y") {
    unlockDeviceParam(id, param);
  }
  else {
    lockDeviceParam(id, param);
  }
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

    string filename = _logRootDir + "/traces/" + _sessionName + ".csv";
    
    file.open(filename, ios::trunc);

    // export format:
    // time, Thread id, sample id, function val, acceptance chance, generating edit name, feature vector
    for (auto& kvp : _samples) {
      for (auto& d : kvp.second) {
        double time = chrono::duration<float>(d._timeStamp - _searchStartTime).count();
        
        file << time << "," << kvp.first << "," << d._sampleId << "," << d._f << "," << d._a << "," << d._editName << "," << d._accepted << ",";
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

    // write metadata file
    filename += ".meta";
    file.open(filename, ios::trunc);
    file << _sessionSearchSettings;

    // write out search specific settings
    file << "Starting Edit Depth: " << _startChainLength << "\n";
    file << "Edit Step Size: " << _editStepSize << "\n";
    file << "Max MCMC Iterations: " << _maxMCMCIters << "\n";
    file << "Max Displayed Results: " << _maxReturnedScenes << "\n";
    file << "Required Distance from Other Results: " << _jndThreshold << "\n";
    file << "Search Failure Limit: " << _searchFailureLimit << "\n";
    file << "Search Threads: " << _searchThreads << "\n";
    file << "Temperature: " << _T << "\n";

    file.close();

    if (_autoRunTraceGraph) {
      // actually just go generate a report now
      string cmd = "python C:/Users/eshimizu/Documents/AttributesInterface/dataviz/tsne2.py " + _logRootDir + "/traces/" + _sessionName + " 30";
      system(cmd.c_str());
    }
  }

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

      vector<double> sceneVals;
      string tooltip;
      DebugData sample;

      int i = 0;
      while (getline(lineStream, cell, ',')) {
        if (i == 1) {
          sample._threadId = stoi(cell);
        }
        else if (i == 2) {
          sample._sampleId = stoi(cell);
        }
        else if (i == 3) {
          sample._f = stod(cell);
        }
        else if (i == 4) {
          sample._a = stod(cell);
        }
        else if (i == 5) {
          sample._editName = cell;
        }
        else if (i == 6) {
          sample._accepted = (stoi(cell) == 1) ? true : false;
        }
        else if (i > 6) {
          sceneVals.push_back(stod(cell));
        }
        
        i++;
      }

      // create scene vector
      sample._scene.resize(sceneVals.size());
      for (i = 0; i < sceneVals.size(); i++) {
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

void GlobalSettings::updateCache()
{
  ArnoldAnimationPatch* p = getAnimationPatch();

  if (p == nullptr) {
    getRecorder()->log(RENDER, "Render failed. Could not find ArnoldAnimationPatch.");
    return;
  }

  // Get the image dimensions
  int width = 100 * 2; //getGlobalSettings()->_renderWidth;
  int height = 100 * 2; //getGlobalSettings()->_renderHeight;
  p->setDims(width, height);
  p->setSamples(getGlobalSettings()->_stageRenderSamples);

  Image cache = Image(Image::ARGB, width, height, true);
  uint8* bufptr = Image::BitmapData(cache, Image::BitmapData::readWrite).getPixelPointer(0, 0);
  p->renderSingleFrameToBuffer(getRig()->getDeviceRaw(), bufptr, width, height);

  setCache(cache);
}

void GlobalSettings::setCache(Image img)
{
  _renderCache = img;
  _cacheUpdated = true;
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

void GlobalSettings::generateDefaultConstraints()
{
  // create a constraint for each system
  auto systems = getRig()->getMetadataValues("system");
  auto& constraints = getGlobalSettings()->_constraints;

  for (auto& s : systems) {
    string query = "$system=" + s;
    constraints[query] = ConsistencyConstraint(query, LOCAL, { INTENSITY, RED, GREEN, BLUE, HUE, SAT });
  }
}

void GlobalSettings::exportSettings()
{
  File f = File::getCurrentWorkingDirectory().getChildFile("settings.json");

  JSONNode settings;
  settings.push_back(JSONNode("thumbnailRenderSamples", _thumbnailRenderSamples));
  settings.push_back(JSONNode("stageRenderSamples", _stageRenderSamples));
  settings.push_back(JSONNode("searchDerivDelta", _searchDerivDelta));
  settings.push_back(JSONNode("renderWidth", _renderWidth));
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
  _startChainLength = 30;
  _clusterDistThreshold = 0.30;
  _editStepSize = 0.25;
  _maxMCMCIters = 10;
  _numDisplayClusters = 1;
  _jndThreshold = 3;
  _currentSortMode = "Attribute Default";
  _clusterElemsPerRow = 6;
  _maxReturnedScenes = 500;
  _showThumbnailImg = false;
  _T = 1;
  _exportTraces = false;
  _logRootDir = ".";
  _clusterCounter = 0;
  _meanShiftEps = 1e-4;
  _meanShiftBandwidth = 7;
  _grayscaleMode = false;
  _searchFailureLimit = 3;
  _searchThreads = thread::hardware_concurrency() / 2;
  _autoRunTraceGraph = false;
  _standardMCMCIters = (int) 5e3;
  _numPrimaryClusters = 6;
  _primaryClusterMethod = DIVISIVE; // "K-Means";
  _primaryClusterMetric = DIRPPAVGLAB; // "Per-Pixel Average Lab Difference";
  _primaryFocusArea = ALL_IMAGE; // "All";
  _numSecondaryClusters = 4;
  _secondaryClusterMethod = DIVISIVE; // "K-Means";
  _secondaryClusterMetric = DIRPPAVGLAB; // "Per-Pixel Average Lab Difference";
  _secondaryFocusArea = ALL_IMAGE; // "All";
  _spectralBandwidth = 2;
  _useFGMask = false;
  _primaryDivisiveThreshold = 7.5;
  _secondaryDivisiveThreshold = 5;
	_clusterDisplay = COLUMNS;
	_searchMode = GIBBS_SAMPLING;
	_maxGradIters = 200;
  _reduceRepeatEdits = true;
  _autoTimeout = 30;
  _evWeight = 0;
  _uniformEditWeights = false;
  _randomInit = false;
  _resampleTime = 30;
  _resampleThreads = thread::hardware_concurrency();
  _maskTolerance = 5;
  _editSelectMode = SIMPLE_BANDIT;
  _continuousSort = false;
  _useSearchStyles = false;
  _searchFrontierSize = 4;
  _repulsionConeK = 0.5;
  _repulsionCostK = 0.5;
  _numPairs = 100;
  _searchDistMetric = L2PARAM;
  _searchDispMetric = PPAVGLAB;

  _imageAttrLoc = File::getCurrentWorkingDirectory().getChildFile("image_attributes");

  if (_searchThreads <= 0)
    _searchThreads = 1;
}

GlobalSettings::~GlobalSettings()
{
  for (auto& e : _edits) {
    delete e;
  }
}

Eigen::Vector3d rgbToLab(double r, double g, double b)
{
  Eigen::Vector3d xyz = ColorUtils::convRGBtoXYZ(r, g, b, sRGB);
  return ColorUtils::convXYZtoLab(xyz, refWhites[D65] / 100.0);
}

double avgLabMaskedImgDiff(Image & a, Image & b, Image & mask)
{
  // dimensions must match
  if (a.getWidth() != b.getWidth() || a.getHeight() != b.getHeight())
    return -1;

  double sum = 0;
  int count = 0;

  for (int y = 0; y < a.getWidth(); y++) {
    for (int x = 0; x < a.getWidth(); x++) {
      // check if in masked set
      if (mask.getPixelAt(x, y).getRed() > 0) {
        count++;

        auto px = a.getPixelAt(x, y);
        Eigen::Vector3d Lab1 = rgbToLab(px.getRed() / 255.0, px.getGreen() / 255.0, px.getBlue() / 255.0);

        auto px2 = b.getPixelAt(x, y);
        Eigen::Vector3d Lab2 = rgbToLab(px2.getRed() / 255.0, px2.getGreen() / 255.0, px2.getBlue() / 255.0);

        sum += (Lab1 - Lab2).norm();
      }
    }
  }

  return sum / count;
}

Image renderImage(Snapshot * s, int width, int height)
{
  auto p = getAnimationPatch();

  if (p == nullptr) {
    return Image(Image::ARGB, width, height, true);
  }

  // with caching we can render at full and then scale down
  Image img = Image(Image::ARGB, width, height, true);
  uint8* bufptr = Image::BitmapData(img, Image::BitmapData::readWrite).getPixelPointer(0, 0);
  p->setDims(width, height);
  p->setSamples(getGlobalSettings()->_thumbnailRenderSamples);

  getAnimationPatch()->renderSingleFrameToBuffer(s->getDevices(), bufptr, width, height);

  return img;
}

set<string> getUnlockedSystems()
{
  map<string, bool> systemIsUnlocked;
  DeviceSet all = getRig()->getAllDevices();

  for (auto d : all.getDevices()) {
    if (!d->metadataExists("system"))
      continue;

    string sys = d->getMetadata("system");
    bool locked = isDeviceParamLocked(d->getId(), "intensity");

    if (systemIsUnlocked.count(sys) == 0) {
      systemIsUnlocked[sys] = false;
    }

    systemIsUnlocked[sys] |= !locked;
  }

  set<string> sets;
  for (auto s : systemIsUnlocked) {
    if (s.second) {
      sets.insert(s.first);
    }
  }

  return sets;
}
