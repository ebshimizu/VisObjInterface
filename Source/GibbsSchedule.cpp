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
#include "gibbs_with_gaussian_mixture.h"

Sampler::Sampler(DeviceSet affectedDevices, Rectangle<float> region, set<string> intensPins, set<string> colorPins) :
  _devices(affectedDevices), _region(region), _intensPins(intensPins), _colorPins(colorPins)
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
  set<string> intensPins, set<string> colorPins,
  vector<Eigen::Vector3d> colors, vector<float> weights) :
  Sampler(affectedDevices, region, intensPins, colorPins), _colors(colors), _weights(weights)
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
  vector<float> bins;
  bins.resize(_colors.size());
  vector<DeviceSet> systemMap;
  auto stateData = state->getRigData();

  // ok so here we'll want to do some pre-filling based on what's pinned
  // process goes like this
  // - find light's closest color if pinned, add intensity (%) to bin
  // - If not pinned, add light's intensity to system total

  auto systems = getRig()->getMetadataValues("system");
  int i = 0;
  for (auto s : systems) {
    DeviceSet localSys(getRig());
    DeviceSet globalSys = getRig()->select("$system=" + s);

    float totalIntens = 0;
    for (auto id : globalSys.getIds()) {
      if (_devices.contains(id)) {
        if (!stateData[id]->paramExists("color"))
          continue;

        if (_colorPins.count(id) == 0) {
          // unconstrained
          localSys = localSys.add(id);
          totalIntens += stateData[id]->getIntensity()->asPercent();
        }
        else {
          // pinned, find closest color
          int closestColorIndex = getClosestColorIndex(stateData[id]->getColor()->getHSV());

          // add to color bin, note that this is not actually part of the
          // local system set because it's pinned
          bins[closestColorIndex] += stateData[id]->getIntensity()->asPercent();
        }
      }
    }
    
    // ensure proper size for results, even if the value is 0
    results.push_back(totalIntens);
    systemMap.push_back(localSys);
    i++;
  }

  vector<int> colorIds;
  colorIds.resize(results.size());

  // do the sampling
  ClosureOverUnevenBuckets(results, _weights, colorIds, bins);

  // fill in the results if not pinned
  for (int i = 0; i < results.size(); i++) {
    Eigen::Vector3d color = _colors[colorIds[i]];

    // perterb the color a little bit
    std::random_device rd;
    std::mt19937 gen(rd());
    normal_distribution<double> hueDist(0, 0.02);
    normal_distribution<double> satDist(0, 0.1);

    color[0] += hueDist(gen);
    color[1] += satDist(gen);
    color[2] += satDist(gen);

    // apply color to each light in the system
    for (string id : systemMap[i].getIds()) {
      if (_colorPins.count(id) == 0 && stateData[id]->paramExists("color")) {
          stateData[id]->getColor()->setHSV(color[0] * 360.0, color[1], color[2]);
      }
    }
  }
}

