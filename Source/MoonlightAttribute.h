/*
  ==============================================================================

    MoonlightAttribute.h
    Created: 26 Apr 2016 4:55:46pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef MOONLIGHTATTRIBUTE_H_INCLUDED
#define MOONLIGHTATTRIBUTE_H_INCLUDED

#include "HistogramAttribute.h"

class MoonlightAttribute : public HistogramAttribute
{
public:
  MoonlightAttribute(int w, int h);
  ~MoonlightAttribute();

  double evaluateScene(Snapshot* s) override;

  void preProcess() override;

private:
  float getMaxSystemBrightness(string sys, Snapshot* s);
  float getAvgSystemBrightness(string sys, Snapshot* s);
  Eigen::Vector3d getAvgColor(string sys, Snapshot* s);

  map<string, DeviceSet> _systemCache;
};



#endif  // MOONLIGHTATTRIBUTE_H_INCLUDED
