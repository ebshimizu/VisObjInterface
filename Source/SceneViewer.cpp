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

  // initialize toolbar
  _tools.add(new ToggleButton("None"));
  _tools.add(new ToggleButton("Paint"));
  _tools.add(new ToggleButton("Select"));
  _tools.add(new ToggleButton("Erase"));
  _tools.add(new ToggleButton("Erase Region"));

  for (auto b : _tools) {
    b->addListener(this);
    addAndMakeVisible(b);
    b->setRadioGroupId(999);
  }

  _clearMask = new TextButton("Clear Mask");
  _showMask = new TextButton("Show Mask");
  _clearMask->addListener(this);
  _showMask->addListener(this);
  addAndMakeVisible(_clearMask);
  addAndMakeVisible(_showMask);

  _showMask->setColour(TextButton::ColourIds::buttonOnColourId, Colours::lightblue);
  _drawMask = false;
  _brushSize = 50;
}

SceneViewer::~SceneViewer()
{
  for (auto b : _tools) {
    delete b;
  }

  delete _clearMask;
  delete _showMask;
}

void SceneViewer::paint (Graphics& g)
{
  /* This demo code just fills the component's background and
     draws some placeholder text to get you started.

     You should replace everything in this method with your own
     drawing code..
  */
  g.fillAll(Colour(0xff333333));

  auto lbounds = getLocalBounds();
  auto toolbar = lbounds.removeFromTop(_toolbarHeight);

  g.setColour(Colour(0xffa2a2a2));
  g.fillRect(toolbar);

  if (getGlobalSettings()->_showThumbnailImg) {
    g.drawImageWithin(_preview, 0, _toolbarHeight, lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);
  }
  else {
    g.drawImageWithin(_currentRender, 0, _toolbarHeight, lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);
  }

  g.setColour(Colours::red);
  auto focus = getGlobalSettings()->_focusBounds;
  if (focus.getWidth() != 0 || focus.getHeight() != 0) {
    Point<float> topLeft = getWorldImageCoords(focus.getTopLeft());
    Point<float> bottomRight = getWorldImageCoords(focus.getBottomRight());
    g.drawRect(Rectangle<float>::leftTopRightBottom(topLeft.x, topLeft.y + _toolbarHeight, bottomRight.x, bottomRight.y + _toolbarHeight), 2);
  }

  if (_drawMask) {
    g.setOpacity(0.5);
    g.drawImageWithin(getGlobalSettings()->_freeze, 0, _toolbarHeight, lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);
  }
}

void SceneViewer::resized()
{
  // This method is where you should set the bounds of any child
  // components that your component contains..

  auto lbounds = getLocalBounds();
  auto toolbar = lbounds.removeFromTop(_toolbarHeight);

  for (auto& b : _tools) {
    b->setBounds(toolbar.removeFromLeft(80).reduced(2));
  }

  _clearMask->setBounds(toolbar.removeFromRight(60).reduced(2));
  _showMask->setBounds(toolbar.removeFromRight(60).reduced(2));
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

  if (width == 0 || height == 0)
    return;

  _currentRender = Image(Image::ARGB, width, height, true);
  uint8* bufptr = Image::BitmapData(_currentRender, Image::BitmapData::readWrite).getPixelPointer(0,0);

  getRecorder()->log(RENDER, "Render started.");
  (new RenderBackgroundThread(p, bufptr))->runThread();
  getRecorder()->log(RENDER, "Render finished.");

  getGlobalSettings()->updateCache();
  //getGlobalSettings()->setCache(_currentRender);

  if (getGlobalSettings()->_grayscaleMode) {
    _currentRender.desaturate();
  }

  repaint();
}

