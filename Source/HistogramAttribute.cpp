/*
  ==============================================================================

    HistogramAttribute.cpp
    Created: 21 Apr 2016 12:07:38pm
    Author:  falindrith

  ==============================================================================
*/

#include "HistogramAttribute.h"

HistogramAttribute::HistogramAttribute(string name) : AttributeControllerBase(name)
{
}

HistogramAttribute::HistogramAttribute(string name, int w, int h) :
  AttributeControllerBase(name, w, h)
{
}

HistogramAttribute::~HistogramAttribute()
{
}

Histogram1D HistogramAttribute::getGrayscaleHist(Image& canonical, int numBins)
{
  Histogram1D gs(numBins);

  for (int x = 0; x < _canonicalWidth; x++) {
    for (int y = 0; y < _canonicalHeight; y++) {
      auto color = canonical.getPixelAt(x, y);
      gs.addValToBin(color.getBrightness());
    }
  }

  return gs;
}

Histogram1D HistogramAttribute::getPerceptualGrayscaleHist(Image& canonical, int numBins)
{
  Histogram1D gs(numBins);

  for (int x = 0; x < _canonicalWidth; x++) {
    for (int y = 0; y < _canonicalHeight; y++) {
      auto color = canonical.getPixelAt(x, y);
      gs.addValToBin(color.getPerceivedBrightness());
    }
  }

  return gs;
}

Histogram1D HistogramAttribute::getChannelHist(Image& canonical, int numBins, int channel)
{
  Histogram1D gs(numBins);

  for (int x = 0; x < _canonicalWidth; x++) {
    for (int y = 0; y < _canonicalHeight; y++) {
      auto color = canonical.getPixelAt(x, y);

      if (channel == 0) {
        gs.addValToBin(color.getRed());
      }
      if (channel == 1) {
        gs.addValToBin(color.getGreen());
      }
      if (channel == 2) {
        gs.addValToBin(color.getBlue());
      }
    }
  }

  return gs;
}

Histogram1D HistogramAttribute::getHueHist(Image& canonical, int numBins)
{
  Histogram1D gs(numBins);

  for (int x = 0; x < _canonicalWidth; x++) {
    for (int y = 0; y < _canonicalHeight; y++) {
      auto color = canonical.getPixelAt(x, y);
      gs.addValToBin(color.getHue());
    }
  }

  return gs;
}

Histogram1D HistogramAttribute::getSatHist(Image& canonical, int numBins)
{
  Histogram1D gs(numBins);

  for (int x = 0; x < _canonicalWidth; x++) {
    for (int y = 0; y < _canonicalHeight; y++) {
      auto color = canonical.getPixelAt(x, y);
      gs.addValToBin(color.getSaturation());
    }
  }

  return gs;
}

Eigen::Vector3d HistogramAttribute::getAverageColor(Image i)
{
  Eigen::Vector3d color(0,0,0);

  for (int y = 0; y < i.getHeight(); y++) {
    for (int x = 0; x < i.getWidth(); x++) {
      auto c = i.getPixelAt(x, y);
      color[0] += c.getRed() / 255.0;
      color[1] += c.getGreen() / 255.0;
      color[2] += c.getBlue() / 255.0;
    }
  }

  return color / (i.getWidth() * i.getHeight());
}

double HistogramAttribute::getAverageHue(Image i)
{
  double hue = 0;

  for (int y = 0; y < i.getHeight(); y++) {
    for (int x = 0; x < i.getWidth(); x++) {
      auto c = i.getPixelAt(x, y);
      hue += c.getHue();
    }
  }

  return hue / (i.getHeight() * i.getWidth());
}