double ColorSampler::score(Snapshot * state, Image & img, bool masked)
{
  return 0.0;
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

int ColorSampler::getClosestColorIndex(Eigen::Vector3d color)
{
  int closest = 0;
  double closestDist = DBL_MAX;
  
  // normalization factor. Lumiverse returns hue in [0-360]
  color[0] /= 360.0;

  for (int i = 0; i < _colors.size(); i++) {
    double dist = (color - _colors[i]).norm();
    if (dist < closestDist) {
      closest = 0;
      closestDist = dist;
    }
  }
  
  return closest;
}

// =============================================================================
PinSampler::PinSampler(DeviceSet affectedDevice, Rectangle<float> region, set<string> intensPins, set <string> colorPins) :
  Sampler(affectedDevice, region, intensPins, colorPins)
{

}

PinSampler::~PinSampler()
{
}

void PinSampler::sample(Snapshot * state)
{
  // the pin sampler takes the pinned values and 'wiggles' them a bit, showing small
  // variations of the current state of the device parameter
  std::random_device rd;
  std::mt19937 gen(rd());
  normal_distribution<double> hueDist(0, 2);
  normal_distribution<double> satDist(0, 0.05);
  normal_distribution<double> intensDist(0, 0.05);

  auto stateData = state->getRigData();

  // intensity
  for (auto id : _intensPins) {
    if (stateData[id]->paramExists("intensity")) {
      double newIntens = stateData[id]->getIntensity()->asPercent() + intensDist(gen);
      stateData[id]->getIntensity()->setValAsPercent(newIntens);
    }
  }

  // color
  for (auto id : _colorPins) {
    if (stateData[id]->paramExists("color")) {
      Eigen::Vector3d hsv = stateData[id]->getColor()->getHSV();
      hsv[0] += hueDist(gen);
      hsv[1] += satDist(gen);

      stateData[id]->getColor()->setHSV(hsv[0], hsv[1], hsv[2]);
    }
  }
}

// =============================================================================

IntensitySampler::IntensitySampler(DeviceSet affectedDevices, Rectangle<float> region,
  set<string> intensPins, set<string> colorPins, int k, float bm, float m) :
  Sampler(affectedDevices, region, intensPins, colorPins), _k(k), _brightMean(bm), _mean(m), _srcBrightness(1, { 0, 0.1f })
{
  computeSystemSensitivity();
}

IntensitySampler::~IntensitySampler()
{
}

void IntensitySampler::sample(Snapshot * state)
{
  // the color sampler will sample by system
  vector<float> results;
  vector<int> constraint;
  vector<float> sens;
  vector<DeviceSet> systemMap;
  auto stateData = state->getRigData();

  auto systems = getRig()->getMetadataValues("system");
  int i = 0;

  for (auto system : systems) {
    DeviceSet globalSys = getRig()->select("$system=" + system);
    DeviceSet localSys(getRig());

    for (auto id : globalSys.getIds()) {
      if (_devices.contains(id)) {
        if (_intensPins.count(id) == 0) {
          localSys = localSys.add(id);
        }
        // some other effect if pinned
      }
    }

    results.push_back(0);
    sens.push_back(_systemSensitivity[system]);
    systemMap.push_back(localSys);

    if (localSys.size() > 0) {
      constraint.push_back(0);
    }
    else {
      // pinned
      constraint.push_back(3);
    }
  }

  // sample
  GibbsSamplingGaussianMixturePrior(results, constraint, sens, results.size(), _k, _brightMean, _mean,
    getGlobalSettings()->_maxAllowedLights, getGlobalSettings()->_maxLightPenalty);

  // apply to snapshot
  for (int i = 0; i < results.size(); i++) {
    float intens = results[i];

    // apply color to each light in the system
    for (string id : systemMap[i].getIds()) {
      if (_intensPins.count(id) == 0 && stateData[id]->paramExists("intensity")) {
        stateData[id]->getIntensity()->setValAsPercent(intens);
      }
    }
  }
}

double IntensitySampler::score(Snapshot * state, Image& img, bool masked)
{
  // compute histogram from scaled region
  Rectangle<int> region = Rectangle<int>(_region.getX() * img.getWidth(), _region.getY() * img.getHeight(),
    _region.getWidth() * img.getWidth(), _region.getHeight() * img.getHeight());
  Image clipped = img.getClippedImage(region);
  Image clippedMask = getGlobalSettings()->_fgMask.rescaled(img.getWidth(), img.getHeight()).getClippedImage(region);

  // scale, 200 max dim
  float scale = (clipped.getWidth() > clipped.getHeight()) ? 200.0f / clipped.getWidth() : 200.0f / clipped.getHeight();
  clipped = clipped.rescaled(scale * clipped.getWidth(), scale * clipped.getHeight());
  clippedMask = clippedMask.rescaled(clipped.getWidth(), clipped.getHeight());

  // do the thing
  SparseHistogram stage(1, _srcBrightness.getBounds());

  for (int y = 0; y < clipped.getHeight(); y++) {
    for (int x = 0; x < clipped.getWidth(); x++) {
      if (masked && getGlobalSettings()->_useFGMask) {
        if (clippedMask.getPixelAt(x, y).getBrightness() > 0) {
          Colour px = clipped.getPixelAt(x, y);
          vector<float> pts;
          pts.push_back(px.getBrightness());
          stage.add(pts);
        }
      }
      else {
        // add everything
        Colour px = clipped.getPixelAt(x, y);
        vector<float> pts;
        pts.push_back(px.getBrightness());
        stage.add(pts);
      }
    }
  }

  // compute the dist
  return _srcBrightness.EMD(stage);
}
void IntensitySampler::setBrightnessHistogram(SparseHistogram b)
{
  _srcBrightness = b;
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

void GibbsSchedule::score(shared_ptr<SearchResult> result, Snapshot* state)
{
  int width = getGlobalSettings()->_renderWidth;
  int height = getGlobalSettings()->_renderHeight;
  Image render = renderImage(state, getGlobalSettings()->_thumbnailRenderScale * width,
    getGlobalSettings()->_thumbnailRenderScale * height);

  for (auto s : _samplers) {
    if (s->_name == "Pinned")
      continue;

    result->_extraFuncs[s->_name] = s->score(state, render, false);
    result->_extraFuncs[s->_name + "_masked"] = s->score(state, render, true);
  }
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