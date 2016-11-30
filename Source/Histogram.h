/*
  ==============================================================================

    Histogram.h
    Created: 29 Nov 2016 8:18:28pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef HISTOGRAM_H_INCLUDED
#define HISTOGRAM_H_INCLUDED

#include "globals.h"
#include "FastEMD-3.1/FastEMD/emd_hat_signatures_interface.hpp"
#include "FastSimplex/network_simplex_simple.h"

using namespace lemon;

typedef FullBipartiteDigraph Digraph;
DIGRAPH_TYPEDEFS(FullBipartiteDigraph);

// a note about all these histogram classes:
// when it says numBins the actual number of bins in the histogram will
// be numBins + 1. The max valued element (1) will be in a bin all by itself.
// The histogram also assumes everything is normalized between 0 and 1
class Histogram1D {
public:
  Histogram1D(int numBins);
  Histogram1D(const Histogram1D& other);
  Histogram1D(map<unsigned int, unsigned int> data, unsigned int count, int numBins);
  ~Histogram1D();

  // doubles are assumed to be normalized between 0 and 1
  void addValToBin(double x);
  void addValToBin(uint8 x);

  unsigned int getBin(unsigned int id);
  unsigned int getNthBin(unsigned int n);

  void addToBin(unsigned int amt, unsigned int id);
  void removeFromBin(unsigned int amt, unsigned int id);

  // Returns the distance between this histogram and another histogram
  double diff(Histogram1D& other);

  // Finds the n most dominant bins in the histogram
  // Idea is that it takes bins under a threshold and merges them into the closest
  // bin above the threshold. Process repeats until target number of bins is
  // reached.
  // Returns a copy of the new histogram
  Histogram1D consolidate(int targetNumBins);

  // Returns an average of values contained in the histogram bins
  double avg();

  // variance and st deviation
  double variance();
  double stdev();

  // Returns the value of the n-th percentile
  double percentile(float pct);

  // Returns the percent of samples greater than n% of the max
  double percentGreaterThan(float pct);

  // Returns the percent of samples less than n% of the max
  double percentLessThan(float pct);

private:
  // Maps bins to the number of pixels in each bin
  map<unsigned int, unsigned int> _histData;

  // total number of elements in the bin
  unsigned int _count;

  // number of bins in the histogram
  int _numBins;
};

typedef vector<int> hist3DData;

// A histogram for color attributes
// typically treats bins as values from 0-1, however a custom range may be specified
// Unlike Histogram1D (which is a bit weird) this histogram class's bins are [low, high) with
// values out of range clamped to closest bin
class Histogram3D {
public:
  Histogram3D(int n);
  Histogram3D(int x, int y, int z);
  Histogram3D(int x, int y, int z, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);
  Histogram3D(const Histogram3D& other);
  Histogram3D& operator=(const Histogram3D& other);
  ~Histogram3D();

  void addValToBin(double x, double y, double z);
  void addToBin(int amt, int x, int y, int z);
  void removeFromBin(int amt, int x, int y, int z);

  unsigned int getBin(int x, int y, int z);

  string toString();

  // Returns the distance between this histogram and a different histogram
  // Histograms must have the same number of bins with the same dimensions
  double L2dist(Histogram3D& other);

  // Earth Mover's distance for a different histogram
  double emd(Histogram3D& other, vector<vector<double> >& gd);

  // Returns the normalized form of the histogram (divides each bin by count)
  vector<double> normalized();

  // Returns the matrix of ground distances for the histogram
  vector<vector<double> > getGroundDistances();

private:
  inline int getIndex(int x, int y, int z);

  // returns the value of the bin
  Eigen::Vector3d getBinVal(int x, int y, int z);

  hist3DData _histData;
  emd_hat_gd_metric<double> _emd;

  unsigned int _count;
  int _x;
  int _y;
  int _z;

  double _xmin, _xmax, _ymin, _ymax, _zmin, _zmax;
};

class HistogramFeature
{
public:
  HistogramFeature();
  HistogramFeature(vector<float> pt);
  bool operator==(const HistogramFeature& other) const;

  // a feature is > than another feature if all of the components of the vectors are larger
  bool operator>(const HistogramFeature& other) const;

  feature_tt _data;
};

namespace std {
  template<>
  struct hash<HistogramFeature>
  {
    std::size_t operator()(const HistogramFeature& f) const
    {
      // potentially lots of collisions idk
      size_t hash = 0;

      for (int i = 0; i < f._data._v.size(); i++) {
        hash |= (size_t)f._data._v[i] << (32 / f._data._v.size());
      }

      return hash;
    }
  };
}

// A histogram for color + position attributes
// Basically a histogram 3D but with 2 more bins for position
// note we don't call this a histogram 5D due to special parameters controlling
// the weight of the x y coordinates in the ground distance
class LabxyHistogram {
public:
  LabxyHistogram(int n, double lambda = 1);
  LabxyHistogram(int l, int a, int b, int x, int y, double lambda = 1);
  LabxyHistogram(int l, int a, int b, int x, int y, vector<double> bounds, double lambda = 1);
  LabxyHistogram(const LabxyHistogram& other);
  LabxyHistogram& operator=(const LabxyHistogram& other);
  ~LabxyHistogram();

  void add(double l, double a, double b, float x, float y);
  void addToBin(double amt, int l, int a, int b, int x, int y);
  void removeFromBin(double amt, int l, int a, int b, int x, int y);

  double getBin(int l, int a, int b, int x, int y);

  // Returns the EMD between this histogram and the given other histogram.
  double EMD(LabxyHistogram& other, const vector<vector<double> >& gd);

  // Returns the normalized form of the histogram
  vector<double> normalized();

  // Returns the matrix of ground distances for the histogram
  vector<vector<double> > getGroundDistances();

  // Sets the weight for the pixel location
  void setLambda(double lambda);

private:
  inline int getIndex(int l, int a, int b, int x, int y);
  inline int numTotalBins();

  // adds a mass of 1 to the histogram propotionally
  // does this by recursively dividing up the mass into the different axes
  void addProportional(float weight, vector<double> coord, vector<int> bin);

  // returns the nearest bin for the given value on the specified axis
  int nearest(double val, int axis);

  // returns value of the bin
  Eigen::VectorXd getBinVal(int l, int a, int b, int x, int y);

  vector<double> _histData;
  emd_hat_gd_metric<double> _emd;

  double _count;
  vector<int> _bins;

  // xy position weight in l2 norm
  double _lambda;

  // format: lmin, lmax, amin, amax, bmin, bmax, xmin, xmax, ymin, ymax
  vector<double> _bounds;
};


#endif  // HISTOGRAM_H_INCLUDED
