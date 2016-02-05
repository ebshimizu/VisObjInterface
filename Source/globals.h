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
  double _searchGDGamma;        // gamma, controls speed of gradient descent
  double _searchGDTol;          // Tolerance, how small difference should be to be equivalent to zero (stopping condition)
  double _minEditDist;          // Minimum attribute difference needed to be returned from an edit
  int _numEditScenes;           // Number of scenes per edit that get returned to the search algorithm
  int _renderWidth;             // Render width
  int _renderHeight;            // Render height
  double _thumbnailRenderScale; // Thumbnail size 
  int _editDepth;               // Maximum edit depth
  double _clusterDistThreshold; // Required average distance from every element to the cluster center
  double _clusterDiffThreshold; // Used to filter out results that are too close to each other in search
  int _maxEditIters;            // In case the search gets hung up on something
  double _searchMomentum;       // For momentum gradient descent, the alpha parameter controlling the strength of the backprop
  double _editStepSize;         // MCMC: Std dev of gaussian sample 
  int _maxMCMCIters;            // MCMC: Max number of iterations
  double _jndThreshold;         // For two feature vectors, how far apart they can be to be considered equivalent
  bool _randomMode;             // Primarily for debugging, turning this on ignores parameter values when searching

  int _numDisplayClusters;      // Number of clusters to display in the results

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

  // Internal
  REFRESH_PARAMS = 0x2000,
  REFRESH_ATTR = 0x2001,

  // Render
  ARNOLD_RENDER = 0x3000,

  // Edit
  SETTINGS = 0x4000,

  // Explore
  SEARCH = 0x5000,
  RECLUSTER = 0x5001,
  VIEW_CLUSTERS = 0x5002

  // Window
};

// Flags for indicating what attributes should be considered
// in the search step
enum AttributeConstraint {
  A_IGNORE,
  A_LESS,
  A_EQUAL,
  A_MORE
};

#endif  // GLOBALS_H_INCLUDED
