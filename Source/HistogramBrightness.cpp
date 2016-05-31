/*
  ==============================================================================

    HistogramBrightness.cpp
    Created: 22 Apr 2016 11:38:15am
    Author:  falindrith

  ==============================================================================
*/

#include "HistogramBrightness.h"

HistogramBrightness::HistogramBrightness(string name, int numBins, int w, int h) :
  _numBins(numBins), HistogramAttribute(name, w, h)
{
  _autoLockParams.insert("color");
  _autoLockParams.insert("penumbraAngle");
  _autoLockParams.insert("polar");
  _autoLockParams.insert("azimuth");
}

HistogramBrightness::~HistogramBrightness()
{
}

double HistogramBrightness::evaluateScene(Snapshot * s)
{
  Image i = generateImage(s);
  Histogram1D brightness = getGrayscaleHist(i, _numBins);

  // return average brightness penalized slightly by how many max brightness pixels we have
  return (brightness.avg() - brightness.percentGreaterThan(99)) * 100;
}
