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

using namespace Lumiverse;
using namespace Lumiverse::ShowControl;

class MainWindow;

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
  double _minEditDist;          // Minimum attribute difference needed to be returned from an edit
  int _numEditScenes;           // Number of scenes per iteration that get passed on to the next stage. i.e. frontier size
  int _renderWidth;             // Render width
  int _renderHeight;            // Render height
  double _thumbnailRenderScale; // Thumbnail size 
  int _editDepth;               // Maximum edit depth
  double _clusterDistThreshold; // Required average distance from every element to the cluster center
  double _editStepSize;         // MCMC: Std dev of gaussian sample 
  int _maxMCMCIters;            // MCMC: Max number of iterations
  double _jndThreshold;         // For two feature vectors, how far apart they can be to be considered equivalent
  bool _randomMode;             // Primarily for debugging, turning this on ignores parameter values when searching
  int _clusterElemsPerRow;      // Number of elements to show in a cluster detail view
  double _accceptBandwidth;     // For MCMC samples that are bad, how tolerant we should be of them
  int _maxReturnedScenes;       // Limiter for how many scenes get returned from a search, primarily limited by thumbnail render speed
  double _jndInc;               // Increment to use for adaptive filtering
  bool _showThumbnailImg;       // Flag to show thumbnail image in the render area.
  double _explorationTolerance; // In the exploration phase, how far we can be off the other attribute values to still be ok

  int _numDisplayClusters;      // Number of clusters to display in the results

  // Current sorting method to use
  string _currentSortMode;

  map<string, string> _commandLineArgs;
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
  VIEW_CLUSTERS = 0x5002,
  LOCK_ALL_POSITION = 0x5003,
  LOCK_ALL_COLOR = 0x5004,
  LOCK_ALL_INTENSITY = 0x5005,
  UNLOCK_ALL = 0x5006,
  LOCK_ALL_AREAS_EXCEPT = 0x5007,
  LOCK_ALL_SYSTEMS_EXCEPT = 0x5008,
  LOCK_AREA = 0x5009,
  LOCK_SYSTEM = 0x500A

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
  D_ALL,    // Search though all devices at once
  D_UNIFORM, // Search through one device at a time (sub-edit)
  D_JOINT   // All lights get the same change applied to them
};

// these constraints define an edit (or rather, which parameters an edit can deal with)
// Some more uncommon edits may have additional constraints (maintain position of
// fill for example) and will be treated separately
struct EditConstraint {
  EditConstraint() { }
  EditConstraint(string select, EditParam p, EditNumDevices q) : _select(select), _param(p), _qty(q) { }

  // this is actually a Lumiverse query string indicating which lights should be selected.
  string _select;
  EditParam _param;
  EditNumDevices _qty;
};

#endif  // GLOBALS_H_INCLUDED
