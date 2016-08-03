/*
  ==============================================================================

    SaturationAttribute.cpp
    Created: 31 Mar 2016 10:48:48am
    Author:  falindrith

  ==============================================================================
*/

#include "SaturationAttribute.h"

SaturationAttribute::SaturationAttribute(int numBins, int w, int h) :
  _numBins(numBins), HistogramAttribute("Saturation", w, h)
{
  _autoLockParams.insert(HUE);
  _autoLockParams.insert(INTENSITY);
}

SaturationAttribute::~SaturationAttribute()
{
}

double SaturationAttribute::evaluateScene(Snapshot * s)
{
  Image i = generateImage(s);
  Histogram1D sat = getSatHist(i, _numBins);
  return sat.avg() * 100;
}

list<Snapshot*> SaturationAttribute::nonSemanticSearch()
{
  if (getStatus() == A_EQUAL || getStatus() == A_IGNORE)
    return list<Snapshot*>();

  // increase the saturation of each light by 0.05, do 10 times for good spread.
  Snapshot* start = new Snapshot(getRig());
  list<Snapshot*> results;
  
  for (int i = 0; i < 10; i++) {
    auto devices = start->getRigData();
    for (auto d : devices) {
      if (!isDeviceParamLocked(d.first, "color")) {
        auto color = d.second->getColor();
        if (color != nullptr) {
          Eigen::Vector3d hsv = color->getHSV();
          if (getStatus() == A_MORE) {
            color->setHSV(hsv[0], hsv[1] + 0.05, hsv[2]);
          }
          else if (getStatus() == A_LESS) {
            color->setHSV(hsv[0], hsv[1] - 0.05, hsv[2]);
          }
        }
      }
    }

    results.push_back(new Snapshot(*start));
  }

  return results;
}
