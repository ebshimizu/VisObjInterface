/*
  ==============================================================================

    AttributeSearch.h
    Created: 6 Jan 2016 11:25:42am
    Author:  falindrith

  ==============================================================================
*/

#ifndef ATTRIBUTESEARCH_H_INCLUDED
#define ATTRIBUTESEARCH_H_INCLUDED

#include "globals.h"
#include "AttributeControllerBase.h"

enum EditType {
  // Normal edits - edits that manipulate specific lighting parameters
  ALL,
  ALL_HSV,
  ALL_RGB,
  ALL_SAT,
  ALL_HUE,
  KEY_HUE,
  FILL_HUE,
  RIM_HUE,
  KEY_INTENS,
  FILL_INTENS,
  RIM_INTENS,
  KEY_POS,
  FILL_POS,
  RIM_POS,
  KEY_SAT,
  FILL_SAT,
  RIM_SAT,
  KEY_HSV,
  FILL_HSV,
  RIM_HSV,
  KEY_FILL_INTENS,
  KEY_RIM_INTENS,
  FILL_RIM_INTENS,
  KEY_FILL_HUE,
  KEY_FILL_SAT,
  KEY_FILL_HSV,
  KEY_FILL_POS,
  KEY_RGB,
  FILL_RGB,
  RIM_RGB,
  // Special edits - edits that require additional constraints to be matched
  // when they are performed.
  KEY_POS_FILL_POS_MATCH,             // Move the key but maintain fill relative position
  KEY_INTENS_RIM_CONTRAST_MATCH,      // Adjust key intensity but keep key-rim contrast constant
  KEY_INTENS_FILL_CONTRAST_MATCH,     // Adjust key intensity but keep key-fill contrast constant
  KEY_HUE_FILL_HUE_MATCH,             // Adjust key color, apply same relative change to fill color (rotate around color wheel basically)
  KEY_HUE_FILL_RIM_HUE_MATCH          // Adjust key color, apply same relative change to rim/fill color
};

// Don't want to use strings for this for speed reasons
enum EditLightType {
  L_KEY,
  L_FILL,
  L_RIM
};

// controllable lighting parameters. Split here since don't want to waste time
// parsing strings for things like color.hue
enum EditParam {
  INTENSITY,
  HUE,
  SAT,
  VALUE,
  RED,
  GREEN,
  BLUE,
  POLAR,
  AZIMUTH
};

// these constraints define an edit (or rather, which parameters an edit can deal with)
// Some more uncommon edits may have additional constraints (maintain position of
// fill for example) and will be treated separately
struct EditConstraint {
  EditConstraint(EditLightType t, EditParam p) : _light(t), _param(p) { }

  EditLightType _light;
  EditParam _param;
};

struct SearchResult {
  Snapshot* _scene;
  Array<EditType> _editHistory;
  map<string, double> _attrVals;
};

Array<SearchResult> attributeSearch(map<string, AttributeControllerBase*> active);

// Given a current configuration, perform an edit on the configuration
Array<Snapshot*> performEdit(EditType t, Snapshot* s, map<string, AttributeControllerBase*> active);

// computes the numeric derivative for the particular lighting parameter and
// specified attribute
double numericDeriv(EditConstraint c, Snapshot* s, AttributeControllerBase* attr);

// updates the value for a Lumiverse parameter
void setUpdatedValue(EditConstraint c, Snapshot* s);

// Given an EditLightType, get the corresponding light in the rig
Device* getSpecifiedDevice(EditLightType l, Snapshot* s);

#endif  // ATTRIBUTESEARCH_H_INCLUDED
