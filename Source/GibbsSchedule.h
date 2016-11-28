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
  Sampler(DeviceSet affectedDevices);

  virtual void sample(Snapshot* state) = 0;

protected:
  DeviceSet _devices;
};

// A color sampler distributes colors across the specified devices
// Requires: list of colors (HSV format, normalized between 0-1), list of corresponding weights
class ColorSampler : Sampler {
public:
  ColorSampler(DeviceSet affectedDevices, vector<Eigen::Vector3d> colors, vector<float> weights);
  ~ColorSampler();

  void sample(Snapshot* state) override;

private:
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

  void setIntensDist(vector<normal_distribution<float> > dists);
  void setColorDists(vector<normal_distribution<float> > hue, vector<normal_distribution<float> > sat, vector<normal_distribution<float> > val);

  // samples the intensity according to the current schedule
  void sampleIntensity(vector<float>& result, const vector<GibbsConstraint> constraints);

  // samples the color according to the current schedule
  void sampleColor(vector<float>& result, const vector<GibbsConstraint> constraints);

  // sample from the specified distribution
  double sampleSat(int id);
  double sampleHue(int id);

  // mixing some parameters here while the final interface gets figured out
  double _avgIntens;
  double _maxIntens;
  int _numBrightLights;
  bool _useSystems;
  vector<float> _colorWeights;

private:
  default_random_engine _gen;

  // list of samplers involved in this schedule
  vector<Sampler*> _samplers;

  vector<normal_distribution<float> > _intensDists;

  // Color is a 3D param, and is specified with three different distributions
  // all sampled at the same time. It is assumed that the indicies match here.
  vector<normal_distribution<float> > _hueDists;
  vector<normal_distribution<float> > _satDists;
  vector<normal_distribution<float> > _valDists;
};




#endif  // GIBBSSCHEDULE_H_INCLUDED
