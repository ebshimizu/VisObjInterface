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

using namespace Lumiverse;
using namespace Lumiverse::ShowControl;

// Gets the application command manager for this application.
ApplicationCommandManager* getApplicationCommandManager();

// Get status bar object
StatusBar* getStatusBar();

// Gets the lumiverse Rig object present in the entire application
Rig* getRig();

// Deletes all global variables, should be called on application destroy
void cleanUpGlobals();

enum command {
  // File
  open = 0x0001,
  close = 0x0002,
  save = 0x0003,
  saveAs = 0x0004

  // Internal
  // Render
  // Window
};

#endif  // GLOBALS_H_INCLUDED
