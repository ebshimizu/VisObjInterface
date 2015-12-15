/*
  ==============================================================================

    AttributeControls.h
    Created: 15 Dec 2015 5:07:22pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef ATTRIBUTECONTROLS_H_INCLUDED
#define ATTRIBUTECONTROLS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class AttributeControls    : public Component
{
public:
    AttributeControls();
    ~AttributeControls();

    void paint (Graphics&);
    void resized();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeControls)
};


#endif  // ATTRIBUTECONTROLS_H_INCLUDED
