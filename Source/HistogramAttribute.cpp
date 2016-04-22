/*
  ==============================================================================

    HistogramAttribute.cpp
    Created: 21 Apr 2016 12:07:38pm
    Author:  falindrith

  ==============================================================================
*/

#include "HistogramAttribute.h"

Histogram1D::Histogram1D(int numBins) {
  // we assume you'll only ever have 255 bins since images here
  // (at the moment) are uint8s
  _count = 0;

  if (numBins > 255)
    _numBins = 255;
  else if (numBins < 1)
    _numBins = 1;
  else
    _numBins = numBins;
}

Histogram1D::Histogram1D(const Histogram1D & other) :
  _histData(other._histData),
  _count(other._count),
  _numBins(other._numBins)
{
}

Histogram1D::Histogram1D(map<unsigned int, unsigned int> data, unsigned int count, int numBins) :
  _histData(data), _count(count), _numBins(numBins)
{
}

Histogram1D::~Histogram1D()
{
}

void Histogram1D::addValToBin(double x)
{
  int bin = (int)(x * _numBins);
  _histData[bin] += 1;
  _count++;
}

void Histogram1D::addValToBin(uint8 x)
{
  addValToBin(x / 255.0);
}

unsigned int Histogram1D::getBin(unsigned int id)
{
  return _histData[id];
}

void Histogram1D::addToBin(unsigned int amt, unsigned int id)
{
  _histData[id] += amt;
  _count += amt;
}

void Histogram1D::removeFromBin(unsigned int amt, unsigned int id)
{
  _histData[id] -= amt;
  _count -= amt;
}

Histogram1D Histogram1D::consolidate(int targetNumBins)
{
  // start with requiring a 5% threshold.
  float t = 0.05f;

  map<unsigned int, unsigned int> newData = _histData;

  while (_histData.size() > targetNumBins) {
    for (auto& b : newData) {
      // if the bin is below the threshold
      if (b.second / (float)_count < t) {
        // find closest bin above threshold
        int start = b.first;

        for (int i = 1; i <= _numBins; i++) {
          // do a search starting at the position of the current bin
          if (start - i >= 0) {
            if (newData.count(start - i) > 0) {
              // check if bin is above threshold
              if (newData[start - i] / (float)_count > t) {
                // if so, move these samples over and break
                b.second += newData[start - i];
                newData[start - i] = 0;
                break;
              }
            }
          }

          // same thing for forward
          if (start + i <= _numBins) {
            if (newData.count(start + i) > 0) {
              if (newData[start + i] / (float)_count > t) {
                b.second += newData[start + i];
                newData[start + i] = 0;
                break;
              }
            }
          }
        }
      }
    }

    // delete bins that are 0
    for (int i = 0; i <= _numBins; i++) {
      if (newData[i] == 0)
        newData.erase(i);
    }

    // increment threshold
    t += 0.05f;
  }

  return Histogram1D(newData, _count, _numBins);
}

double Histogram1D::weightedAvg()
{
  double interval = 1.0 / _numBins;
  double sum = 0;

  for (const auto& vals : _histData) {
    double binVal = interval * vals.first;
    sum += binVal * ((double)vals.second / _count);
  }

  return sum;
}

HistogramAttribute::HistogramAttribute(string name, int w, int h) :
  _canonicalWidth(w), _canonicalHeight(h), AttributeControllerBase(name)
{
  _canonical = Image(Image::ARGB, _canonicalWidth, _canonicalHeight, true);
  _highRes = Image(Image::ARGB, _canonicalWidth * 2, _canonicalHeight * 2, true);
}

HistogramAttribute::~HistogramAttribute()
{
}

void HistogramAttribute::generateImage(Snapshot* s)
{
  auto devices = s->getDevices();
  auto p = getAnimationPatch();

  if (p == nullptr)
    return;

  // render at 2x res, downsample to canonical resolution
  uint8* bufptr = Image::BitmapData(_highRes, Image::BitmapData::readWrite).getPixelPointer(0,0);
  p->setDims(_canonicalWidth * 2, _canonicalHeight * 2);
  p->setSamples(getGlobalSettings()->_thumbnailRenderSamples);

  getAnimationPatch()->renderSingleFrameToBuffer(devices, bufptr);
  
  _canonical = _highRes.rescaled(_canonicalWidth, _canonicalHeight);
}

Histogram1D HistogramAttribute::getGrayscaleHist(int numBins)
{
  Histogram1D gs(numBins);

  for (int x = 0; x < _canonicalWidth; x++) {
    for (int y = 0; y < _canonicalHeight; y++) {
      auto color = _canonical.getPixelAt(x, y);
      gs.addValToBin(color.getBrightness());
    }
  }

  return gs;
}

Histogram1D HistogramAttribute::getPerceptualGrayscaleHist(int numBins)
{
  Histogram1D gs(numBins);

  for (int x = 0; x < _canonicalWidth; x++) {
    for (int y = 0; y < _canonicalHeight; y++) {
      auto color = _canonical.getPixelAt(x, y);
      gs.addValToBin(color.getPerceivedBrightness());
    }
  }

  return gs;
}

Histogram1D HistogramAttribute::getChannelHist(int numBins, int channel)
{
  Histogram1D gs(numBins);

  for (int x = 0; x < _canonicalWidth; x++) {
    for (int y = 0; y < _canonicalHeight; y++) {
      auto color = _canonical.getPixelAt(x, y);

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

Histogram1D HistogramAttribute::getHueHist(int numBins)
{
  Histogram1D gs(numBins);

  for (int x = 0; x < _canonicalWidth; x++) {
    for (int y = 0; y < _canonicalHeight; y++) {
      auto color = _canonical.getPixelAt(x, y);
      gs.addValToBin(color.getHue());
    }
  }

  return gs;
}

Histogram1D HistogramAttribute::getSatHist(int numBins)
{
  Histogram1D gs(numBins);

  for (int x = 0; x < _canonicalWidth; x++) {
    for (int y = 0; y < _canonicalHeight; y++) {
      auto color = _canonical.getPixelAt(x, y);
      gs.addValToBin(color.getSaturation());
    }
  }

  return gs;
}