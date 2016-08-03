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

double TestAttribute::evaluateScene(Snapshot* s, Image& /* img */)
{
  // Since CCT has a singularity built in to its calculations we're going
  // to just define a "warmest" hue and measure how far away each color is
  // from the warmest scaled by intensity.
  Eigen::Vector2d warmest(78.3575, 62.0396);  // CHab from LCHab space, D65 ref white
  
  double err = 0;
  int numDevices = (int) s->getDevices().size();
  for (auto& d : s->getDevices()) {
    auto color = d->getColor();
    if (color == nullptr)
      continue;
    auto lab = color->getLCHab(ReferenceWhite::D65);
    err += (300 - (warmest - Eigen::Vector2d(lab[1], lab[2])).norm());
  }

  err /= (double)numDevices;
  return err / 3.0;
}
