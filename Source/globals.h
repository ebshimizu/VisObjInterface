/*
  ==============================================================================

    globals.h
    Created: 14 Dec 2015 11:45:38am
    Author:  falindrith

  ==============================================================================
*/

#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "LumiverseCore.h"
#include "LumiverseShowControl/LumiverseShowControl.h"
#include "StatusBar.h"
#include "Recorder.h"
#include "Edit.h"
//#include <vld.h>

using namespace Lumiverse;
using namespace Lumiverse::ShowControl;

class MainWindow;
class Edit;

typedef pair<Eigen::VectorXd, unsigned int> mcmcSample;

enum command {
  // File
  OPEN = 0x0001,
  SAVE = 0x0002,
  SAVE_AS = 0x0003,
  SAVE_RENDER = 0x0004,

  // Internal
  REFRESH_PARAMS = 0x2000,
  REFRESH_ATTR = 0x2001,

  // Render
  ARNOLD_RENDER = 0x3000,

  // Edit
  SETTINGS = 0x4000,
  UNDO = 0x4001,
  REDO = 0x4002,
  GET_FROM_ARNOLD = 0x4003,

  // Explore
  SEARCH = 0x5000,
  RECLUSTER = 0x5001,
  LOCK_ALL_POSITION = 0x5003,
  LOCK_ALL_COLOR = 0x5004,
  LOCK_ALL_INTENSITY = 0x5005,
  UNLOCK_ALL = 0x5006,
  LOCK_ALL_AREAS_EXCEPT = 0x5007,
  LOCK_ALL_SYSTEMS_EXCEPT = 0x5008,
  LOCK_AREA = 0x5009,
  LOCK_SYSTEM = 0x500A,
  STOP_SEARCH = 0x500B,
  GET_NEW_RESULTS = 0x500C,
  UPDATE_NUM_THREADS = 0x500D

  // Window
};

// Flags for indicating what attributes should be considered
// in the search step
enum AttributeConstraint {
  A_IGNORE,
  A_LESS,
  A_EQUAL,
  A_MORE,
  A_EXPLORE
};

// Search data structures
// controllable lighting parameters. Split here since don't want to waste time
// parsing strings for things like color.hue
enum EditParam {
  INTENSITY,
  HUE,
  SAT,
  VALUE,
  POLAR,
  AZIMUTH,
  SOFT,
  RED,
  GREEN,
  BLUE
};

// Since we're dealing with a variable number of lights, the system needs
// to know how many devices to change on one edit.
enum EditNumDevices {
  D_ALL,     // Search though all devices at once
  D_UNIFORM, // Search through one device at a time (sub-edit)
  D_JOINT,   // All lights get the same change applied to them
  // the folowing are color schemes for 2, 4, 3, 4, 3 systems respectively
  D_COMPLEMENTARY_COLOR,
  D_ANALOGOUS_COLOR,
  D_TRIADIC_COLOR,
  D_TETRADIC_COLOR,
  D_SPLIT_COMPLEMENTARY_COLOR
};

// these constraints define an edit (or rather, which parameters an edit can deal with)
// Some more uncommon edits may have additional constraints (maintain position of
// fill for example) and will be treated y
struct EditConstraint {
  EditConstraint() { }
  EditConstraint(string select, EditParam p, EditNumDevices q) : _select(select), _param(p), _qty(q) { }

  // this is actually a Lumiverse query string indicating which lights should be selected.
  string _select;
  EditParam _param;
  EditNumDevices _qty;
};

// A container for various things that the entire application may want to access
// TODO: at some point, have this load from a file
class GlobalSettings
{
public:
  GlobalSettings();
  //GlobalSettings(string filename);
  ~GlobalSettings();

