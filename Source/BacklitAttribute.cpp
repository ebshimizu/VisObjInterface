/*
  ==============================================================================

    BacklitAttribute.cpp
    Created: 3 Feb 2016 12:02:57pm
    Author:  falindrith

  ==============================================================================
*/

#include "BacklitAttribute.h"

BacklitAttribute::BacklitAttribute() : AttributeControllerBase("Backlit")
{
}

BacklitAttribute::BacklitAttribute(String name) : AttributeControllerBase(name)
{
}

BacklitAttribute::~BacklitAttribute()
{
}

double BacklitAttribute::evaluateScene(Device * key, Device * fill, Device * rim)
{
  double targetKeyAzimuth = 0.75;
  double targetKeyPolar = 0.7;
  double targetFillAzimuth = 0.5;
  double targetFillPolar = 0.7;
  double targetRimAzimuth = 0.75;
  double targetRimPolar = 0.55;

  double keyPosScore = 2 - (pow(targetKeyAzimuth - key->getParam<LumiverseOrientation>("azimuth")->asPercent(), 2) +
                       pow(targetKeyPolar - key->getParam<LumiverseOrientation>("polar")->asPercent(), 2));
  double fillPosScore = 2 - (pow(targetFillAzimuth - fill->getParam<LumiverseOrientation>("azimuth")->asPercent(), 2) +
                        pow(targetFillPolar - fill->getParam<LumiverseOrientation>("polar")->asPercent(), 2));
  double rimPosScore = 2 - (pow(targetRimAzimuth - rim->getParam<LumiverseOrientation>("azimuth")->asPercent(), 2) +
                       pow(targetRimPolar - rim->getParam<LumiverseOrientation>("polar")->asPercent(), 2));

  double targetRatio = 0.03;
  double keyFillIntensRatio = fill->getIntensity()->getVal() / key->getIntensity()->getVal();
  double keyFillIntensScore = 1 - pow(keyFillIntensRatio - targetRatio, 2);

  double targetRimKeyRatio = 0.5;
  double keyRimIntensRatio = rim->getIntensity()->getVal() / key->getIntensity()->getVal();
  double keyRimIntensScore = 1 - pow(keyRimIntensRatio - targetRimKeyRatio, 2);

  // highest score possible is 8, so to normalize within the range [0, 100]...
  double score = (keyPosScore + fillPosScore + rimPosScore + keyFillIntensScore + keyRimIntensScore) / 8;
  score *= 100;
  return score;
}
