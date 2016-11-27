/*
  ==============================================================================

    GibbsSchedule.cpp
    Created: 2 Nov 2016 10:56:01am
    Author:  falindrith

  ==============================================================================
*/

#include "GibbsSchedule.h"
#include "HistogramAttribute.h"

Sampler::Sampler(DeviceSet affectedDevices) : _devices(affectedDevices)
{
}

// =============================================================================

ColorSampler::ColorSampler(DeviceSet affectedDevices, vector<Eigen::Vector3d> colors, vector<float> weights) :
  Sampler(affectedDevices), _colors(colors), _weights(weights)
{
}

ColorSampler::~ColorSampler()
{
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

void GibbsSchedule::setIntensDist(vector<normal_distribution<float>> dists)
{
  _intensDists = dists;
}

void GibbsSchedule::setColorDists(vector<normal_distribution<float>> hue, vector<normal_distribution<float>> sat, vector<normal_distribution<float>> val)
{
  _hueDists = hue;
  _satDists = sat;
  _valDists = val;
}

void GibbsSchedule::sampleIntensity(vector<float>& result, const vector<GibbsConstraint> constraints)
{
  // check that result and constraints are the same length
  if (result.size() != constraints.size())
    return;

  // constraints indicates which parameters can be modified and how.
  
  // TODO: FIX FOR DIFFERENT SCHEDULES
  uniform_int_distribution<int> rng(0, _intensDists.size() - 1);

  for (int i = 0; i < result.size(); i++) {
    if (constraints[i] == FREE) {
      result[i] = Lumiverse::clamp(_intensDists[rng(_gen)](_gen), 0.0f, 1.0f);
    }
  }
}

void GibbsSchedule::sampleColor(vector<float>& result, const vector<GibbsConstraint> constraints)
{
  // check that result / 3 and constraints are the same length
  if (result.size() / 3 != constraints.size())
    return;

  // constraints indicates which parameters can be modified and how.
  
  // TODO: FIX FOR DIFFERENT SCHEDULES
  uniform_int_distribution<int> rng(0, _hueDists.size() - 1);

  for (int i = 0; i < constraints.size(); i++) {
    if (constraints[i] == FREE) {
      int dist = rng(_gen);
      result[i * 3] = _hueDists[dist](_gen);
      result[i * 3 + 1] = Lumiverse::clamp(_satDists[dist](_gen), 0, 1);
      result[i * 3 + 2] = Lumiverse::clamp(_valDists[dist](_gen), 0, 1);
    }
  }
}

double GibbsSchedule::sampleSat(int id)
{
  return _satDists[id](_gen);
}

double GibbsSchedule::sampleHue(int id)
{
  return _hueDists[id](_gen);
}

