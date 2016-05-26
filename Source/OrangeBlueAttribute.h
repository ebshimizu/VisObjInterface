/*
  ==============================================================================

    OrangeBlueAttribute.h
    Created: 26 Apr 2016 4:55:57pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef ORANGEBLUEATTRIBUTE_H_INCLUDED
#define ORANGEBLUEATTRIBUTE_H_INCLUDED

#include "HistogramAttribute.h"

class OrangeBlueAttribute : public HistogramAttribute
{
public:
  OrangeBlueAttribute(int w, int h);
  ~OrangeBlueAttribute();

  virtual double evaluateScene(Snapshot* s) override;

private:
  // Returns if x or y is closer to the specified Hue Range
  // hue wraps, so 359 is closer to 0 than it is to 350
  // in the event that both are within the range, returns the one closest to the range center
  unsigned int closestToRange(int x, int y, int min, int max);

  int _targetBlue;
  int _targetOrange;
};



#endif  // ORANGEBLUEATTRIBUTE_H_INCLUDED