SparseHistogram HistogramAttribute::getLabxyHist(Image & canonical, float lambda)
{
  SparseHistogram hist(5, { 0, 20, -12.5, 25, -12.5, 25, 0, 0.25f, 0, 0.25f }, lambda);

  for (int y2 = 0; y2 < canonical.getHeight(); y2++) {
    for (int x2 = 0; x2 < canonical.getWidth(); x2++) {
      auto color = canonical.getPixelAt(x2, y2);
      Eigen::Vector3d labColor = RGBtoLab(color.getRed() / 255.0, color.getGreen() / 255.0, color.getBlue() / 255.0);
      float xnrm = x2 / (float)canonical.getWidth();
      float ynrm = y2 / (float)canonical.getHeight();

      hist.add({ (float)labColor[0], (float)labColor[1], (float)labColor[2], xnrm, ynrm });
    }
  }

  return hist;
}

SparseHistogram HistogramAttribute::getLHist(Image & canonical)
{
  SparseHistogram hist(1, { 0, 5 });

  for (int y2 = 0; y2 < canonical.getHeight(); y2++) {
    for (int x2 = 0; x2 < canonical.getWidth(); x2++) {
      auto color = canonical.getPixelAt(x2, y2);
      Eigen::Vector3d labColor = RGBtoLab(color.getRed() / 255.0, color.getGreen() / 255.0, color.getBlue() / 255.0);

      hist.add({ (float)labColor[0] });
    }
  }

  return hist;
}

SparseHistogram HistogramAttribute::getabHist(Image & canonical)
{
  SparseHistogram hist(2, { -5, 10, -5, 10 });

  for (int y2 = 0; y2 < canonical.getHeight(); y2++) {
    for (int x2 = 0; x2 < canonical.getWidth(); x2++) {
      auto color = canonical.getPixelAt(x2, y2);
      Eigen::Vector3d labColor = RGBtoLab(color.getRed() / 255.0, color.getGreen() / 255.0, color.getBlue() / 255.0);

      hist.add({ (float)labColor[1], (float)labColor[2] });
    }
  }

  return hist;
}

SparseHistogram HistogramAttribute::getLabHist(Image & canonical)
{
  SparseHistogram hist(3, { 0, 10, -5, 10, -5, 10 });

  for (int y2 = 0; y2 < canonical.getHeight(); y2++) {
    for (int x2 = 0; x2 < canonical.getWidth(); x2++) {
      auto color = canonical.getPixelAt(x2, y2);
      Eigen::Vector3d labColor = RGBtoLab(color.getRed() / 255.0, color.getGreen() / 255.0, color.getBlue() / 255.0);

      hist.add({ (float)labColor[0], (float)labColor[1], (float)labColor[2] });
    }
  }

  return hist;
}

LabxyHistogram HistogramAttribute::getLabxyHist2(Image & canonical, int n)
{
  return getLabxyHist2(canonical, n, n, n, n, n);
}

LabxyHistogram HistogramAttribute::getLabxyHist2(Image & canonical, int l, int a, int b, int x, int y)
{
  LabxyHistogram hist(l, a, b, x, y, { 0, 100, -70, 70, -70, 70, 0, 1, 0, 1 }, 1);
  double tmp = DBL_MIN;
  for (int y2 = 0; y2 < canonical.getHeight(); y2++) {
    for (int x2 = 0; x2 < canonical.getWidth(); x2++) {
      auto color = canonical.getPixelAt(x2, y2);
      Eigen::Vector3d labColor = RGBtoLab(color.getRed() / 255.0, color.getGreen() / 255.0, color.getBlue() / 255.0);
      float xnrm = x2 / (float)canonical.getWidth();
      float ynrm = y2 / (float)canonical.getHeight();

      hist.add(labColor[0], labColor[1], labColor[2], xnrm, ynrm);

      if (abs(labColor[1]) > tmp)
        tmp = abs(labColor[1]);
    }
  }

  return hist;
}

Eigen::Vector3d HistogramAttribute::RGBtoLab(double r, double g, double b)
{
  Eigen::Vector3d xyz = ColorUtils::convRGBtoXYZ(r, g, b, sRGB);
  return ColorUtils::convXYZtoLab(xyz, refWhites[D65] / 100.0);
}