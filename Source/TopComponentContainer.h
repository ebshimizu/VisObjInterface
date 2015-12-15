/*
  ==============================================================================

    TopComponentContainer.h
    Created: 15 Dec 2015 5:22:50pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef TOPCOMPONENTCONTAINER_H_INCLUDED
#define TOPCOMPONENTCONTAINER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class TopComponentContainer    : public Component
{
public:
    TopComponentContainer()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.

    }

    ~TopComponentContainer()
    {
    }

    void paint (Graphics& g)
    {
        /* This demo code just fills the component's background and
           draws some placeholder text to get you started.

           You should replace everything in this method with your own
           drawing code..
        */

        g.fillAll (Colours::white);   // clear the background

        g.setColour (Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

        g.setColour (Colours::lightblue);
        g.setFont (14.0f);
        g.drawText ("TopComponentContainer", getLocalBounds(),
                    Justification::centred, true);   // draw some placeholder text
    }

    void resized()
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopComponentContainer)
};


#endif  // TOPCOMPONENTCONTAINER_H_INCLUDED
