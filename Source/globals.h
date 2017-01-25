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
#include "EditParams.h"
#include "LumiverseCore.h"
#include "LumiverseShowControl/LumiverseShowControl.h"
#include "StatusBar.h"
#include "Recorder.h"
#include "Edit.h"
#include "Idea.h"
//#include <vld.h>

using namespace Lumiverse;
using namespace Lumiverse::ShowControl;

class MainWindow;
class Edit;
class ConsistencyConstraint;
class Idea;

typedef pair<Eigen::VectorXd, unsigned int> mcmcSample;

enum command {
  // File
  OPEN = 0x0001,
  SAVE = 0x0002,
  SAVE_AS = 0x0003,
  SAVE_RENDER = 0x0004,
  OPEN_MASK = 0x0005,
  LOAD_ATTRS = 0x0006,
  RELOAD_ATTRS = 0x0007,
  SAVE_IDEAS = 0x0008,
  LOAD_IDEAS = 0x0009,

  // Internal
  REFRESH_PARAMS = 0x2000,
  REFRESH_ATTR = 0x2001,
	REFRESH_SETTINGS = 0x2002,
  START_AUTO = 0x2003,
  END_AUTO = 0x2004,

  // Render
  ARNOLD_RENDER = 0x3000,
  TOGGLE_SELECT_VIEW = 0x3001,

  // Edit
  SETTINGS = 0x4000,
  GET_FROM_ARNOLD = 0x4003,
  CONSTRAINTS = 0x4004,
  RESET_ALL = 0x4005,
  RESET_TIMER = 0x4006,
  COPY_DEVICE = 0x4007,
  PASTE_ALL = 0x4008,
  PASTE_INTENS = 0x4009,
  PASTE_COLOR = 0x400A,
  SET_TO_FULL = 0x400B,
  SET_TO_OFF = 0x400C,
  SET_TO_WHITE = 0x400D,

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
  UNLOCK_ALL_SELECTED = 0x5019,
  DELETE_ALL_PINS = 0x501a,

  // Window
  INTERFACE_OLD = 0x6001,
  INTERFACE_NEW = 0x6002,
  INTERFACE_ALL = 0x6003,
  SHOW_PROMPT = 0x6004
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
  TDIVISIVE,  // "Thresholdede Devisive"
  STYLE       // Cluster based on style types
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
  KEYPARAM,         // User selected Key light parameter L2 norm
  L2GRAYPARAM       // L2 norm over paramter values converted to grayscale (basically ignore color)
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
  REDUCE_REDUNDANCY, // No inner edit iteration loop
  RANDOM_START,      // Starting points of the search are randomized
  KRANDOM_START,     // Random start with k-means clustering used to select a frontier
  KMCMC,             // Standard search with a k-size frontier
  CMAES,             // Gradient free opt
  REPULSION_KMCMC,   // K-Frontier MCMC with a repulsion term added to configs that have already been seen.
  GIBBS_SAMPLING     // Not a MCMC method, uses Gibbs sampling to generate examples based on user constraints
};

enum EditSelectMode {
  DEFAULT_CHOICE,     // the default edit selection mode that's been in this application forever
  SIMPLE_BANDIT,      // a multi-armed bandit that count successes and failures as weights
  UNIFORM_RANDOM,     // all edits are chosen at random all the time
  ADVERSARIAL_BANDIT, // adversarial bandit approach, see Auer et al. 1998
  DIRECTED_SAMPLING   // Gibbs sampling test mode. Attempts to draw samples from a particular 
};

enum DrawMode {
  RECT_ADD,
  RECT_PIN,
  SELECT_ONLY
};

enum GibbsConstraint {
  LOCKED = -1,
  FREE,
  HIGH,
  LOW
};

enum GibbsParam {
  GINTENSITY,
  GCOLOR
};

struct Timing {
  float _sampleTime;
  float _sampleRenderTime;
  float _sampleEvalTime;
  float _addResultTime;
  float _addResultRenderTime;
  float _addResultEvalTime;
  int _numSamples;
  int _numResults;
};

struct GibbsScheduleConstraint {
  DeviceSet _targets;
  GibbsParam _param;

