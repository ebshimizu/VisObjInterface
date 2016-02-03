/*
  ==============================================================================

    BacklitAttribute.h
    Created: 3 Feb 2016 12:02:57pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef BACKLITATTRIBUTE_H_INCLUDED
#define BACKLITATTRIBUTE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

//==============================================================================
/*
*/
class BacklitAttribute : public AttributeControllerBase
{
public:
  BacklitAttribute();
  BacklitAttribute(String name);
  ~BacklitAttribute();

protected:
  virtual double evaluateScene(Device* key, Device* fill, Device* rim) override;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BacklitAttribute)
};




#endif  // BACKLITATTRIBUTE_H_INCLUDED
