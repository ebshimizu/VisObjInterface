/*
  ==============================================================================

    SoftAttribute.h
    Created: 3 Feb 2016 4:06:19pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef SOFTATTRIBUTE_H_INCLUDED
#define SOFTATTRIBUTE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

//==============================================================================
/*
*/
class SoftAttribute : public AttributeControllerBase
{
public:
  SoftAttribute();
  SoftAttribute(String name);
  ~SoftAttribute();

protected:
  virtual double evaluateScene(Device* key, Device* fill, Device* rim) override;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoftAttribute)
};




#endif  // SOFTATTRIBUTE_H_INCLUDED
