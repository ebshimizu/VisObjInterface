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
  _autoLockParams.insert(HUE);
  _autoLockParams.insert(SAT);
  _autoLockParams.insert(VALUE);
  _autoLockParams.insert(POLAR);
  _autoLockParams.insert(AZIMUTH);
  _autoLockParams.insert(SOFT);
}

HistogramContrast::~HistogramContrast()
{
}

double HistogramContrast::evaluateScene(Snapshot * s)
{
  Image i = generateImage(s);
  Histogram1D brightness = getGrayscaleHist(i, _numBins);

  double low = brightness.percentile(20);
  double high = brightness.percentile(80);

  return (high - low) * 100;
  //return ((high - low) / brightness.avg()) * 100;
}
