/*
  ==============================================================================

    SoftAttribute.cpp
    Created: 3 Feb 2016 4:06:19pm
    Author:  falindrith

  ==============================================================================
*/

#include "SoftAttribute.h"

SoftAttribute::SoftAttribute() : AttributeControllerBase("Soft")
{
}

SoftAttribute::SoftAttribute(String name) : AttributeControllerBase(name)
{
}

SoftAttribute::~SoftAttribute()
{
}

double SoftAttribute::evaluateScene(Device * key, Device * fill, Device * rim)
{
  // Here softness will be defined to be a low contrast between the key-rim, fill-rim
  // and slight contrast between key-fill

  double keyFillRatio = fill->getIntensity()->getVal() / key->getIntensity()->getVal();

  double rimScore = 1 - pow(0 - rim->getIntensity()->asPercent(), 2);
  double keyFillScore = 1 - pow(1 - keyFillRatio, 2);

  double score = (rimScore + keyFillScore) / 2;
  score *= 100;
  return score;
}
