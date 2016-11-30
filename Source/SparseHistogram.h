/*
  ==============================================================================

    SparseHistogram.h
    Created: 29 Nov 2016 8:45:57pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef SPARSEHISTOGRAM_H_INCLUDED
#define SPARSEHISTOGRAM_H_INCLUDED

#include "globals.h"

// a histogram that doesn't explicitly store every bin.
// most high dimensional histograms are sparse, and unlike the other classes,
// you do not need to specify bounds, just bin size and starting offset.
// histogram bins are centered around 0. So a bin size of 10 would result in a
// bin with range [-5, 5] and so on.
// Note that if lambda < 0 then we don't use it in the distance metric
// otherwise we then assume the last two dimensions are weighted by this lambda (weighted labxy histogram)
class SparseHistogram {
public:
  SparseHistogram(int dims, vector<float> bounds, float lambda = -1);
  ~SparseHistogram();

  void add(vector<float> pt);
  void addToBin(float amt, vector<float> pt);
  void removeFromBind(float amt, vector<float> pt);

  float getBin(vector<float> pt);
  float getBin(HistogramFeature pt);

  // Returns the weight of the largest bin
  HistogramFeature getLargestBin();

  // Returns the EMD between this histogram and the given other histogram.
  float EMD(SparseHistogram& other);

  // Retrieves a vector of the histogram weights and the corresponding features
  vector<int> weights(vector<feature_tt>& out);
  vector<int> negWeights(vector<feature_tt>& out);

  // Returns the normalized weights of the histogram and the corresponding features
  vector<float> normalizedWeights(vector<feature_tt>& out);
  vector<float> negNormalizedWeights(vector<feature_tt>& out);

  // Sets the weight for the pixel location
  void setLambda(double lambda);

  // Returns the dimensionality of this histogram
  int getDims();

  // returns the weights of bins along a single dimension
  map<double, double> getDimension(int dim);

  // Returns the total weight of the elements in the histogram
  float getTotalWeight();

private:
  // Retrieves the closest bin center to the specified point
  HistogramFeature closestBin(vector<float> pt);

  float closest(float val, int boundsIndex);

  unordered_map<HistogramFeature, float> _histData;
  float _totalWeight;

  // xy position weight in ground distance
  float _lambda;

  // format: Lbase, Lsize...
  vector<float> _bounds;

  // dimensionality of the histogram
  int _n;
};



#endif  // SPARSEHISTOGRAM_H_INCLUDED
