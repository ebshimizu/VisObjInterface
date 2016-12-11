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
  Sampler(affectedDevices, region, intensPins, colorPins), _colors(colors), _weights(weights),
  _srcColor(3, { 0, 0.05f, 0, 0.2f, 0, 0.2f })
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
  SparseHistogram stage(3, _srcColor.getBounds());

  for (int y = 0; y < clipped.getHeight(); y++) {
    for (int x = 0; x < clipped.getWidth(); x++) {
      if (masked && getGlobalSettings()->_useFGMask) {
        if (clippedMask.getPixelAt(x, y).getBrightness() > 0) {
          Colour px = clipped.getPixelAt(x, y);
          vector<float> pts;
          pts.resize(3);
          px.getHSB(pts[0], pts[1], pts[2]);
          stage.add(pts);
        }
      }
      else {
        // add everything
        Colour px = clipped.getPixelAt(x, y);
        vector<float> pts;
        pts.resize(3);
        px.getHSB(pts[0], pts[1], pts[2]);
        stage.add(pts);
      }
    }
  }

  // compute the dist
  return _srcColor.EMD(stage);
}

void ColorSampler::setColorHistogram(SparseHistogram c)
{
  _srcColor = c;
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

MonochromeSampler::MonochromeSampler(DeviceSet affectedDevices, Rectangle<float> region,
  set<string> intensPins, set<string> colorPins, Colour color) :
  Sampler(affectedDevices, region, intensPins, colorPins), _target(color)
{
}

MonochromeSampler::~MonochromeSampler()
{
}

void MonochromeSampler::sample(Snapshot * state)
{
  // this one is pretty easy
  // we just pick a color close to the target, and adjust the satration a bit
  // the color sampler will sample by system
  vector<DeviceSet> systemMap;
  auto stateData = state->getRigData();

  auto systems = getRig()->getMetadataValues("system");
  int i = 0;

  for (auto system : systems) {
    DeviceSet globalSys = getRig()->select("$system=" + system);
    DeviceSet localSys(getRig());

    for (auto id : globalSys.getIds()) {
      if (_devices.contains(id)) {
        if (_colorPins.count(id) == 0) {
          localSys = localSys.add(id);
        }
      }
    }

    systemMap.push_back(localSys);
  }

  // sample
  std::random_device rd;
  std::mt19937 gen(rd());
  normal_distribution<double> hueDist(0, 0.01);
  normal_distribution<double> satDist(0, 0.2);
  float hue = _target.getHue() + hueDist(gen);
  float sat = _target.getSaturation() + satDist(gen);

  // apply to snapshot
  for (int i = 0; i < systemMap.size(); i++) {

    // apply color to each light in the system
    for (string id : systemMap[i].getIds()) {
      if (_colorPins.count(id) == 0 && stateData[id]->paramExists("color")) {
        stateData[id]->getColor()->setHSV(hue * 360, sat, _target.getBrightness());
      }
    }
  }
}

double MonochromeSampler::score(Snapshot * state, Image & img, bool masked)
{
  Eigen::Vector3f t;
  _target.getHSB(t[0], t[1], t[2]);

  float sum = 0;
  // per-pixel average difference from target
  for (int y = 0; y < img.getHeight(); y++) {
    for (int x = 0; x < img.getWidth(); x++) {
      Eigen::Vector3f px;
      img.getPixelAt(x, y).getHSB(px[0], px[1], px[2]);

      sum += (t - px).norm();
    }
  }

  return sum / (img.getHeight() * img.getWidth());
}

TheatricalSampler::TheatricalSampler(DeviceSet affectedDevices, Rectangle<float> region,
  set<string> intensPins, set<string> colorPins,
  vector<Eigen::Vector3d> colors, vector<float> weights) :
  ColorSampler(affectedDevices, region, intensPins, colorPins, colors, weights),
  front(getRig()) 
{
  DeviceSet other(getRig());

  // separate front devices
  for (auto id : _devices.getIds()) {
    string system = getRig()->getDevice(id)->getMetadata("system");
    if (system == "front" || system == "front left" || system == "front right") {
      front = front.add(id);
    }
    else {
      other = other.add(id);
    }
  }
  
  // update internal affected devices
  _devices = other;
}

TheatricalSampler::~TheatricalSampler()
{
}

void TheatricalSampler::sample(Snapshot * state)
{
  // this sampler handles the front lights, everything else is handled by the parent class
  auto data = state->getRigData();
  vector<DeviceSet> systemMap;

  vector<string> systems = { "front", "front left", "front right" };
  int i = 0;

  for (auto system : systems) {
    DeviceSet globalSys = getRig()->select("$system=" + system);
    DeviceSet localSys(getRig());

    for (auto id : globalSys.getIds()) {
      if (_devices.contains(id)) {
        if (_colorPins.count(id) == 0) {
          localSys = localSys.add(id);
        }
      }
    }

    systemMap.push_back(localSys);
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  uniform_int_distribution<int> ctb(6500, 12000);
  uniform_int_distribution<int> ctw(1900, 6500);
  uniform_int_distribution<int> all(1900, 12000);
  vector<Eigen::Vector3d> colors;
  colors.push_back(cctToRgb(ctb(gen)));
  colors.push_back(cctToRgb(ctw(gen)));
  colors.push_back(cctToRgb(all(gen)));

  // assign to systems in random order
  shuffle(systemMap.begin(), systemMap.end(), gen);

  // apply to snapshot
  for (int i = 0; i < systemMap.size(); i++) {
    Eigen::Vector3d color = colors[i];

    // apply color to each light in the system
    for (string id : systemMap[i].getIds()) {
      if (_colorPins.count(id) == 0 && data[id]->paramExists("color")) {
        data[id]->getColor()->setRGBRaw(color[0], color[1], color[2]);
      }
    }
  }

  // sample remaining colors
  ColorSampler::sample(state);
}

Eigen::Vector3d TheatricalSampler::cctToRgb(int cct)
{
  // based on http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/ 
  float temp = cct / 100.0f;

  // red
  float red = 0;
  if (temp <= 66) {
    red = 255;
  }
  else {
    red = temp - 60;
    red = 329.698727446 * pow(red, -0.1332047592);
    red = Lumiverse::clamp(red, 0, 255);
  }

  float green;
  if (temp <= 66) {
    green = temp;
    green = 99.4708025861 * log(green) - 161.1195681661;
  }
  else {
    green = temp - 60;
    green = 288.1221695283 * pow(green, -0.0755148492);
  }
  green = Lumiverse::clamp(green, 0, 255);

  float blue;
  if (temp >= 66) {
    blue = 255;
  }
  else {
    if (temp <= 19)
      blue = 0;
    else {
      blue = temp - 10;
      blue = 138.5177312231 * log(blue) - 305.0447927307;
    }
  }

  return Eigen::Vector3d(red / 255.0f, green / 255.0f, blue / 255.0f);
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

