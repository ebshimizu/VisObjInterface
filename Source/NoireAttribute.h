/*
  ==============================================================================

    NoireAttribute.h
    Created: 6 Apr 2016 2:50:15pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef NOIREATTRIBUTE_H_INCLUDED
#define NOIREATTRIBUTE_H_INCLUDED

#include "AttributeControllerBase.h"

class NoireAttribute : public AttributeControllerBase
{
public:
  NoireAttribute();
  ~NoireAttribute();

  virtual double evaluateScene(Snapshot* s) override;

  virtual void preProcess() override;

private:
  double getScoreForArea(Snapshot* s, string a);
  double getAvgIntensity(Snapshot* s, string area, string angle);

  unordered_map<string, DeviceSet> _cache;
  unordered_map<string, double> _weights;
  set<string> _areas;
};

#endif  // NOIREATTRIBUTE_H_INCLUDED
