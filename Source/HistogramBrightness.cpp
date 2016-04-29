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
  if (getGlobalSettings()->_renderInProgress)
    return 0;

  generateImage(s);
  Histogram1D brightness = getGrayscaleHist(_numBins);
  return brightness.avg() * 100;
}
