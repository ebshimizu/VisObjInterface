/*
  ==============================================================================

    HighAngleAttribute.h
    Created: 29 Feb 2016 11:26:28am
    Author:  falindrith

  ==============================================================================
*/

#ifndef HIGHANGLEATTRIBUTE_H_INCLUDED
#define HIGHANGLEATTRIBUTE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

//==============================================================================
/*
*/
class HighAngleAttribute : public AttributeControllerBase
{
public:
  HighAngleAttribute();
  HighAngleAttribute(String name);
  ~HighAngleAttribute();

protected:
  virtual double evaluateScene(Device* key, Device* fill, Device* rim) override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HighAngleAttribute)
};

#endif  // HIGHANGLEATTRIBUTE_H_INCLUDED
