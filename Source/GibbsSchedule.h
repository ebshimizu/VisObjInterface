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

  void setIntensDist(vector<normal_distribution<float> > dists);
  void setColorDists(vector<normal_distribution<float> > hue, vector<normal_distribution<float> > sat, vector<normal_distribution<float> > val);

  // samples the intensity according to the current schedule
  void sampleIntensity(vector<float>& result, const vector<GibbsConstraint> constraints);

  // samples the color according to the current schedule
  void sampleColor(vector<float>& result, const vector<GibbsConstraint> constraints);

private:
  default_random_engine _gen;

  vector<normal_distribution<float> > _intensDists;

  // Color is a 3D param, and is specified with three different distributions
  // all sampled at the same time. It is assumed that the indicies match here.
  vector<normal_distribution<float> > _hueDists;
  vector<normal_distribution<float> > _satDists;
  vector<normal_distribution<float> > _valDists;

};




#endif  // GIBBSSCHEDULE_H_INCLUDED
