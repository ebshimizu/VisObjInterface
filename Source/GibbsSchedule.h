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
  Sampler(DeviceSet affectedDevices, Rectangle<float> region, set<string> intensPins, set<string> colorPins);

  // every sampler has to have a sample function
  virtual void sample(Snapshot* state) = 0;
  
  // evaluation function for each sampler
  virtual double score(Snapshot* state, Image& img) = 0;

  // name for id'ing the sampler
  string _name;
protected:
  // computes the per-system intensity based on affected devices
  void computeSystemSensitivity();

  DeviceSet _devices;
  Rectangle<float> _region;
  map<string, double> _systemSensitivity;
  set<string> _intensPins;
  set<string> _colorPins;
};

// A color sampler distributes colors across the specified devices
// Requires: list of colors (HSV format, normalized between 0-1), list of corresponding weights
class ColorSampler : public Sampler {
public:
  ColorSampler(DeviceSet affectedDevices, Rectangle<float> region,
    set<string> intensPins, set<string> colorPins,
    vector<Eigen::Vector3d> colors, vector<float> weights);
  ~ColorSampler();

  void sample(Snapshot* state) override;

  // returns a score based on..... something
  // TODO: determine what the something is
  double score(Snapshot* state, Image& img);

private:
  void normalizeWeights();
  int getClosestColorIndex(Eigen::Vector3d color);

  vector<Eigen::Vector3d> _colors;
  vector<float> _weights;
};

// A pin sampler samples all the pinned lights.
class PinSampler : public Sampler {
public:
  PinSampler(DeviceSet affectedDevices, Rectangle<float> region, set<string> intensPins, set<string> colorPins);
  ~PinSampler();

  void sample(Snapshot* state) override;
  
  // the pin sampler won't really ever have a useful score
  double score(Snapshot* /* state */, Image& /* img */) { return 0; }
};

// an intensity sampler samples a target average intensity and peak intensity
// using a few priors at the moment
// TODO: determine better intensity sampling method from image
class IntensitySampler : public Sampler {
public:
  IntensitySampler(DeviceSet affectedDevices, Rectangle<float> region, set<string> intensPins, set<string> colorPins,
    int k, float bm, float m);
  ~IntensitySampler();

  void sample(Snapshot* state) override;

  double score(Snapshot* state, Image& img) override;

  void setBrightnessHistogram(SparseHistogram b);

private:
  // for computing the score, the histogram of the idea
  SparseHistogram _srcBrightness;

  // parameters for sampling
  int _k;
  float _brightMean;
  float _mean;
};

// GibbsSchedules consist of a set of samplers that take the current state
// and return a new sample that may or may not be connected to that current state
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

  // adds score data to the result for each sampler in the schedule
  void score(shared_ptr<SearchResult> result, Snapshot* state);

  // returns a new snapshot based off of some input state.
  // Snapshot returned is owned by calling scope and should be deleted there.
  Snapshot* sample(Snapshot* state);

private:
  default_random_engine _gen;

  // list of samplers involved in this schedule
  vector<Sampler*> _samplers;
};




#endif  // GIBBSSCHEDULE_H_INCLUDED
