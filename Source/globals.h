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
class ConsistencyConstraint;

typedef pair<Eigen::VectorXd, unsigned int> mcmcSample;

enum command {
  // File
  OPEN = 0x0001,
  SAVE = 0x0002,
  SAVE_AS = 0x0003,
  SAVE_RENDER = 0x0004,
  OPEN_MASK = 0x0005,
  RELOAD_ATTRS = 0x0006,

  // Internal
  REFRESH_PARAMS = 0x2000,
  REFRESH_ATTR = 0x2001,
	REFRESH_SETTINGS = 0x2002,
  START_AUTO = 0x2003,
  END_AUTO = 0x2004,

  // Render
  ARNOLD_RENDER = 0x3000,

  // Edit
  SETTINGS = 0x4000,
  GET_FROM_ARNOLD = 0x4003,
  CONSTRAINTS = 0x4004,

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
  UPDATE_NUM_THREADS = 0x500D,
  SAVE_RESULTS = 0x500E,
  LOAD_RESULTS = 0x500F,
  LOAD_TRACES = 0x5010,
  PICK_TRACE = 0x5011,
	SAVE_CLUSTERS = 0x5012,
	LOAD_CLUSTERS = 0x5013,
  LOCK_SELECTED_INTENSITY = 0x5014,
  LOCK_SELECTED_COLOR = 0x5015,
  LOCK_ALL_SELECTED = 0x5016,
  UNLOCK_SELECTED_INTENSITY = 0x5017,
  UNLOCK_SELECTED_COLOR = 0x5018,
  UNLOCK_ALL_SELECTED = 0x5019

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

struct DebugData {
  double _f;
  double _a;
  string _editName;
  bool _accepted;
  unsigned int _sampleId;
  int _threadId;
  Eigen::VectorXd _scene;
  chrono::time_point<chrono::high_resolution_clock> _timeStamp;
};

enum ClusterMethod {
  KMEANS,     // "K-Means"
  MEAN_SHIFT, // "Mean Shift"
  SPECTRAL,   // "Spectral Clustering"
  DIVISIVE,   // "Divisive K-Means"
  TDIVISIVE   // "Thresholdede Devisive"
};

enum DistanceMetric {
  PPAVGLAB,         // Per-Pixel Average Lab Difference
  PPMAXLAB,         // Per-Pixel Maximum Lab Difference
  PP90LAB,          // Per-Pixel 90th Percentile Difference
  L2LAB,            // Lab L2 Norm
  L2LUMINANCE,      // Luminance L2 Norm
  L2PARAM,          // Parameter L2 Norm
  L2SOFTMAXPARAM,   // Softmax Parameter L2 Norm
  ATTRDIST,         // Attribute Function Distance
	DIRPPAVGLAB,			// Directed Per-Pixel Average Lab Difference
	DIRPPAVG,					// Per-Pixel Average Gradient Direction Difference
  KEYPARAM          // User selected Key light parameter L2 norm
};

enum FocusArea {
  ALL_IMAGE,  // All
  FOREGROUND, // Foreground
  BACKGROUND  // Background
};

enum ClusterDisplayMode {
	COLUMNS,		// Arrange clusters in variable height columns
	GRID				// Arrange clusters in a fixed grid size
};

enum SearchMode {
	MCMC_EDIT,				 // Markov-Chain Monte Carlo with Metropolis-Hastings criteria + edit system
	LM_GRAD_DESCENT,	 // Levenberg-Marquardt Method
	HYBRID_EXPLORE,		 // Hybrid explore/exploit method using both MCMC Edits and LM
	HYBRID_DEBUG,			 // Single-threaded mode that investigates certain properties of the LM/MCMC methods. Not a real search.
  MIN_MCMC_EDIT,     // Minimizing MCMC (choses minimal point when running MCMC step)
  MCMCLMGD,          // MCMC with an additional LM refinement step
  RECENTER_MCMC_EDIT,// Re-centers the search to different locations once max depth reached
  RECENTER_MCMC_LM   // Does MCMCLMGD and recenters on reaching max depth
};

enum DrawMode {
  NO_DRAW,
  BRUSH_ADD,
  RECT_ADD,
  BRUSH_REMOVE,
  RECT_REMOVE
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
  bool _standardMCMC;           // A mode where the search ignores all edits and runs a standard MCMC search
  int _standardMCMCIters;       // Iterations for a normal MCMC search
  int _clusterElemsPerRow;      // Number of elements to show in a cluster detail view
  int _maxReturnedScenes;       // Limiter for how many scenes get returned from a search, primarily limited by memory 
  bool _showThumbnailImg;       // Flag to show thumbnail image in the render area.
  double _T;                    // Temperature controlling MCMC tolerance to worse suggestions
  bool _exportTraces;           // Export trace data for each search operation
  bool _autoRunTraceGraph;      // Automatically call the python script for making graphs. Can be slow, so you can turn it off if you want.
  string _logRootDir;         // Trace file location
  double _meanShiftEps;         // Mean shift epsilon
  double _meanShiftBandwidth;   // Mean shift bandwidth
  bool _grayscaleMode;          // Render images in grayscale instead of color
  int _searchFailureLimit;      // How many times a search thread can fail before increasing the max depth.
  int _searchThreads;           // Background threads for search
  double _spectralBandwidth;    // Bandwidth used in spectral clustering similarity function
  bool _useFGMask;              // toggles the use of the foreground mask, if loaded
  ClusterMethod _primaryClusterMethod;    // Primary cluster method
  DistanceMetric _primaryClusterMetric;   // Primary cluster distance metric to use
  int _numPrimaryClusters;                // Primary number of clusters
  ClusterMethod _secondaryClusterMethod;  // Secondary clustering method
  DistanceMetric _secondaryClusterMetric; // Secondary clustering distance metric
  int _numSecondaryClusters;    // Number of secondary clusters
  FocusArea _primaryFocusArea;  // If using a mask, which area to focus on during clustering
  FocusArea _secondaryFocusArea;// If using a mask, which area to focus on during secondary clustering
  double _primaryDivisiveThreshold;       // Divisive clustering threshold
  double _secondaryDivisiveThreshold;     // Divisive clustering threshold - secondary
	ClusterDisplayMode _clusterDisplay;			// Determines how clusters are layed out, primary and secondary clusters are still used
	SearchMode _searchMode;				// Search method
	int _maxGradIters;						// Maximum number of gradient descent iterations.
  bool _reduceRepeatEdits;      // Reduce the probability of repeat edits during a search
  int _autoTimeout;             // Maximum amout of time to keep the program running during auto
  float _evWeight;              // Weight of the expected value in the MCMC Edit generation step
  bool _uniformEditWeights;     // Use uniform edit weights, or don't. Default: false
  bool _randomInit;             // If true, each thread randomizes its starting position before running the search
  int _resampleTime;            // For RECENTER_MCMC variants. Number of sample to take before resampling and moving
  int _resampleThreads;         // Number of threads to allow recentering on