  // this bool controls whether or not the devices are sampled accoring to
  // their system or if they're all independently free parameters
  bool _followConventions;
};

// caches images rendered at 50% and 51% of the whole stage
struct sensCache {
  Image i50;
  Image i51;
  Image i100;
  
  float maxBr;
  int numMaxBr;

  float avgVal;
  int numAboveAvg;

  // generic cache, since i keep adding additional fields
  map<string, float> data;
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
  int _startChainLength;               // Initial edit depth
  double _clusterDistThreshold; // Required average distance from every element to the cluster center
  double _editStepSize;         // MCMC: Std dev of gaussian sample 
  int _maxMCMCIters;            // MCMC: Max number of iterations
  double _jndThreshold;         // For two feature vectors, how far apart they can be to be considered equivalent
  double _viewJndThreshold;     // For views, the jnd threshold
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
  double _maskTolerance;        // Acceptable difference in highligted regions during search
  EditSelectMode _editSelectMode;         // Edit selection mode
  bool _continuousSort;         // Continuously sort the results
  bool _useSearchStyles;        // Adds style-based searches into the search process
  int _searchFrontierSize;      // Size of the search frontier for k-means based search versions
  double _repulsionConeK;       // Repulsion radius factor
  double _repulsionCostK;       // Repulsion cost strength
  int _numPairs;                // Number of pairs to estimate cone radius
  DistanceMetric _searchDistMetric;   // Metric to use for determining how the frontier is selected during a search
  DistanceMetric _searchDispMetric;   // Metric to use for determining if a result should be displayed during a search
  double _thresholdDecayRate;   // how quickly the jnd decays for accepting samples. Smaller is faster
  String _showName;             // current show name / file the rig was loaded from
  float _bigBucketSize;         // weight of the large bucket for color sampling
  bool _recalculateWeights;     // In the color sampler, use the old version or the new version (defualt = true)
  bool _autoCluster;            // turns on or turns off automatic cluster mode
  bool _unconstrained;          // Turns off constraints in the samplers
  bool _pxIntensDist;           // Alternate mode for intensity sampler: samples pixel distribution in image

  int _clusterCounter;          // Index for identifying accepted samples
  int _numDisplayClusters;      // Number of clusters to display in the results

  // Current sorting method to use
  string _currentSortMode;

  map<String, String> _commandLineArgs;

  // Diagnostic storage for search
  map<int, vector<DebugData> > _samples;
  map<int, Timing> _timings;

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
  juce::Rectangle<float> _focusBounds;

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
  DrawMode _freezeDrawMode;

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

  // Save settings to a file in the same directory as the application
  void exportSettings();

  // Load settings from a file, which is typically in the same
  // directory as the application
  void loadSettings(string file = "");

  // folder containing the image attributes to load
  File _imageAttrLoc;

  // temporary working directory for color palette extraction
  File _tempDir;

  // app dir
  File _paletteAppDir;

  // Stores the sensitivity value of each light in the rig.
  // computed on load, this is the average per-pixel change in brightness
  // centered on 50%.
  map<string, double> _sensitivity;
  map<string, double> _systemSensitivity;

  map<string, sensCache> _sensitivityCache;

  // maps ideas to areas of the stage
  // may end up being more complicated based on how sampling needs to work
  map<shared_ptr<Idea>, juce::Rectangle<float> > _ideaMap;
  shared_ptr<Idea> _activeIdea;

  Array<juce::Rectangle<float> > _pinnedRegions;

  // updated before every search, the list of currently pinned devices
  set<string> _intensityPins;
  set<string> _colorPins;

  // stores the rig state before any preview operations
  Snapshot* _rigState;
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
  
  // extra function evaluations for particular attributes
  map<String, double> _extraFuncs;
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

double avgLabMaskedImgDiff(Image& a, Image& b, Image& mask);

// renders the specified snapshot to an image of the specified size
Image renderImage(Snapshot* s, int width, int height);

// returns a set of systems that have at least one light unlocked
set<string> getUnlockedSystems();

// populates the sensitivity map for each light
void computeLightSensitivity();

#endif  // GLOBALS_H_INCLUDED
