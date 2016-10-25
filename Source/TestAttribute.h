/*
  ==============================================================================

    TestAttribute.h
    Created: 4 Jan 2016 3:10:38pm
    Author:  Evan

  ==============================================================================
*/

#ifndef TESTATTRIBUTE_H_INCLUDED
#define TESTATTRIBUTE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

//==============================================================================
/*
*/
class TestAttribute : public AttributeControllerBase
{
public:
  TestAttribute();
  TestAttribute(String name);
  ~TestAttribute();

protected: 
  virtual double evaluateScene(Snapshot* s, Image& img) override;

private:
  double cctPenalty(double cct);
  double duvPenalty(double duv);

  // For the objective warm function, a 'warmest CCT'.
  double _targetCCT = 1900;
  double _keyWeight = 0.4;
  double _fillWeight = 0.4;
  double _rimWeight = 0.2;
  double _warmMax = 100;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestAttribute)
};

#endif  // TESTATTRIBUTE_H_INCLUDED
