/*
  ==============================================================================

    NoireAttribute.cpp
    Created: 6 Apr 2016 2:50:15pm
    Author:  falindrith

  ==============================================================================
*/

#include "NoireAttribute.h"

NoireAttribute::NoireAttribute() : HistogramAttribute("Noir", 50, 50)
{
  _autoLockParams.insert("polar");
  _autoLockParams.insert("azimuth");
  _autoLockParams.insert("penumbraAngle");
  _autoLockParams.insert("color"); // temporary lock for color?
}

NoireAttribute::~NoireAttribute()
{
}

double NoireAttribute::evaluateScene(Snapshot * s)
{
  Image i = generateImage(s);

  Histogram1D brightness = getGrayscaleHist(i, 100);

  // Targeting 80% dark (below 20%) and 5% bright (above 75%)
  // remaining 10% can be anywhere
  double pctBelow15 = brightness.percentLessThan(20);
  double pctAbove = brightness.percentGreaterThan(75);

  double darkScore = min(1.0, 1 - ((.8 - pctBelow15) / .8));
  double brightScore = min(1.0, 1 - ((.05 - pctAbove) / .05));

  return (darkScore * 0.6 + brightScore * 0.4) * 100;
}