/*
  ==============================================================================

    SceneViewer.cpp
    Created: 15 Dec 2015 5:07:32pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SceneViewer.h"
#include <thread>
#include <chrono>

//==============================================================================
RenderBackgroundThread::RenderBackgroundThread(ArnoldAnimationPatch* p, uint8* bufptr) :
  ThreadWithProgressWindow("Rendering Current Scene", true, true), _p(p), _bufptr(bufptr)
{
  setStatusMessage("Preparing...");
  setProgress(-1.0);
}

RenderBackgroundThread::~RenderBackgroundThread()
{
}

void RenderBackgroundThread::run()
{
  // little bit of thread inception here...
  thread r(&RenderBackgroundThread::renderLoop, this);

  float prog = 0;
  setStatusMessage("Rendering...");
  this_thread::sleep_for(std::chrono::milliseconds(100));
  while (prog < 1) {
    this_thread::sleep_for(std::chrono::milliseconds(20));
    prog = _p->getPercentage() / 100.0f;

    if (threadShouldExit()) {
      _p->forceInterrupt();
      return;
    }
    
    setProgress(prog);
  }

  setProgress(-1.0);
  setStatusMessage("Finalizing frame...");
  r.join();
}

void RenderBackgroundThread::threadComplete(bool userPressedCancel)
{
  if (userPressedCancel) {
    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
      "Render Current Scene",
      "Render canceled");
  }

  delete this;
}

void RenderBackgroundThread::renderLoop() {
  _p->renderSingleFrameToBuffer(getRig()->getDeviceRaw(), _bufptr, getGlobalSettings()->_renderWidth, getGlobalSettings()->_renderHeight);
}

//==============================================================================
SceneViewer::SceneViewer()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
  _currentRender = Image();
}

SceneViewer::~SceneViewer()
{
}

void SceneViewer::paint (Graphics& g)
{
  /* This demo code just fills the component's background and
     draws some placeholder text to get you started.

     You should replace everything in this method with your own
     drawing code..
  */
  g.fillAll(Colour(0xff333333));

  if (getGlobalSettings()->_showThumbnailImg) {
    g.drawImageWithin(_preview, 0, 0, getWidth(), getHeight(), RectanglePlacement::centred);
  }
  else {
    g.drawImageWithin(_currentRender, 0, 0, getWidth(), getHeight(), RectanglePlacement::centred);
  }

  g.setColour(Colours::red);
  auto focus = getGlobalSettings()->_focusBounds;
  Point<float> topLeft = getWorldImageCoords(focus.getTopLeft());
  Point<float> bottomRight = getWorldImageCoords(focus.getBottomRight());
  g.drawRect(Rectangle<float>::leftTopRightBottom(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y), 2);
}

void SceneViewer::resized()
{
  // This method is where you should set the bounds of any child
  // components that your component contains..

}

void SceneViewer::renderScene() {
  // Renders a scene using Arnold through Lumiverse.
  // Find the patch, we search for the first ArnoldAnimationPatch we can find.
  ArnoldAnimationPatch* p = getAnimationPatch();

  if (p == nullptr) {
    getRecorder()->log(RENDER, "Render failed. Could not find ArnoldAnimationPatch.");
    return;
  }

  // Get the image dimensions
  int width = getGlobalSettings()->_renderWidth;
  int height = getGlobalSettings()->_renderHeight;
  p->setDims(width, height);
  p->setSamples(getGlobalSettings()->_stageRenderSamples);

  _currentRender = Image(Image::ARGB, width, height, true);
  uint8* bufptr = Image::BitmapData(_currentRender, Image::BitmapData::readWrite).getPixelPointer(0,0);

  getRecorder()->log(RENDER, "Render started.");
  getGlobalSettings()->_renderInProgress = true;
  (new RenderBackgroundThread(p, bufptr))->runThread();
  getGlobalSettings()->_renderInProgress = false;
  getRecorder()->log(RENDER, "Render finished.");

  repaint();
}

