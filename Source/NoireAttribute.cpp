/*
  ==============================================================================

    NoireAttribute.cpp
    Created: 6 Apr 2016 2:50:15pm
    Author:  falindrith

  ==============================================================================
*/

#include "NoireAttribute.h"
#include "BrightAttribute.h"

NoireAttribute::NoireAttribute() : HistogramAttribute("Noire", 50, 50)
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
  // goals: high contrast, low overall brightness, no front light,
  // one directional source per area, low overall saturation
  double score = 0;
  generateImage(s);

  Histogram1D brightness = getGrayscaleHist(100);
  double avgBrightness = (1-brightness.avg()) * 100;
  double low = brightness.percentile(20);
  double high = brightness.percentile(80);
  double contrast = ((high - low) / avgBrightness) * 100;
  
  double penalties = 0;

  // calculate penalties from having conflicting cross lights 
  for (auto& a : _areas) {
    // look for front lights, penalize based on absolute intensity
    // front intensity. Penalty may be positive.
    double fintens = getAreaIntens(s, a, "front") / 1000.0;
    double crossPenalty = getAreaCrossPenalty(s, a);

    penalties += -fintens + crossPenalty * 100;
  }

  score = contrast - avgBrightness + penalties;
  return score;
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

double NoireAttribute::getAreaCrossPenalty(Snapshot* s, string area) {
  double lintens = 0;
  double rintens = 0;

  auto& sd = s->getRigData();

  auto rdevices = _cache[area + "_right"].getDevices();
  auto ldevices = _cache[area + "_left"].getDevices();

  // calculate total intensities
  for (auto& d : rdevices) {
    rintens += d->getIntensity()->getVal();
  }

  for (auto& d : ldevices) {
    lintens += d->getIntensity()->getVal();
  }

  // determine difference between intensities, larger is better
  double diff = abs(rintens - lintens);

  // determine penalty
  // divide by 1000
  diff /= 1000.0;
  return (1 - exp(-diff)) / (1 + exp(-diff)) - 0.5;
}