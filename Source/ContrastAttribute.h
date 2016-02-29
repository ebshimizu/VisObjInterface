/*
  ==============================================================================

    ContrastAttribute.h
    Created: 29 Feb 2016 11:25:00am
    Author:  falindrith

  ==============================================================================
*/

#ifndef CONTRASTATTRIBUTE_H_INCLUDED
#define CONTRASTATTRIBUTE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

//==============================================================================
/*
*/
class ContrastAttribute : public AttributeControllerBase
{
public:
  ContrastAttribute();
  ContrastAttribute(String name);
  ~ContrastAttribute();

protected:
  virtual double evaluateScene(Device* key, Device* fill, Device* rim) override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ContrastAttribute)

};

#endif  // CONTRASTATTRIBUTE_H_INCLUDED
