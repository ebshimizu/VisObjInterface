/*
  ==============================================================================

    HistogramContrast.h
    Created: 25 Apr 2016 3:11:45pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef HISTOGRAMCONTRAST_H_INCLUDED
#define HISTOGRAMCONTRAST_H_INCLUDED

#include "HistogramAttribute.h"

class HistogramContrast : public HistogramAttribute
{
public:
  HistogramContrast(string name, int numBins);
  HistogramContrast(string name, int numBins, int w, int h);
  ~HistogramContrast();

  virtual double evaluateScene(Snapshot* s, Image& img) override;

private:
  int _numBins;
};

#endif  // HISTOGRAMCONTRAST_H_INCLUDED
