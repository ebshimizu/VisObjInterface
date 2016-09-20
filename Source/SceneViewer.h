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
class SceneViewer    : public Component, public ButtonListener
{
public:
  SceneViewer();
  ~SceneViewer();

  void paint (Graphics&);
  void resized();

  // Does a synchronous render of the current scene specified in the Rig
  void renderScene();

  // Does a render of the current scene without showing the progress window
  void renderSceneNoPopup();

  Image getRender() { return _currentRender; }
  void setRender(Image img);
  void setPreview(Image prev);

  // callbacks for drawing the box
  // The box dimensions are set in the global settings class to make sure
  // other parts of the application can read them easily
  virtual void mouseDown(const MouseEvent& event);
  virtual void mouseUp(const MouseEvent& event);
  virtual void mouseDrag(const MouseEvent& event);

  virtual void buttonClicked(Button* b) override;

  // clears out the freeze mask
  void clearMask();

  // Draws the mask as an overlay on the current image
  void showMask();

private:
  Point<float> getRelativeImageCoords(const Point<float>& pt);
  Point<float> getWorldImageCoords(const Point<float>& pt);

  Image _currentRender;
  Image _preview;

  Point<float> _startPoint;
  Point<float> _currentPoint;

  // tools contains a series of buttons to lock down specified regions of the scene.
  Array<ToggleButton*> _tools;
  TextButton _clearMask;
  TextButton _showMask;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneViewer)
};


#endif  // SCENEVIEWER_H_INCLUDED
