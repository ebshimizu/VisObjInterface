/*
  ==============================================================================

    AttributeStyles.h
    Created: 19 Oct 2016 3:16:29pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef ATTRIBUTESTYLES_H_INCLUDED
#define ATTRIBUTESTYLES_H_INCLUDED

#include "globals.h"
#include "LumiverseCore.h"
#include "LumiverseShowControl/LumiverseShowControl.h"
#include "../JuceLibraryCode/JuceHeader.h"

using namespace Lumiverse;
using namespace Lumiverse::ShowControl;

double getStyleTerm(Style st, Snapshot* s, Image& img);

double sideLightStyle(Snapshot * s, Image& img);
double directionalLightStyle(Snapshot * s, Image& img);
double flatLightStyle(Snapshot* s, Image& img);

#endif  // ATTRIBUTESTYLES_H_INCLUDED