void SceneViewer::renderSceneNoPopup()
{
  // Renders a scene using Arnold through Lumiverse.
  // Find the patch, we search for the first ArnoldAnimationPatch we can find.
  ArnoldAnimationPatch* p = getAnimationPatch();

  if (p == nullptr) {
    getRecorder()->log(RENDER, "Render failed. Could not find ArnoldAnimationPatch.");
    getGlobalSettings()->_focusBounds.setWidth(0);
    getGlobalSettings()->_focusBounds.setHeight(0);
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
  p->renderSingleFrameToBuffer(getRig()->getDeviceRaw(), bufptr, width, height);
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

  if (getGlobalSettings()->_grayscaleMode) {
    _preview.desaturate();
  }
}

void SceneViewer::mouseDown(const MouseEvent & event)
{
  if (_currentRender.getWidth() <= 0)
    return;

  switch (getGlobalSettings()->_freezeDrawMode) {
  case DrawMode::BRUSH_ADD:
  {
    // compute current relative coords
    auto imgPt = getAbsImageCoords(event.position);
    paint(imgPt, Colour(0xffffffff));
    break;
  }
  case DrawMode::RECT_ADD:
  case DrawMode::RECT_REMOVE:
  {
    _startPoint = getRelativeImageCoords(event.position);
    break;
  }
  case DrawMode::BRUSH_REMOVE:
  {
    auto imgPt = getAbsImageCoords(event.position);
    paint(imgPt, Colour(0x00000000));
    break;
  }
  default:
    getGlobalSettings()->_focusBounds.setWidth(0);
    getGlobalSettings()->_focusBounds.setHeight(0);
    break;
  }

  repaint();
}

void SceneViewer::mouseUp(const MouseEvent & event)
{
  if (_currentRender.getWidth() <= 0)
    return;

  if (getGlobalSettings()->_freezeDrawMode == DrawMode::RECT_ADD ||
      getGlobalSettings()->_freezeDrawMode == DrawMode::RECT_REMOVE) {
    _currentPoint = getRelativeImageCoords(event.position);
    Array<Point<float> > pts;
    pts.add(_startPoint);
    pts.add(_currentPoint);

    getGlobalSettings()->_focusBounds = Rectangle<float>::findAreaContainingPoints(pts.getRawDataPointer(), 2);
    
    // fill in the focus bounds
    Point<float> topLeft = getGlobalSettings()->_focusBounds.getTopLeft();
    topLeft.setX(topLeft.getX() * _currentRender.getWidth());
    topLeft.setY(topLeft.getY() * _currentRender.getHeight());

    Point<float> botRight = getGlobalSettings()->_focusBounds.getBottomRight();
    botRight.setX(botRight.getX() * _currentRender.getWidth());
    botRight.setY(botRight.getY() * _currentRender.getHeight());
    Rectangle<float> imgBounds = Rectangle<float>::leftTopRightBottom(topLeft.getX(), topLeft.getY(), botRight.getX(), botRight.getY());

    if (getGlobalSettings()->_freezeDrawMode == DrawMode::RECT_ADD) {
      paintRect(imgBounds, Colour(0xffffffff));
    }
    else {
      paintRect(imgBounds, Colour(0xff000000));
    }

    getGlobalSettings()->_focusBounds.setWidth(0);
    getGlobalSettings()->_focusBounds.setHeight(0);
  }

  repaint();
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_ATTR, true);
}

void SceneViewer::mouseDrag(const MouseEvent & event)
{
  if (_currentRender.getWidth() <= 0)
    return;

  switch (getGlobalSettings()->_freezeDrawMode) {
  case DrawMode::BRUSH_ADD:
  {
    // compute current relative coords
    auto imgPt = getAbsImageCoords(event.position);
    paint(imgPt, Colour(0xffffffff));
    break;
  }
  case DrawMode::RECT_ADD:
  case DrawMode::RECT_REMOVE:
  {
    _currentPoint = getRelativeImageCoords(event.position);
    Array<Point<float> > pts;
    pts.add(_startPoint);
    pts.add(_currentPoint);

    getGlobalSettings()->_focusBounds = Rectangle<float>::findAreaContainingPoints(pts.getRawDataPointer(), 2);
    break;
  }
  case DrawMode::BRUSH_REMOVE:
  {
    auto imgPt = getAbsImageCoords(event.position);
    paint(imgPt, Colour(0xff000000));
    break;
  }
  default:
    getGlobalSettings()->_focusBounds.setWidth(0);
    getGlobalSettings()->_focusBounds.setHeight(0);
    break;
  }

  repaint();
}

