/*
  ==============================================================================

    ContrastAttribute.cpp
    Created: 29 Feb 2016 11:25:00am
    Author:  falindrith

  ==============================================================================
*/

#include "ContrastAttribute.h"

ContrastAttribute::ContrastAttribute() : AttributeControllerBase("Contrast")
{
}

ContrastAttribute::ContrastAttribute(String name) : AttributeControllerBase(name)
{
}

ContrastAttribute::~ContrastAttribute()
{
}

double ContrastAttribute::evaluateScene(Device * key, Device * fill, Device * rim)
{
  auto kIntens = key->getIntensity()->asPercent() * key->getParam<LumiverseColor>("color")->getLab(ReferenceWhite::D65)[0];
  auto fIntens = fill->getIntensity()->asPercent() * fill->getParam<LumiverseColor>("color")->getLab(ReferenceWhite::D65)[0];

  // By definitiion kIntens >= fIntens
  if (kIntens != 0) {
    return (1 - (fIntens / kIntens)) * 100;
  }
  else {
    return 0;
  }
}
