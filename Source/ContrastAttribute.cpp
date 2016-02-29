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
  auto kIntens = key->getIntensity()->asPercent();
  auto fIntens = fill->getIntensity()->asPercent();

  // By definitiion kIntens >= fIntens
  if (kIntens != 0) {
    return (1 - (fIntens / kIntens)) * 100;
  }
  else {
    return 0;
  }
}
