/*
  ==============================================================================

    globals.cpp
    Created: 14 Dec 2015 1:54:11pm
    Author:  falindrith

  ==============================================================================
*/

#include "globals.h"

static ApplicationCommandManager* _manager;
static Rig* _rig;
static StatusBar* _status;

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
}

Rig* getRig() {
  if (_rig == nullptr) {
    _rig = new Rig();
  }

  return _rig;
}

StatusBar* getStatusBar() {
  if (_status == nullptr) {
    _status = new StatusBar();
  }

  return _status;
}