  int _clusterCounter;          // Index for identifying accepted samples
  int _numDisplayClusters;      // Number of clusters to display in the results

  // Current sorting method to use
  string _currentSortMode;

  map<String, String> _commandLineArgs;

  // Diagnostic storage for search
  map<int, vector<DebugData> > _samples;

  // Diagnostic storage for loaded traces
  map<int, vector<DebugData> > _loadedTraces;

  string _sessionName;

  // string containing the status of the attributes at search time
  string _sessionSearchSettings;

  // dumps search diagnostics to a file
  void dumpDiagnosticData();
  void loadDiagnosticData(string filename);

  unsigned int getSampleID();

  // Caches rendered images for attributes that need it.
  // not intended for other uses besides attributes
  void updateCache();
  void setCache(Image img);
  void invalidateCache();

  Image _renderCache;
  bool _cacheUpdated;

  // Focus region dimensions
  Rectangle<float> _focusBounds;

  // Used by attribute searches and results to track where the scene came from
  vector<Edit*> _edits;

	// Potentially temporary. Contains the relative importance of each edit in _edits.
	// Needs to be initialized by a call to computeEditWeights in an AttributeSearchThread
	// contained by the globals for convenience at the moment. Future versions of this may
	// request that each scene have weigts attached to it.
	map<double, Edit*> _globalEditWeights;

  // deletes edits currently stored in the global cache
  void clearEdits();

  // Foregorund mask
  Image _fgMask;

  // Freeze mask. If a pixel is non-zero, it should be held as constant as possible
  // in resultant searches
  Image _freeze;

  // Consistency constraints
  map<string, ConsistencyConstraint> _constraints;

  // manipulating constraints
  // Done mostly on file load, or maybe when constraints need to be reset.
  // Creates the proper constraints to enfore default consistency settings
  // which are: per-system within edit consistency
  void generateDefaultConstraints();

  // Device IDs selected as the key lights for clustering.
  // All other lights are treated as secondary lights
  vector<string> _keyIds;

  // distance matrix for EMD. initialized on attribute load
  vector<vector<double> > _metric;

  // records the time the search was started and ended
  chrono::time_point<chrono::high_resolution_clock> _searchStartTime;
  chrono::time_point<chrono::high_resolution_clock> _searchEndTime;
  chrono::time_point<chrono::system_clock> _searchAbsStartTime;
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
  chrono::time_point<chrono::high_resolution_clock> _creationTime;

  // Paired with a vector of cluster centers, indicates which cluster the result belongs to.
  unsigned long _cluster;

	// Extra data that may be used by any part of program
	map<String, String> _extraData;
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
void toggleDeviceParamLock(string id, string param);

// For debugging, set the common filename for all files in the search session
void setSessionName();

// clamps a value to be between the min and max values
float clamp(float val, float min, float max);

// D65 RGB to Lab conversion
Eigen::Vector3d rgbToLab(double r, double g, double b);

#endif  // GLOBALS_H_INCLUDED
