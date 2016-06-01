/*
  ==============================================================================

    NoireAttribute.cpp
    Created: 6 Apr 2016 2:50:15pm
    Author:  falindrith

  ==============================================================================
*/

#include "NoireAttribute.h"
#include "BrightAttribute.h"

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

  // Targeting 75% dark (below 15%) and 10% bright (above 80%)
  // remaining 10% can be anywhere
  double pctBelow15 = brightness.percentLessThan(15);
  double pctAbove80 = brightness.percentGreaterThan(80);

  double darkScore = min(1.0, 1 - ((.75 - pctBelow15) / .75));
  double brightScore = min(1.0, 1 - ((.1 - pctAbove80) / .1));

  return ((darkScore * 0.4 + brightScore * 0.6) / 2.0) * 100;
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
    if (d->metadataExists("brightnessAttributeWeight")) {
      _weights[d->getId()] = stof(d->getMetadata("brightnessAttributeWeight"));
    }
  }
}

double NoireAttribute::getAreaIntens(Snapshot * s, string area, string angle)
{
  double intens = 0;
  auto& devices = _cache[area + "_" + "angle"].getDevices();
  auto& sd = s->getRigData();

  for (auto& d : devices) {
    intens += sd[d->getId()]->getIntensity()->getVal();
  }

  return intens;
}

double NoireAttribute::getRelativeAreaIntens(string area, string angle)
{
  double relIntens = 0;
  auto devices = _cache[area + "_" + angle].getDevices();

  for (auto& d : devices) {
    relIntens += _weights[d->getId()] * d->getIntensity()->asPercent();
  }

  return relIntens;
}

double NoireAttribute::getAreaWeightTotal(string area, string angle)
{
  double relIntens = 0;
  auto devices = _cache[area + "_" + angle].getDevices();

  for (auto& d : devices) {
    relIntens += _weights[d->getId()];
  }

  return relIntens;
}

double NoireAttribute::getAreaCrossPenalty(Snapshot* s, string area) {
  double lintens = getRelativeAreaIntens(area, "left");
  double rintens = getRelativeAreaIntens(area, "right");

  string minAngle = (lintens > rintens) ? "right" : "left";

  if (getAreaWeightTotal(area, minAngle) == 0)
    return 0;

  double propIntens = min(lintens, rintens) / getAreaWeightTotal(area, minAngle);

  // penalize based on how much the min angle is messing up the max angle
  return -propIntens;
}