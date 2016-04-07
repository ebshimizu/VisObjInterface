/*
  ==============================================================================

    BrightAttribute.h
    Created: 20 Jan 2016 6:30:19pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef BRIGHTATTRIBUTE_H_INCLUDED
#define BRIGHTATTRIBUTE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

//==============================================================================
/*
*/
class BrightAttribute : public AttributeControllerBase
{
public:
  BrightAttribute();
  BrightAttribute(String name);
  ~BrightAttribute();

  virtual double evaluateScene(Snapshot* s) override;

  // Renders an image for each individual light on load
  // and proceeds to assign weights based on brightness of resulting images.
  virtual void preProcess() override;

  virtual map<string, vector<EditConstraint> > getExploreEdits() override;

private:
  // map of device id to numeric weight assosicated with the device
  unordered_map<string, double> _weights;

  double getLightIntens(Device* d);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BrightAttribute)
};


#endif  // BRIGHTATTRIBUTE_H_INCLUDED
