/*
  ==============================================================================

    gibbs_with_gaussian_mixture.cpp
    Created: 17 Nov 2016 2:57:11pm
    Author:  falindrith

  ==============================================================================
*/

#include "gibbs_with_gaussian_mixture.h"

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
  bool use_image_intensity,
  const float sh,
  const float sl)
{
  if (result.size() != n)
    result.resize(n);

  assert(c.size() == n);

  if (use_image_intensity)
    assert(s.size() == n);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> uniform_0_1(0, 1);
  std::normal_distribution<float> high_gaussian(mh, sh);
  float sample_mean = 0.0f;
  int sample_k = 0;
  int n_sampled = 0;

  // sample constrained lights
  for (int i = 0; i < n; ++i)
  {
    if (c[i] == 0)
      continue;

    int n_left = n - n_sampled;

    // Target number of directed lights
    int target_k = (k - sample_k > 0) ? (k - sample_k) : 0; // clip at zero

                                                            // Target mean for the rest of the samples
    float target_ma = ((float)n * ma - (float)n_sampled * sample_mean) / (float)n_left;

    // Target mean for the low intensity gaussian
    float target_ml = ((float)n_left * target_ma - (float)target_k * mh) / ((float)n_left - (float)target_k);

    float current_sample = 0.0f;
    if (c[i] == 1)
    {
      current_sample = high_gaussian(gen);
    }
    else if (c[i] == 2)
    {
      std::normal_distribution<float> low_gaussian(target_ml, sl);
      current_sample = low_gaussian(gen);
    }

    // Clip to [0 1]
    if (current_sample > 1.0f)
      current_sample = 1.0f;
    else if (current_sample < 0.0f)
      current_sample = 0.0f;

    // Assume all lights within one sigma of high intensity mean are high
    if (current_sample > mh - sh)
      sample_k++;

    result[i] = current_sample;
    n_sampled++;
    sample_mean = ((n_sampled - 1) * sample_mean + current_sample) / n_sampled;
  }

  std::vector<float> unconstrained_samples;
  unconstrained_samples.resize(n - n_sampled, 0.0f);

  // sample the unconstrained lights
  for (int i = 0; i < unconstrained_samples.size(); ++i)
  {
    if (c[i] != 0)
      continue;

    int n_left = n - n_sampled;

    // Target number of directed lights
    int target_k = (k - sample_k > 0) ? (k - sample_k) : 0; // clip at zero

                                                            // Target high gaussian mixture weight
    float target_wh = ((float)target_k / (float)n_left < 1.0f) ? ((float)target_k / (float)n_left) : 1.0f; // clip highest at one

                                                                                                           // Target low gaussian mixture weight (Not used, here for completeness)
                                                                                                           // float target_wl = 1.0f - target_wh;

                                                                                                           // Target mean for the rest of the samples
    float target_ma = ((float)n * ma - (float)n_sampled * sample_mean) / (float)n_left;

    // Target mean for the low intensity gaussian
    float target_ml = ((float)n_left * target_ma - (float)target_k * mh) / ((float)n_left - (float)target_k);

    // Begin: Sample from the mixture
    // Sample uniform from (0 1) and decide which gaussian to sample based
    // on mixture weights such that:
    // (0 target_wh): sample from high gaussian
    // (target_wh 1): sample from low gaussian
    float current_sample = 0.0f;
    float current_mixture = uniform_0_1(gen);

    // Constraints to get as close number of high intensity lights
    if (sample_k >= k) // Enough already :)
      current_mixture = 1.0f;
    else if (k - sample_k >= n_left) // Not enough
      current_mixture = 0.0f;

    if (current_mixture < target_wh)
      current_sample = high_gaussian(gen);
    else
    {
      std::normal_distribution<float> low_gaussian(target_ml, sl);
      current_sample = low_gaussian(gen);
    }

    // Clip to [0 1]
    if (current_sample > 1.0f)
      current_sample = 1.0f;
    else if (current_sample < 0.0f)
      current_sample = 0.0f;

    // Assume all lights within one sigma of high intensity mean are high
    if (current_sample > mh - sh)
      sample_k++;

    unconstrained_samples[i] = current_sample;
    n_sampled++;
    sample_mean = ((n_sampled - 1) * sample_mean + current_sample) / n_sampled;
  }

  std::shuffle(unconstrained_samples.begin(), unconstrained_samples.end(), gen);

  int j = 0;
  for (int i = 0; i < n; ++i)
  {
    if (c[i] == 0)
    {
      result[i] = unconstrained_samples[j];
      j++;
    }
  }

  if (use_image_intensity)
  {
    float sum_s = 0.0f;
    for (int i = 0; i < n; ++i)
    {
      float current_s = (result[i] / s[i] > 1.0f) ? (result[i] / 1.0f) : s[i];
      result[i] = result[i] / current_s;
      sum_s += current_s;
    }

    sum_s = sum_s / (float)n;

    for (int i = 0; i < n; ++i)
    {
      result[i] = result[i] * sum_s;
    }
  }
}