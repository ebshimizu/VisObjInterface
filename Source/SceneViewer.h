/*
  ==============================================================================

    SceneViewer.h
    Created: 15 Dec 2015 5:07:32pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef SCENEVIEWER_H_INCLUDED
#define SCENEVIEWER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class SceneViewer    : public Component
{
public:
    SceneViewer();
    ~SceneViewer();

    void paint (Graphics&);
    void resized();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneViewer)
};


#endif  // SCENEVIEWER_H_INCLUDED
