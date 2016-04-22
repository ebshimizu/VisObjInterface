/*
  ==============================================================================

    HistogramBrightness.h
    Created: 22 Apr 2016 11:38:15am
    Author:  falindrith

  ==============================================================================
*/

#ifndef HISTOGRAMBRIGHTNESS_H_INCLUDED
#define HISTOGRAMBRIGHTNESS_H_INCLUDED

#include "HistogramAttribute.h"

// calculates brightness from a histogram. May eventually replace that
// attribute in favor of this one
class HistogramBrightness : public HistogramAttribute
{
public:
  HistogramBrightness(string name, int numBins, int w, int h);
  ~HistogramBrightness();

  virtual double evaluateScene(Snapshot* s) override;

private:
  int _numBins;
};



#endif  // HISTOGRAMBRIGHTNESS_H_INCLUDED