void SceneViewer::buttonClicked(Button * b)
{
  if (b->getName() == "None") {
    getGlobalSettings()->_freezeDrawMode = DrawMode::NO_DRAW;
  }
  else if (b->getName() == "Paint") {
    getGlobalSettings()->_freezeDrawMode = DrawMode::BRUSH_ADD;
  }
  else if (b->getName() == "Select") {
    getGlobalSettings()->_freezeDrawMode = DrawMode::RECT_ADD;
  }
  else if (b->getName() == "Erase") {
    getGlobalSettings()->_freezeDrawMode = DrawMode::BRUSH_REMOVE;
  }
  else if (b->getName() == "Clear Mask") {
    clearMask();
  }
  else if (b->getName() == "Show Mask") {
    showMask();
  }
  else if (b->getName() == "Erase Region") {
    getGlobalSettings()->_freezeDrawMode = DrawMode::RECT_REMOVE;
  }

}

void SceneViewer::clearMask()
{
  getGlobalSettings()->_freeze.clear(getGlobalSettings()->_freeze.getBounds(), Colour(0xff000000));
  repaint();
}

void SceneViewer::showMask()
{
  _drawMask = !_drawMask;
  _showMask->setToggleState(_drawMask, dontSendNotification);
  repaint();
}

void SceneViewer::setBrushSize(float size)
{
  _brushSize = size;
}

void SceneViewer::paint(Point<float> pt, Colour c)
{
  Graphics g(getGlobalSettings()->_freeze);
  g.setColour(c);

  g.fillEllipse(pt.getX(), pt.getY(), _brushSize, _brushSize);
}

void SceneViewer::paintRect(Rectangle<float> pt, Colour c)
{
  Graphics g(getGlobalSettings()->_freeze);
  g.setColour(c);

  g.fillRect(pt);
}

Point<float> SceneViewer::getRelativeImageCoords(const Point<float>& pt)
{
  auto lbounds = getLocalBounds();
  lbounds.removeFromTop(_toolbarHeight);

  // determine the image location
  float scaleX = (float)lbounds.getWidth() / _currentRender.getWidth();
  float scaleY = (float)lbounds.getHeight() / _currentRender.getHeight();
  float scale = 0;

  // test xscaling
  if (_currentRender.getHeight() * scaleX <= lbounds.getHeight()) {
    scale = scaleX;
  }
  else if (_currentRender.getWidth() * scaleY <= lbounds.getWidth()) {
    scale = scaleY;
  }

  float imgWidth = _currentRender.getWidth()  * scale;
  float imgHeight = _currentRender.getHeight() * scale;
  float ptHeightAdjust = (float) abs((imgHeight - lbounds.getHeight()) / 2.0);
  float ptWidthAdjust = (float) abs((imgWidth - lbounds.getWidth()) / 2.0);
  
  float x = Lumiverse::clamp((pt.x - ptWidthAdjust) / imgWidth, 0, 1);
  float y = Lumiverse::clamp((pt.y - _toolbarHeight - ptHeightAdjust) / imgHeight, 0, 1);
  
  return Point<float>(x, y);
}

Point<float> SceneViewer::getWorldImageCoords(const Point<float>& pt)
{
  auto lbounds = getLocalBounds();
  lbounds.removeFromTop(_toolbarHeight);

  // determine the image location
  float scaleX = (float)lbounds.getWidth() / _currentRender.getWidth();
  float scaleY = (float)lbounds.getHeight() / _currentRender.getHeight();
  float scale = 0;

  // test xscaling
  if (_currentRender.getHeight() * scaleX <= lbounds.getHeight()) {
    scale = scaleX;
  }
  else if (_currentRender.getWidth() * scaleY <= lbounds.getWidth()) {
    scale = scaleY;
  }

  float imgWidth = _currentRender.getWidth()  * scale;
  float imgHeight = _currentRender.getHeight() * scale;
  float ptHeightAdjust = (float) abs((imgHeight - lbounds.getHeight()) / 2.0);
  float ptWidthAdjust = (float) abs((imgWidth - lbounds.getWidth()) / 2.0);

  float x = pt.x * imgWidth + ptWidthAdjust;
  float y = pt.y * imgHeight + ptHeightAdjust;

  return Point<float>(x, y);
}

Point<float> SceneViewer::getAbsImageCoords(const Point<float>& pt)
{
  auto relPt = getRelativeImageCoords(pt);
  
  return Point<float>(relPt.getX() * _currentRender.getWidth(), relPt.getY() * _currentRender.getHeight());
}
