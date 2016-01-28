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

protected:
  virtual double evaluateScene(Device* key, Device* fill, Device* rim) override;

private:
  double getLightIntens(Device* d);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BrightAttribute)
};


#endif  // BRIGHTATTRIBUTE_H_INCLUDED
