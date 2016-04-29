/*
  ==============================================================================

    NoireAttribute.h
    Created: 6 Apr 2016 2:50:15pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef NOIREATTRIBUTE_H_INCLUDED
#define NOIREATTRIBUTE_H_INCLUDED

#include "HistogramAttribute.h"

class NoireAttribute : public HistogramAttribute
{
public:
  NoireAttribute();
  ~NoireAttribute();

  virtual double evaluateScene(Snapshot* s) override;

  virtual void preProcess() override;

private:
  double getAreaIntens(Snapshot* s, string area, string angle);
  double getAreaCrossPenalty(Snapshot* s, string area);

  unordered_map<string, DeviceSet> _cache;
  set<string> _areas;
};

#endif  // NOIREATTRIBUTE_H_INCLUDED
