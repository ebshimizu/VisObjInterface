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
#include "globals.h"

//==============================================================================
// Render thread class

class RenderBackgroundThread : public ThreadWithProgressWindow
{
public:
  RenderBackgroundThread(ArnoldAnimationPatch* p, uint8* bufptr);
  ~RenderBackgroundThread();

  void run() override;
  void threadComplete(bool userPressedCancel) override;

private:
  void renderLoop();

  ArnoldAnimationPatch* _p;
  uint8* _bufptr;
};

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

  // Does a synchronous render of the current scene specified in the Rig
  void renderScene();

  Image getRender() { return _currentRender; }
  void setRender(Image img);

private:
  Image _currentRender;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneViewer)
};


#endif  // SCENEVIEWER_H_INCLUDED
