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

  virtual double evaluateScene(set<Device*> devices) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestAttribute)
};


#endif  // TESTATTRIBUTE_H_INCLUDED
