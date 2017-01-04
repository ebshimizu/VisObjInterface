#pragma once
#ifndef _GIBBS_WITH_GAUSSIAN_MIXTURE_H_
#define _GIBBS_WITH_GAUSSIAN_MIXTURE_H_

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cassert>
#include <math.h>
#include "LumiverseCore.h"

// Gibbs sampling MCMC schedule to sample `k` number of directed lights in an
// `n` light system with custom target mean intensity for the directed lights
// and the entire system.
//
// Intensity is assumed to be in range [0 1]
//
// yumer, Oct-28-2016
//
//
// inputs:
// c  = constraints per light (0: no constraint, 1: sample from high, 2: sample from low)
// s  = approximate average sensitivity per light per pixel.
// n  = number of samples to generate (in our case, number of lights or
//      light systems to generate intensity for).
// k  = target number of directed (i.e. high intensity) lights.
// mh = target mean intensity of directed lights.
// ma = target average intensity of all lights combined.
//
// sh = sigma (square root of variance) for high intensity gaussian.
// sl = sigma (square root of variance) for low intensity gaussian.
//
void GibbsSamplingGaussianMixture(std::vector<float>& result,
  const std::vector<int>& c,
  const std::vector<float>& s,
  const int n,
  const int k,
  const float mh,
  const float ma,
  bool use_image_intensity = true,
  const float sh = 0.1,
  const float sl = 0.05);

// Gibbs sampler that accounts for current light state and limits the number of sampled
// lights according to some priors
void GibbsSamplingGaussianMixturePrior(std::vector<float>& result,
  const std::vector<int>& c,
  const std::vector<float>& s,
  const int n,
  const int k,
  const float mh,
  const float ma,
  bool use_image_intensity = true,
  const float sh = 0.1,
  const float sl = 0.05);
#endif
