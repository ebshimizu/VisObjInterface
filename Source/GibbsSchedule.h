/*
  ==============================================================================

    GibbsSchedule.h
    Created: 2 Nov 2016 10:56:01am
    Author:  falindrith

  ==============================================================================
*/

#ifndef GIBBSSCHEDULE_H_INCLUDED
#define GIBBSSCHEDULE_H_INCLUDED

#include "globals.h"

// A sampler consists of the following:
// - A set of devices that are being sampled
// - A function that performs the sampling over a Snapshot
// Samplers are expected to be subclassed based on the needs for each sampling method.
class Sampler {
public:
  Sampler(DeviceSet affectedDevices, Rectangle<float> region);

  virtual void sample(Snapshot* state) = 0;

protected:
  // computes the per-system intensity based on affected devices
  void computeSystemSensitivity();

  DeviceSet _devices;
  Rectangle<float> _region;
  map<string, double> _systemSensitivity;
};

// A color sampler distributes colors across the specified devices
// Requires: list of colors (HSV format, normalized between 0-1), list of corresponding weights
class ColorSampler : Sampler {
public:
  ColorSampler(DeviceSet affectedDevices,  Rectangle<float> region, vector<Eigen::Vector3d> colors, vector<float> weights);
  ~ColorSampler();

  void sample(Snapshot* state) override;

private:
  void normalizeWeights();

  vector<Eigen::Vector3d> _colors;
  vector<float> _weights;
};

// Gibbs schedules consist of a distribution for intensity parameters and a
// distribution for color parameters.
// Each distribution is a set of gaussians that are sampled from in a particular
// way. Right now, we'll keep it simple and have the distributions be selected from at
// random (i.e. uniformly).
// TODO: Support multivariate sampling distributions
class GibbsSchedule {
public:
  // create an empty schedule
  GibbsSchedule();
  
  // create a schedule from an image
  GibbsSchedule(Image& img);

  ~GibbsSchedule();

  // add a new sampler to the schedule
  void addSampler(Sampler* s);

  // removes all samplers from the schedule
  void deleteSamplers();

  // returns a new snapshot based off of some input state.
  // Snapshot returned is owned by calling scope and should be deleted there.
  Snapshot* sample(Snapshot* state);

private:
  default_random_engine _gen;

  // list of samplers involved in this schedule
  vector<Sampler*> _samplers;
};




#endif  // GIBBSSCHEDULE_H_INCLUDED
