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

  void setBrushSize(float size);

  // paint in to the image. point should be in image coordinates (not relative)
  void paint(Point<float> pt, Colour c);

  // paint in the selected region of the image. coordinates of rect should be image space
  void paintRect(Rectangle<float> pt, Colour c);

  void showSelection();
  void showSelection(DeviceSet selected);
  void hideSelection();

  // returns the display status of the viewer
  bool isInSelectionMode();

  // clears the selection rectangle
  void clearSelection();
  
private:
  class ParamShifter : public Component, public SliderListener {
  public:
    ParamShifter(DeviceSet affected);
    ~ParamShifter();

    void resized() override;
    void paint(Graphics& g) override;

    void sliderDragEnded(Slider* s) override;
    void sliderValueChanged(Slider* s) override;

  private:
    DeviceSet _affected;
    Snapshot* _state;

    Slider _intens;
    Slider _hue;
    Slider _sat;
  };

  // returns image coordinates normalized between 0 and 1
  Point<float> getRelativeImageCoords(const Point<float>& pt);

  // returns image coordinates in screen-space
  Point<float> getWorldImageCoords(const Point<float>& pt);

  // Returns image coordinates in the image coordinate space from a screen-space mouse click
  Point<float> getAbsImageCoords(const Point<float>& pt);

  // region used just for selection purposes
  Rectangle<float> _selectedRegion;

  Image _currentRender;
  Image _preview;

  Point<float> _startPoint;
  Point<float> _currentPoint;

  // tools contains a series of buttons to lock down specified regions of the scene.
  Array<ToggleButton*> _tools;
  TextButton* _clearMask;
  TextButton* _showMask;
  TextButton* _showAllBoxesButton;
  TextButton* _hideAllBoxesButton;

  bool _drawMask;
  int _toolbarHeight = 25;
  float _brushSize;
  bool _hideAllBoxes;
  
  // vars for managing selection state
  bool _showSelectionMode;
  Image _selectionRender;
  TextButton* _exitSelectView;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneViewer)
};


#endif  // SCENEVIEWER_H_INCLUDED
