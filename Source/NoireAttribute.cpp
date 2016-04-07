/*
  ==============================================================================

    NoireAttribute.cpp
    Created: 6 Apr 2016 2:50:15pm
    Author:  falindrith

  ==============================================================================
*/

#include "NoireAttribute.h"
#include "BrightAttribute.h"

NoireAttribute::NoireAttribute() : AttributeControllerBase("Noire")
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
  double score = 0;

  // measure strength of noire-ness in all areas, take weighted average
  for (auto& a : _areas) {
    // calculate weight for area
    auto devices = _cache[a].getDevices();
    double totalWeight = 0;
    for (auto& d : devices) {
      totalWeight += _weights[d->getId()];
    }

    score += getScoreForArea(s, a) * totalWeight;
  }

  return score * 100;
}

void NoireAttribute::preProcess()
{
  // gather lights into groups for use later
  _areas = getRig()->getMetadataValues("area");

  for (auto& a : _areas) {
    _cache[a + "_left"] = getRig()->select("$area=" + a + "[$angle=left]");
    _cache[a + "_right"] = getRig()->select("$area=" + a + "[$angle=right]");
    _cache[a + "_top"] = getRig()->select("$area=" + a + "[$angle=top]");
    _cache[a + "_front"] = getRig()->select("$area=" + a + "[$angle=front]");
    _cache[a] = getRig()->select("$area=" + a);
  }

  auto& devices = getRig()->getDeviceRaw();

  // this attribute attempts to find the pre-computed brightnessAttributeWeight if it exists
  // if it doesn't we kinda just cheat a bit by creating a brightness attribute and preprocessing it
  // and then copying the values resulting from that.
  bool weightsExist = true;
  for (const auto& d : devices) {
    if (!d->metadataExists("brightnessAttributeWeight")) {
      weightsExist = false;
      break;
    }
  }

  if (!weightsExist) {
    // make those things exist
    BrightAttribute* ba = new BrightAttribute("NOIRE HELPER");
    ba->preProcess();
    delete ba;
  }

  // now that the metadata exists, copy it
  for (const auto& d : devices) {
    _weights[d->getId()] = stof(d->getMetadata("brightnessAttributeWeight"));
  }
}

double NoireAttribute::getScoreForArea(Snapshot* s, string a)
{
  // returns a score for the given area

  // calculate average top, left, right, and front brightness (purely light based)
  double topAvg = getAvgIntensity(s, a, "top");
  double leftAvg = getAvgIntensity(s, a, "left");
  double rightAvg = getAvgIntensity(s, a, "right");
  double frontAvg = getAvgIntensity(s, a, "front");

  // score compute time.
  // side score is diff between brightest of left/right (abs diff)
  double sideScore = abs(leftAvg - rightAvg);

  // contrast score measures brightness difference between top and brightest side (linear abs diff)
  double contrastScore = abs(topAvg - ((leftAvg > rightAvg) ? leftAvg : rightAvg));

  // total score is side + contrast - front
  return sideScore + contrastScore - frontAvg;
}

double NoireAttribute::getAvgIntensity(Snapshot * s, string area, string angle)
{
  string cacheId = area + "_" + angle;
  auto& rigData = s->getRigData();
  auto devices = _cache[cacheId].getDevices();

  // abort if no devices
  if (devices.size() == 0)
    return 0;

  // devices should be weighted according to their relative influence within their areas
  // so we should recompute weights here
  double areaWeightTotal = 0;
  auto areaDevices = _cache[area].getDevices();
  for (auto& d : areaDevices) {
    areaWeightTotal += _weights[d->getId()];
  }

  double avg = 0;
  for (auto& d : devices) {
    avg += rigData[d->getId()]->getIntensity()->asPercent() * (_weights[d->getId()] / areaWeightTotal);
  }

  return avg / devices.size();
}
