/*
  ==============================================================================

    HistogramAttribute.h
    Created: 21 Apr 2016 12:07:38pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef HISTOGRAMATTRIBUTE_H_INCLUDED
#define HISTOGRAMATTRIBUTE_H_INCLUDED

#include "AttributeControllerBase.h"

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

typedef unsigned int* hist3DData;

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
  ~Histogram3D();

  void addValToBin(double x, double y, double z);
  void addToBin(int amt, int x, int y, int z);
  void removeFromBin(int amt, int x, int y, int z);

  unsigned int getBin(int x, int y, int z);

private:
  inline int getIndex(int x, int y, int z);

  hist3DData _histData;

  unsigned int _count;
  int _x;
  int _y;
  int _z;

  double _xmin, _xmax, _ymin, _ymax, _zmin, _zmax;
};

// This is the base class for attribute which use histograms created in the pixel
// space of the image. The base class automatically creates the histogram of
// the specified type for base classes and handles other common histogram functions
class HistogramAttribute : public AttributeControllerBase
{
public:
  HistogramAttribute(string name, int w, int h);
  ~HistogramAttribute();

  // popualtes the image field of the histogram attribute
  Image generateImage(Snapshot* s);

  // Generates a histogram based on the brightness of the image
  Histogram1D getGrayscaleHist(Image& canonical, int numBins);
  Histogram1D getPerceptualGrayscaleHist(Image& canonical, int numBins);

  // Generates a histogram based on one color channel of the image
  Histogram1D getChannelHist(Image& canonical, int numBins, int channel);

  // Generate histogram based on HSV (but not all 3)
  Histogram1D getHueHist(Image& canonical, int numBins);
  Histogram1D getSatHist(Image& canonical, int numBins);
  // val is basically brightness here

  // Returns the average color of the specified image (in RGB)
  Eigen::Vector3d getAverageColor(Image i);
  double getAverageHue(Image i);

  // Returns a n x n x n histogram of Lab pixel values
  // Lab histogram has L on x, a on y, and b on z
  Histogram3D getLabHist(int n);
  
  // Returns a x x y x z histogram of Lab pixel values
  // Lab histogram has L on x, a on y, and b on z
  Histogram3D getLabHist(int x, int y, int z);

  // Histogram3D getRGBHist();
  // Histogram3D getHSVHist();

protected:
  // size of the canonical image for the given attributre
  int _canonicalHeight;
  int _canonicalWidth;
};



#endif  // HISTOGRAMATTRIBUTE_H_INCLUDED
