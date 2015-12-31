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

// Gets the application command manager for this application.
ApplicationCommandManager* getApplicationCommandManager();

// Get status bar object
StatusBar* getStatusBar();

// Gets the lumiverse Rig object present in the entire application
Rig* getRig();

// Get the action logging object
Recorder* getRecorder();

DocumentWindow* getAppTopLevelWindow();

// Deletes all global variables, should be called on application destroy
void cleanUpGlobals();

enum command {
  // File
  OPEN = 0x0001,

  // Internal
  REFRESH_PARAMS = 0x1000,

  // Render
  ARNOLD_RENDER = 0x2000,
  RENDER_SETTINGS = 0x2001

  // Window
};

#endif  // GLOBALS_H_INCLUDED
