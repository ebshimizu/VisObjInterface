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
#include "Histogram.h"
#include "SparseHistogram.h"

// This is the base class for attribute which use histograms created in the pixel
// space of the image. The base class automatically creates the histogram of
// the specified type for base classes and handles other common histogram functions
class HistogramAttribute : public AttributeControllerBase
{
public:
  HistogramAttribute(string name);
  HistogramAttribute(string name, int w, int h);
  ~HistogramAttribute();

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

  // Returns the color and position histogram for the given image.
  SparseHistogram getLabxyHist(Image& canonical, float lambda = 50);

  SparseHistogram getLHist(Image& canonical);
  SparseHistogram getabHist(Image& canonical);
  SparseHistogram getLabHist(Image& canonical);

  LabxyHistogram getLabxyHist2(Image& canonical, int n);
  LabxyHistogram getLabxyHist2(Image& canonical, int l, int a, int b, int x, int y);
  // Histogram3D getRGBHist();
  // Histogram3D getHSVHist();

protected:
  Eigen::Vector3d RGBtoLab(double r, double g, double b);
};



#endif  // HISTOGRAMATTRIBUTE_H_INCLUDED