void SceneViewer::renderSceneNoPopup()
{
  // Renders a scene using Arnold through Lumiverse.
  // Find the patch, we search for the first ArnoldAnimationPatch we can find.
  ArnoldAnimationPatch* p = getAnimationPatch();

  if (p == nullptr) {
    getRecorder()->log(RENDER, "Render failed. Could not find ArnoldAnimationPatch.");
    return;
  }

  // Get the image dimensions
  int width = getGlobalSettings()->_renderWidth;
  int height = getGlobalSettings()->_renderHeight;
  p->setDims(width, height);
  p->setSamples(getGlobalSettings()->_stageRenderSamples);

  _currentRender = Image(Image::ARGB, width, height, true);
  uint8* bufptr = Image::BitmapData(_currentRender, Image::BitmapData::readWrite).getPixelPointer(0, 0);

  getRecorder()->log(RENDER, "Render started.");
  getGlobalSettings()->_renderInProgress = true;
  p->renderSingleFrameToBuffer(getRig()->getDeviceRaw(), bufptr, width, height);
  getGlobalSettings()->_renderInProgress = false;
  getRecorder()->log(RENDER, "Render finished.");

  repaint();
}

void SceneViewer::setRender(Image img)
{
  _currentRender = img;
  repaint();
}

void SceneViewer::setPreview(Image prev)
{
  _preview = prev;
}

void SceneViewer::mouseDown(const MouseEvent & event)
{
  if (_currentRender.getWidth() <= 0)
    return;

  _startPoint = getRelativeImageCoords(event.position);
  getGlobalSettings()->_focusBounds.setWidth(0);
  getGlobalSettings()->_focusBounds.setHeight(0);
  repaint();
}

void SceneViewer::mouseUp(const MouseEvent & event)
{
  if (_currentRender.getWidth() <= 0)
    return;

  _currentPoint = getRelativeImageCoords(event.position);
  Array<Point<float> > pts;
  pts.add(_startPoint);
  pts.add(_currentPoint);

  getGlobalSettings()->_focusBounds = Rectangle<float>::findAreaContainingPoints(pts.getRawDataPointer(), 2);
  repaint();
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_ATTR, true);
}

void SceneViewer::mouseDrag(const MouseEvent & event)
{
  if (_currentRender.getWidth() <= 0)
    return;

  _currentPoint = getRelativeImageCoords(event.position);
  Array<Point<float> > pts;
  pts.add(_startPoint);
  pts.add(_currentPoint);

  getGlobalSettings()->_focusBounds = Rectangle<float>::findAreaContainingPoints(pts.getRawDataPointer(), 2);
  repaint();
}

Point<float> SceneViewer::getRelativeImageCoords(const Point<float>& pt)
{
  auto lbounds = getLocalBounds();
  // determine the image location
  float scaleX = (float)lbounds.getWidth() / _currentRender.getWidth();
  float scaleY = (float)lbounds.getHeight() / _currentRender.getHeight();
  float scale;

  // test xscaling
  if (_currentRender.getHeight() * scaleX <= lbounds.getHeight()) {
    scale = scaleX;
  }
  else if (_currentRender.getWidth() * scaleY <= lbounds.getWidth()) {
    scale = scaleY;
  }

  float imgWidth = _currentRender.getWidth()  * scale;
  float imgHeight = _currentRender.getHeight() * scale;
  float ptHeightAdjust = abs((imgHeight - lbounds.getHeight()) / 2.0);
  float ptWidthAdjust = abs((imgWidth - lbounds.getWidth()) / 2.0);
  
  float x = clamp((pt.x - ptWidthAdjust) / imgWidth, 0, 1);
  float y = clamp((pt.y - ptHeightAdjust) / imgHeight, 0, 1);
  
  return Point<float>(x, y);
}

Point<float> SceneViewer::getWorldImageCoords(const Point<float>& pt)
{
  auto lbounds = getLocalBounds();
  // determine the image location
  float scaleX = (float)lbounds.getWidth() / _currentRender.getWidth();
  float scaleY = (float)lbounds.getHeight() / _currentRender.getHeight();
  float scale;

  // test xscaling
  if (_currentRender.getHeight() * scaleX <= lbounds.getHeight()) {
    scale = scaleX;
  }
  else if (_currentRender.getWidth() * scaleY <= lbounds.getWidth()) {
    scale = scaleY;
  }

  float imgWidth = _currentRender.getWidth()  * scale;
  float imgHeight = _currentRender.getHeight() * scale;
  float ptHeightAdjust = abs((imgHeight - lbounds.getHeight()) / 2.0);
  float ptWidthAdjust = abs((imgWidth - lbounds.getWidth()) / 2.0);

  float x = pt.x * imgWidth + ptWidthAdjust;
  float y = pt.y * imgHeight + ptHeightAdjust;

  return Point<float>(x, y);
}