  int _thumbnailRenderSamples;
  int _stageRenderSamples;
  double _searchDerivDelta;     // h, size of window for finite difference derivative
  int _renderWidth;             // Render width
  int _renderHeight;            // Render height
  double _thumbnailRenderScale; // Thumbnail size 
  int _editDepth;               // Initial edit depth
  double _clusterDistThreshold; // Required average distance from every element to the cluster center
  double _editStepSize;         // MCMC: Std dev of gaussian sample 
  int _maxMCMCIters;            // MCMC: Max number of iterations
  double _jndThreshold;         // For two feature vectors, how far apart they can be to be considered equivalent
  bool _randomMode;             // Primarily for debugging, turning this on ignores parameter values when searching
  int _clusterElemsPerRow;      // Number of elements to show in a cluster detail view
  int _maxReturnedScenes;       // Limiter for how many scenes get returned from a search, primarily limited by memory 
  bool _showThumbnailImg;       // Flag to show thumbnail image in the render area.
  double _T;                    // Temperature controlling MCMC tolerance to worse suggestions
  bool _exportTraces;           // Export trace data for each search operation
  string _traceRootDir;         // Trace file location
  double _meanShiftEps;         // Mean shift epsilon
  double _meanShiftBandwidth;   // Mean shift bandwidth
  bool _grayscaleMode;          // Render images in grayscale instead of color
  int _searchFailureLimit;      // How many times a search thread can fail before increasing the max depth.
  int _searchThreads;           // Background threads for search

  int _clusterCounter;          // Index for identifying accepted samples
  int _numDisplayClusters;      // Number of clusters to display in the results

  // Current sorting method to use
  string _currentSortMode;

  map<string, string> _commandLineArgs;

  // Diagnostic storage for search
  vector<double> _fxs;
  vector<double> _as;
  vector<string> _editNames;
  vector<unsigned int> _selectedSamples;
  string _sessionName;

  // dumps search diagnostics to a file
  // file is 1 csv: first row is accepted function values, second is the a that accepted the value
  void dumpDiagnosticData();

  unsigned int getSampleID();

  // Caches rendered images for attributes that need it.
  // not intended for other uses besides attributes
  Image getCachedImage(Snapshot* s, int w, int h, int samples);
  void invalidateCache();

  Image _renderCache;
  bool _cacheUpdated;

  // Focus region dimensions
  Rectangle<float> _focusBounds;

  // Used by attribute searches and results to track where the scene came from
  vector<Edit*> _edits;

  // deletes edits currently stored in the global cache
  void clearEdits();
};

// Results that eventually get returned to the UI layer
// contains edit history for debug, attribute value, and scene
class SearchResult {
public:
  SearchResult();
  SearchResult(const SearchResult& other);
  ~SearchResult();

  Eigen::VectorXd _scene;
  vector<Edit*> _editHistory;
  double _objFuncVal;
  unsigned int _sampleNo;

  // Paired with a vector of cluster centers, indicates which cluster the result belongs to.
  unsigned long _cluster;
};

// Gets the application command manager for this application.
ApplicationCommandManager* getApplicationCommandManager();

// Get status bar object
StatusBar* getStatusBar();

// Gets the global settings object
GlobalSettings* getGlobalSettings();

// Gets the lumiverse Rig object present in the entire application
Rig* getRig();

// Get the action logging object
Recorder* getRecorder();

DocumentWindow* getAppTopLevelWindow();
DocumentWindow* getAppMainContentWindow();

// Returns the animation patch from the rig, if one exists.
ArnoldAnimationPatch* getAnimationPatch();

// Deletes all global variables, should be called on application destroy
void cleanUpGlobals();

// Returns true if the device has a metadata string equal to 'y' for [param]_lock
bool isDeviceParamLocked(string id, string param);

void lockDeviceParam(string id, string param);
void unlockDeviceParam(string id, string param);

void exportSearchResults(list<SearchResult*>& results, int depth, string desc = "", bool makeGraph = false);

// For debugging, set the common filename for all files in the search session
void setSessionName();

// clamps a value to be between the min and max values
float clamp(float val, float min, float max);

#endif  // GLOBALS_H_INCLUDED
