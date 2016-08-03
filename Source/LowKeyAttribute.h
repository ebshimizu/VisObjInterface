/*
  ==============================================================================

    LowKeyAttribute.h
    Created: 30 Mar 2016 5:15:56pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef LOWKEYATTRIBUTE_H_INCLUDED
#define LOWKEYATTRIBUTE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

/*
Following the typical definition of low-key lighting, this attribute finds the
brightest light in the relevant area and computes the ratio of key (brightest)
to fill (rest of light) intensity. This is done purely on base
light intensity. Direcionality may also prove to be important at some point.
*/
class LowKeyAttribute : public AttributeControllerBase
{
public:
  LowKeyAttribute(string area);
  ~LowKeyAttribute();

  virtual double evaluateScene(Snapshot* s, Image& img) override;
  virtual void preProcess() override;

private:
  vector<string> _devices;
  string _area;
};




#endif  // LOWKEYATTRIBUTE_H_INCLUDED
