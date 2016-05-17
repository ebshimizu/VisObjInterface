/*
  ==============================================================================

    MoonlightAttribute.cpp
    Created: 26 Apr 2016 4:55:46pm
    Author:  falindrith

  ==============================================================================
*/

#include "MoonlightAttribute.h"

MoonlightAttribute::MoonlightAttribute(int w, int h) : HistogramAttribute("Moonlight", w, h)
{
}

MoonlightAttribute::~MoonlightAttribute()
{
}

double MoonlightAttribute::evaluateScene(Snapshot * s)
{
  // moonlight is defined by the following things (accodring to this attribute):
  // -strong single system light source that is a pale white-blue
  // -overall low brightness
  // -overall cool color tone
  Image i = generateImage(s);

  // determine brightest system (according to proportional _light_ intensities)
  // brightest system is defined by the brightest light in the system. there will be a penalty
  // for systems with uneven lighting for this case
  string brightest;
  float max = 0;

  for (const auto& sys : _systemCache) {
    float brightness = getMaxSystemBrightness(sys.first, s);
    if (brightness > max) {
      brightest = sys.first;
      max = brightness;
    }
  }

  // calculate score based on deviation from average system brightness
  // (moonlight is usually not uneven)
  float avgBrScore = getAvgSystemBrightness(brightest, s) * 100;

  // also add score based on color
  // there's a target HSV and a radius. If within radius, no penalty, otherwise penalty based on squared
  // distance
  auto bHSV = getAvgColor(brightest, s);

  float hueScore = abs(bHSV[0] - 180) <= 20 ? 100 : 100 - min(abs(bHSV[0] - 200), abs(bHSV[0] - 160));
  float satScore = bHSV[1] > 0.2 ? 100 - (bHSV[1] - 0.2 * 100) : 100;

  // low contrast (other lights) penalty
  float other = 0;
  for (const auto& sys : _systemCache) {
    if (sys.first == brightest)
      continue;

    float brightness = getAvgSystemBrightness(sys.first, s);
    other += brightness;
  }

  float otherBrScore = 0;

  if (other > 0.1)
    otherBrScore = -(other - 0.1) * 100;

  // penalty for scene being too bright
  Histogram1D bright = getGrayscaleHist(i, 20);
  float overallBrScore = bright.avg() * 100;

  return avgBrScore * 0.25 + hueScore * 0.15 + satScore * 0.25 + otherBrScore * 0.25 + overallBrScore * 0.1;
}

void MoonlightAttribute::preProcess()
{
  auto systems = getRig()->getMetadataValues("system");

  for (auto& s : systems) {
    _systemCache[s] = getRig()->select("$system=" + s);
  }
}

float MoonlightAttribute::getMaxSystemBrightness(string sys, Snapshot * s)
{
  auto devices = _systemCache[sys].getDevices();
  auto r = s->getRigData();

  float max = 0;
  for (auto& d : devices) {
    if (r[d->getId()]->getIntensity()->asPercent() > max)
      max = r[d->getId()]->getIntensity()->asPercent();
  }
  return max;
}

float MoonlightAttribute::getAvgSystemBrightness(string sys, Snapshot * s)
{
  auto r = s->getRigData();
  auto devices = _systemCache[sys].getDevices();

  float avg = 0;
  for (auto& d : devices) {
    avg += r[d->getId()]->getIntensity()->asPercent();
  }
  return avg / devices.size();
}

Eigen::Vector3d MoonlightAttribute::getAvgColor(string sys, Snapshot * s)
{
  auto r = s->getRigData();
  auto devices = _systemCache[sys].getDevices();

  Eigen::Vector3d avg(0, 0, 0);
  for (auto& d : devices) {
    avg += r[d->getId()]->getColor()->getHSV();
  }
  return avg / devices.size();
}
