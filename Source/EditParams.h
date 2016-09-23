/*
  ==============================================================================

    EditParams.h
    Created: 23 Sep 2016 5:14:50pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef EDITPARAMS_H_INCLUDED
#define EDITPARAMS_H_INCLUDED

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

#endif  // EDITPARAMS_H_INCLUDED
