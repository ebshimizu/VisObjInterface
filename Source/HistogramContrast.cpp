/*
  ==============================================================================

    HistogramContrast.cpp
    Created: 25 Apr 2016 3:11:45pm
    Author:  falindrith

  ==============================================================================
*/

#include "HistogramContrast.h"

HistogramContrast::HistogramContrast(string name, int numBins, int w, int h) :
  _numBins(numBins), HistogramAttribute(name, w, h)
{
  _autoLockParams.insert("color");
  _autoLockParams.insert("penumbraAngle");
  _autoLockParams.insert("polar");
  _autoLockParams.insert("azimuth");
}

HistogramContrast::~HistogramContrast()
{
}

double HistogramContrast::evaluateScene(Snapshot * s)
{
  if (getGlobalSettings()->_renderInProgress)
    return 0;

  generateImage(s);
  Histogram1D brightness = getGrayscaleHist(_numBins);

  return brightness.contrast() * 10;
}
