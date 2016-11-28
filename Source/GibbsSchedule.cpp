/*
  ==============================================================================

    GibbsSchedule.cpp
    Created: 2 Nov 2016 10:56:01am
    Author:  falindrith

  ==============================================================================
*/

#include "GibbsSchedule.h"
#include "HistogramAttribute.h"
#include "closure_over_uneven_buckets.h"

Sampler::Sampler(DeviceSet affectedDevices, Rectangle<float> region) : _devices(affectedDevices), _region(region)
{
}

void Sampler::computeSystemSensitivity()
{
  int imgWidth = 100;
  int imgHeight = 100;
  Rectangle<int> cropRegion = Rectangle<int>(_region.getX() * imgWidth, _region.getY() * imgHeight,
    _region.getWidth() * imgWidth, _region.getHeight() * imgHeight);

  _systemSensitivity.clear();
  DeviceSet allDevices = getRig()->getAllDevices();
  Snapshot* tmp = new Snapshot(getRig());
  auto tmpData = tmp->getRigData();

  // systems
  auto systems = getRig()->getMetadataValues("system");
  for (auto system : systems) {
    DeviceSet fullSystem = getRig()->select("$system=" + system);
    DeviceSet active(getRig());

    // gather devices
    for (auto id : getRig()->getAllDevices().getIds()) {
      if (_devices.contains(id)) {
        active = active.add(id);
      }
    }

    // render image at 50% with selected devices
    for (auto id : allDevices.getIds()) {
      if (active.contains(id)) {
        tmpData[id]->getIntensity()->setValAsPercent(0.5);

        if (tmpData[id]->paramExists("color")) {
          tmpData[id]->setColorRGBRaw("color", 1, 1, 1);
        }
      }
      else {
        tmpData[id]->setIntensity(0);
      }
    }

    // render
    Image base = renderImage(tmp, imgWidth, imgHeight).getClippedImage(cropRegion);

    // adjust to 51%
    for (auto id : active.getIds()) {
      tmpData[id]->getIntensity()->setValAsPercent(0.51);
    }

    // render
    Image brighter = renderImage(tmp, imgWidth, imgHeight).getClippedImage(cropRegion);

    // calculate avg per-pixel brightness difference
    double diff = 0;
    for (int y = 0; y < cropRegion.getHeight(); y++) {
      for (int x = 0; x < cropRegion.getWidth(); x++) {
        diff += brighter.getPixelAt(x, y).getBrightness() - base.getPixelAt(x, y).getBrightness();
      }
    }

    _systemSensitivity[system] = diff / (base.getHeight() * base.getWidth()) * 100;
  }
}

// =============================================================================

ColorSampler::ColorSampler(DeviceSet affectedDevices, Rectangle<float> region,
  vector<Eigen::Vector3d> colors, vector<float> weights) :
  Sampler(affectedDevices, region), _colors(colors), _weights(weights)
{
  normalizeWeights();
}

ColorSampler::~ColorSampler()
{
}

void ColorSampler::sample(Snapshot * state)
{
  // the color sampler will sample by system
  vector<float> results;
  vector<DeviceSet> systemMap;
  auto stateData = state->getRigData();

  auto systems = getRig()->getMetadataValues("system");
  for (auto s : systems) {
    DeviceSet localSys(getRig());
    DeviceSet globalSys = getRig()->select("$system=" + s);

    float avgIntens = 0;
    for (auto id : globalSys.getIds()) {
      if (_devices.contains(id)) {
        localSys = localSys.add(id);
        avgIntens += stateData[id]->getIntensity()->asPercent();
      }
    }
    
    if (localSys.size() > 0) {
      avgIntens /= localSys.size();
      results.push_back(avgIntens);
      systemMap.push_back(localSys);
    }
  }

  vector<int> colorIds;
  colorIds.resize(results.size());

  // do the sampling
  ClosureOverUnevenBuckets(results, _weights, colorIds);

  // fill in the results
  for (int i = 0; i < results.size(); i++) {
    Eigen::Vector3d color = _colors[colorIds[i]];

    // apply color to each light in the system
    for (string id : systemMap[i].getIds()) {
      if (stateData[id]->paramExists("color")) {
        stateData[id]->getColor()->setHSV(color[0] * 360.0, color[1], color[2]);
      }
    }
  }
}

void ColorSampler::normalizeWeights()
{
  // compute sum
  double sum = 0;
  for (auto w : _weights) {
    sum += w;
  }

  // divide
  for (int i = 0; i < _weights.size(); i++) {
    _weights[i] /= sum;
  }
}

// =============================================================================

GibbsSchedule::GibbsSchedule()
{
}

GibbsSchedule::GibbsSchedule(Image & img)
{
}

GibbsSchedule::~GibbsSchedule()
{
  deleteSamplers();
}

void GibbsSchedule::addSampler(Sampler * s)
{
  _samplers.push_back(s);
}

void GibbsSchedule::deleteSamplers()
{
  for (auto s : _samplers)
    delete s;
  _samplers.clear();
}

Snapshot * GibbsSchedule::sample(Snapshot * state)
{
  // TODO: implement the sampling for real. Right now we simply sample things
  // iteratively. At some point we'll probably want to sample intensity then color
  // so the color sampler can actually do things? it may get complicated
  Snapshot* newState = new Snapshot(*state);

  for (auto s : _samplers) {
    // state carries over to each new sampler
    // may have to check for conflicts before running at some point
    s->sample(newState);
  }

  return newState;
}