/*
  ==============================================================================

    TestAttribute.cpp
    Created: 4 Jan 2016 3:10:38pm
    Author:  Evan

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "TestAttribute.h"

//==============================================================================
TestAttribute::TestAttribute() : AttributeControllerBase("Test (Warm)")
{
}

TestAttribute::TestAttribute(String name) : AttributeControllerBase(name)
{
}

TestAttribute::~TestAttribute()
{
}

double TestAttribute::cctPenalty(double cct)
{
  return pow((cct - _targetCCT) / 1000, 2);
}

double TestAttribute::duvPenalty(double duv)
{
  // Scale duv to be close to returned value from cctPenalty
  return pow((0 - duv * 1000), 2);
}

double TestAttribute::evaluateScene(Device* key, Device* fill, Device* rim)
{
  // this test function attempts to define a measure of "warmness" of a scene
  // it does this by measuring how far away the average cct is from a target value
  // and how far away the average Duv is from 0

  double sumCCTP = 0, sumDuvP = 0;

  // we explicitly assume the existence of three lights: key, fill, and rim.
  // weights for each light are set accordingly.
  auto kcct = key->getColor()->getCCT();
  auto kintens = key->getIntensity()->asPercent();
  sumCCTP += (100 - (cctPenalty(kcct[0]) * _keyWeight)) * kintens;
  sumDuvP += (100 - (duvPenalty(kcct[1]) * _keyWeight)) * kintens;

  auto fcct = fill->getColor()->getCCT();
  auto fintens = fill->getIntensity()->asPercent();
  sumCCTP += (100 - (cctPenalty(fcct[0]) * _fillWeight)) * fintens;
  sumDuvP += (100 - (duvPenalty(fcct[1]) * _fillWeight)) * fintens;;

  auto rcct = rim->getColor()->getCCT();
  auto rintens = rim->getIntensity()->asPercent();
  sumCCTP += (100 - (cctPenalty(rcct[0]) * _rimWeight)) * rintens;
  sumDuvP += (100 - (duvPenalty(rcct[1]) * _rimWeight)) * rintens;

  return (sumCCTP + sumDuvP);
}
