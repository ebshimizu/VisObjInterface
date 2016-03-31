/*
  ==============================================================================

    SaturationAttribute.h
    Created: 31 Mar 2016 10:48:48am
    Author:  falindrith

  ==============================================================================
*/

#ifndef SATURATIONATTRIBUTE_H_INCLUDED
#define SATURATIONATTRIBUTE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

class SaturationAttribute : public AttributeControllerBase
{
public:
  SaturationAttribute();
  ~SaturationAttribute();

  virtual double evaluateScene(Snapshot* s) override;

  virtual void preProcess() override;

private:
  unordered_map<string, double> _weights;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SaturationAttribute);
};




#endif  // SATURATIONATTRIBUTE_H_INCLUDED
