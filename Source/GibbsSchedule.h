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
  virtual double score(Snapshot* state, Image& img, bool masked) = 0;

  // returns the affected region of the sampler
  Rectangle<float> getRegion();

  // returns string contining the settings and affected devices in the sampler;
  virtual string info() = 0;

  // name for id'ing the sampler
  string _name;

  // visual concept image
  Image _concept;

  virtual string getType() = 0;
protected:
  // computes the per-system intensity based on affected devices
  void computeSystemSensitivity();

  // lists ids of affected devices
  string getAffectedDevices();

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

  virtual void sample(Snapshot* state) override;

  // returns a score based on..... something
  // TODO: determine what the something is
  virtual double score(Snapshot* state, Image& img, bool masked);

  void setColorHistogram(SparseHistogram c);

  string info() override;
  string getType() { return "color"; }

protected:
  void normalizeWeights();
  int getClosestColorIndex(Eigen::Vector3d color);

  vector<Eigen::Vector3d> _colors;
  vector<float> _weights;

  // for computing a score
  SparseHistogram _srcColor;
};

// A pin sampler samples all the pinned lights.
class PinSampler : public Sampler {
public:
  PinSampler(DeviceSet affectedDevices, Rectangle<float> region, set<string> intensPins, set<string> colorPins);
  ~PinSampler();

  void sample(Snapshot* state) override;
  
  // the pin sampler won't really ever have a useful score
  double score(Snapshot* /* state */, Image& /* img */, bool /* masked */) { return 0; }

  string info() override;
  string getType() { return "pin"; }
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

  double score(Snapshot* state, Image& img, bool masked) override;

  void setBrightnessHistogram(SparseHistogram b);

  string info() override;
  string getType() { return "intensity"; }

private:
  // for computing the score, the histogram of the idea
  SparseHistogram _srcBrightness;

  // parameters for sampling
  int _k;
  float _brightMean;
  float _mean;
};

class MonochromeSampler : public Sampler {
public:
  MonochromeSampler(DeviceSet affectedDevices, Rectangle<float> region, set<string> intensPins,
    set<string> colorPins, Colour color);
  ~MonochromeSampler();

  void sample(Snapshot* state) override;

  double score(Snapshot* state, Image& img, bool masked) override;

  string info() override;

  string getType() { return "monochrome"; }
private:
  Colour _target;
};

class TheatricalSampler : public ColorSampler {
public:
  TheatricalSampler(DeviceSet affectedDevices, Rectangle<float> region,
    set<string> intensPins, set<string> colorPins,
    vector<Eigen::Vector3d> colors, vector<float> weights);
  ~TheatricalSampler();

  void sample(Snapshot* state) override;
  string getType() { return "theatrical"; }

private:
  Eigen::Vector3d cctToRgb(int cct);

  DeviceSet front;
};

// GibbsSchedules consist of a set of samplers that take the current state
// and return a new sample that may or may not be connected to that current state
class GibbsSchedule {
public:
  // create an empty schedule
  GibbsSchedule();
  
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

  // logs info about all the active samplers to a file
  void log();
  
  // makes sure intensity samplers happen first.
  void moveIntensityUp();

private:
  default_random_engine _gen;

  // list of samplers involved in this schedule
  vector<Sampler*> _samplers;
};




#endif  // GIBBSSCHEDULE_H_INCLUDED
