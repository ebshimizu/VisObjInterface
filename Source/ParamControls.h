/*
  ==============================================================================

    ParamControls.h
    Created: 15 Dec 2015 5:07:12pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef PARAMCONTROLS_H_INCLUDED
#define PARAMCONTROLS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class ParamControls    : public Component
{
public:
    ParamControls();
    ~ParamControls();

    void paint (Graphics&);
    void resized();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamControls)
};


#endif  // PARAMCONTROLS_H_INCLUDED
