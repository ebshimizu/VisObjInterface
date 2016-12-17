/*
  ==============================================================================

    SceneViewer.cpp
    Created: 15 Dec 2015 5:07:32pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SceneViewer.h"
#include "MainComponent.h"
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
  _tools.add(new ToggleButton("Idea Target"));
  _tools.add(new ToggleButton("Add Pin"));
  _tools.add(new ToggleButton("Select"));

  _tools.getFirst()->triggerClick();
  for (auto b : _tools) {
    b->addListener(this);
    addAndMakeVisible(b);
    b->setRadioGroupId(999);
  }

  _clearMask = new TextButton("Clear Mask");
  _showMask = new TextButton("Show Mask");
  _exitSelectView = new TextButton("Return to Stage");
  _showAllBoxesButton = new TextButton("Show Targets");
  _hideAllBoxesButton = new TextButton("Hide Targets");
  _showAllBoxesButton->setColour(TextButton::ColourIds::buttonOnColourId, Colour(0xff202020));
  _showAllBoxesButton->setColour(TextButton::ColourIds::textColourOnId, Colours::white);
  _showMask->setColour(TextButton::ColourIds::buttonOnColourId, Colour(0xff202020));
  _showMask->setColour(TextButton::ColourIds::textColourOnId, Colours::white);
  _hideAllBoxesButton->setColour(TextButton::ColourIds::buttonOnColourId, Colour(0xff202020));
  _hideAllBoxesButton->setColour(TextButton::ColourIds::textColourOnId, Colours::white);
  _showAllBoxesButton->setToggleState(true, dontSendNotification);
  //_clearMask->addListener(this);
  _showMask->addListener(this);
  _showAllBoxesButton->addListener(this);
  _hideAllBoxesButton->addListener(this);
  _exitSelectView->addListener(this);
  //addAndMakeVisible(_clearMask);
  addAndMakeVisible(_showMask);
  addAndMakeVisible(_showAllBoxesButton);
  addAndMakeVisible(_hideAllBoxesButton);
  addChildComponent(_exitSelectView);

  //_showMask->setColour(TextButton::ColourIds::buttonOnColourId, Colours::lightblue);
  _drawMask = false;
  _hideAllBoxes = false;
  _showSelectionMode = false;
  _brushSize = 50;
}

SceneViewer::~SceneViewer()
{
  for (auto b : _tools) {
    delete b;
  }

  delete _clearMask;
  delete _showMask;
  delete _showAllBoxesButton;
  delete _hideAllBoxesButton;
  delete _exitSelectView;
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
  else if (_showSelectionMode) {
    g.drawImageWithin(_selectionRender, 0, _toolbarHeight, lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);
  }
  else {
    g.drawImageWithin(_currentRender, 0, _toolbarHeight, lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);
  }

  if (_drawMask) {
    g.setOpacity(0.5);
    g.drawImageWithin(getGlobalSettings()->_fgMask, 0, _toolbarHeight, lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);
    g.setOpacity(1);
  }

  // selection mode always visible
  if (getGlobalSettings()->_freezeDrawMode == DrawMode::SELECT_ONLY) {
    g.setColour(Colours::red);

    // draw current
    Array<Point<float> > pts;
    pts.add(getWorldImageCoords(_startPoint) + Point<float>(0, (float)_toolbarHeight));
    pts.add(getWorldImageCoords(_currentPoint) + Point<float>(0, (float)_toolbarHeight));
    Rectangle<float> region = Rectangle<float>::findAreaContainingPoints(pts.getRawDataPointer(), 2);

    g.drawRect(region, 2);

    // draw saved
    Point<float> topLeft = getWorldImageCoords(_selectedRegion.getTopLeft());
    Point<float> bottomRight = getWorldImageCoords(_selectedRegion.getBottomRight());

    g.drawRect(Rectangle<float>::leftTopRightBottom(topLeft.x, topLeft.y + _toolbarHeight, bottomRight.x, bottomRight.y + _toolbarHeight), 2);
  }

  if (!_hideAllBoxes) {
    // draw all bounding boxes on the image. Currently they aren't labeled, but may want to add that
    for (auto b : getGlobalSettings()->_ideaMap) {
      if (b.first == getGlobalSettings()->_activeIdea) {
        g.setColour(Colour(0xffa0a0a0));
      }
      else {
        g.setColour(Colour(0x70a0a0a0));
      }

      Point<float> topLeft = getWorldImageCoords(b.second.getTopLeft());
      Point<float> bottomRight = getWorldImageCoords(b.second.getBottomRight());
      g.drawRect(Rectangle<float>::leftTopRightBottom(topLeft.x, topLeft.y + _toolbarHeight, bottomRight.x, bottomRight.y + _toolbarHeight), 2);

      Rectangle<int> textBox = Rectangle<int>::leftTopRightBottom((int)topLeft.x, (int)topLeft.y + _toolbarHeight - 12, (int)bottomRight.x, (int)topLeft.y + _toolbarHeight - 2);
      g.drawFittedText(String(b.first->getName()), textBox, Justification::bottomLeft, 1);
    }

    g.setColour(Colour(0xffffff00));
    if (getGlobalSettings()->_freezeDrawMode == DrawMode::RECT_PIN) {
      // draw current
      Array<Point<float> > pts;
      pts.add(getWorldImageCoords(_startPoint) + Point<float>(0, (float)_toolbarHeight));
      pts.add(getWorldImageCoords(_currentPoint) + Point<float>(0, (float)_toolbarHeight));
      Rectangle<float> region = Rectangle<float>::findAreaContainingPoints(pts.getRawDataPointer(), 2);

      g.drawRect(region, 2);
    }

    int i = 1;
    for (auto b : getGlobalSettings()->_pinnedRegions) {
      Point<float> topLeft = getWorldImageCoords(b.getTopLeft());
      Point<float> bottomRight = getWorldImageCoords(b.getBottomRight());

      g.drawRect(Rectangle<float>::leftTopRightBottom(topLeft.x, topLeft.y + _toolbarHeight, bottomRight.x, bottomRight.y + _toolbarHeight), 2);

      Rectangle<int> textBox = Rectangle<int>::leftTopRightBottom((int)topLeft.x, (int)topLeft.y + _toolbarHeight - 12, (int)bottomRight.x, (int)topLeft.y + _toolbarHeight - 2);
      g.drawFittedText(String(i), textBox, Justification::bottomLeft, 1);

      i++;
    }
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

  _showAllBoxesButton->setBounds(toolbar.removeFromRight(80).reduced(2));
  _hideAllBoxesButton->setBounds(toolbar.removeFromRight(80).reduced(2));
  _showMask->setBounds(toolbar.removeFromRight(80).reduced(2));
  _exitSelectView->setBounds(toolbar.removeFromRight(80).reduced(2));

  //_clearMask->setBounds(toolbar.removeFromRight(60).reduced(2));
  //_showMask->setBounds(toolbar.removeFromRight(60).reduced(2));
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

  if (event.mods.isLeftButtonDown()) {
    if (event.mods.isCommandDown()) {
      // check regions, select one to add a popup controller to
      auto pt = getRelativeImageCoords(event.position);
      PopupMenu menu;

      DeviceSet affected;
      Rectangle<float> region;
      if (getGlobalSettings()->_freezeDrawMode == DrawMode::RECT_ADD) {
        map<int, shared_ptr<Idea> > ids;
        int i = 1;

        for (auto b : getGlobalSettings()->_ideaMap) {
          if (b.second.contains(pt)) {
            ids[i] = b.first;
            menu.addItem(i, b.first->getName() + ": Modify Intensity / Color");
          }
          i++;
        }

        int result = 0;
        if (ids.size() == 1) {
          result = 1;
        }
        else if (ids.size() == 0) {
          return;
        }
        else {
          result = menu.show();
          if (result == 0)
            return;
        }

        region = getGlobalSettings()->_ideaMap[ids[result]];
      }
      else if (getGlobalSettings()->_freezeDrawMode == DrawMode::SELECT_ONLY) {
        region = _selectedRegion;
      }
      else {
        return;
      }

      // get affected devices
      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
      affected = mc->computeAffectedDevices(region);

      // create a popup window
      ParamShifter* ps = new ParamShifter(affected);
      ps->setSize(300, 24 * 3);

      auto lbounds = getLocalBounds();
      
      vector<Point<float> > pts;
      pts.push_back(getWorldImageCoords(region.getTopLeft()));
      pts.push_back(getWorldImageCoords(region.getBottomRight()));
      Rectangle<float> absRegion = Rectangle<float>::findAreaContainingPoints(pts.data(), 2);
      
      Rectangle<int> loc(this->getScreenBounds().getX() + (int)absRegion.getX(),
        this->getScreenBounds().getY() + (int)absRegion.getY() + _toolbarHeight,
        (int)absRegion.getWidth(), (int)absRegion.getHeight());

      CallOutBox::launchAsynchronously(ps, loc, nullptr);
    }
    else {
      clearSelection();
      _startPoint = getRelativeImageCoords(event.position);
      _currentPoint = getRelativeImageCoords(event.position);
    }
  }

  if (event.mods.isRightButtonDown()) {    
    // context menus
    if (getGlobalSettings()->_freezeDrawMode == DrawMode::RECT_ADD) {
      // check idea rectangle
      auto pt = getRelativeImageCoords(event.position);
      if (!_hideAllBoxes) {
        PopupMenu menu;

        map<int,shared_ptr<Idea> > ids;
        int i = 1;
        
        for (auto b : getGlobalSettings()->_ideaMap) {
          if (b.second.contains(pt)) {
            ids[i] = b.first;
            menu.addItem(i, b.first->getName() + ": Delete");
            i++;

            menu.addItem(i, b.first->getName() + ": Show Selection");
            i++;

            menu.addItem(i, b.first->getName() + ": [DEBUG] Display Selection Info");
          }
          i++;
        }

        int result = menu.show();

        if (result == 0)
          return;
        
        MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

        if (result % 3 == 1) {
          getGlobalSettings()->_ideaMap.erase(ids[result]);
          repaint();
        }
        else if (result % 3 == 2) {
          DeviceSet affected = mc->computeAffectedDevices(getGlobalSettings()->_ideaMap[ids[result - 1]]);
          showSelection(affected);
          mc->setSelectedIds(affected);
        }
        else {
          // want to pop up a dialog showing what's in the box
          mc->debugShowAffectedDevices(getGlobalSettings()->_ideaMap[ids[result - 2]]);
        }
      }
      else {
        if (getGlobalSettings()->_ideaMap[getGlobalSettings()->_activeIdea].contains(pt)) {
          PopupMenu menu;
          menu.addItem(1, "Delete");
          int result = menu.show();

          if (result == 1) {
            getGlobalSettings()->_ideaMap.erase(getGlobalSettings()->_activeIdea);
          }
        }
      }
    }
    else if (getGlobalSettings()->_freezeDrawMode == DrawMode::RECT_PIN) {
      // check pinned regions
      auto pt = getRelativeImageCoords(event.position);
      PopupMenu menu;
      int i = 1;

      for (auto b : getGlobalSettings()->_pinnedRegions) {
        if (b.contains(pt)) {
          menu.addItem(i, String(i) + ": Delete");
        }
        i++;
      }

      if (menu.getNumItems() > 0) {
        int toDelete = menu.show();

        if (toDelete == 0)
          return;

        getGlobalSettings()->_pinnedRegions.removeRange(toDelete - 1, 1);
        repaint();
      }
    }
    else if (getGlobalSettings()->_freezeDrawMode == DrawMode::SELECT_ONLY) {
      // selection manipulation options
      PopupMenu menu;

      menu.addItem(1, "Select Devices");
      menu.addItem(2, "Show Selection");
      menu.addItem(3, "Create View from Selection");

      PopupMenu systems;
      int i = 4;
      auto systemNames = getRig()->getMetadataValues("system");
      vector<string> indexedSystems;
      for (auto s : systemNames) {
        indexedSystems.push_back(s);
        systems.addItem(i, s);
        i++;
      }

      menu.addSubMenu("Create View from Selected System", systems);

      int result = menu.show();

      if (result == 0)
        return;

      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
      DeviceSet affected = mc->computeAffectedDevices(_selectedRegion);

      if (result == 1) {
        mc->setSelectedIds(affected);
      }
      else if (result == 2) {
        showSelection(affected);
      }
      else if (result == 3) {
        mc->createView(affected);
      }
      else if (result >= 4) {
        // filter by system
        DeviceSet filtered(getRig());
        string sys = indexedSystems[result - 4];

        for (auto id : affected.getIds()) {
          if (getRig()->getDevice(id)->getMetadata("system") == sys)
            filtered = filtered.add(id);
        }

        mc->createView(filtered);
      }
    }
  }

  repaint();
}

void SceneViewer::mouseUp(const MouseEvent & event)
{
  if (_currentRender.getWidth() <= 0)
    return;

  if (getGlobalSettings()->_freezeDrawMode == DrawMode::RECT_PIN) {
    if (event.mods.isLeftButtonDown()) {
      _currentPoint = getRelativeImageCoords(event.position);
      Array<Point<float> > pts;
      pts.add(_startPoint);
      pts.add(_currentPoint);
      Rectangle<float> region = Rectangle<float>::findAreaContainingPoints(pts.getRawDataPointer(), 2);

      getGlobalSettings()->_pinnedRegions.add(region);
    }
  }
  else if (getGlobalSettings()->_freezeDrawMode == DrawMode::SELECT_ONLY) {
    if (event.mods.isLeftButtonDown()) {
      _currentPoint = getRelativeImageCoords(event.position);
      Array<Point<float> > pts;
      pts.add(_startPoint);
      pts.add(_currentPoint);
      Rectangle<float> region = Rectangle<float>::findAreaContainingPoints(pts.getRawDataPointer(), 2);

      _selectedRegion = region;
    }
  }

  _startPoint = Point<float>(0, 0);
  _currentPoint = Point<float>(0, 0);
  repaint();
  //getApplicationCommandManager()->invokeDirectly(command::REFRESH_ATTR, true);
}

void SceneViewer::mouseDrag(const MouseEvent & event)
{
  if (_currentRender.getWidth() <= 0)
    return;

  if (event.mods.isLeftButtonDown()) {
    _currentPoint = getRelativeImageCoords(event.position);
    Array<Point<float> > pts;
    pts.add(_startPoint);
    pts.add(_currentPoint);

    if (getGlobalSettings()->_activeIdea != nullptr && getGlobalSettings()->_freezeDrawMode == DrawMode::RECT_ADD) {
      Rectangle<float> region = Rectangle<float>::findAreaContainingPoints(pts.getRawDataPointer(), 2);
      getGlobalSettings()->_ideaMap[getGlobalSettings()->_activeIdea] = region;
    }
  }

  repaint();
}

void SceneViewer::buttonClicked(Button * b)
{
  if (b->getName() == "Idea Target") {
    getGlobalSettings()->_freezeDrawMode = DrawMode::RECT_ADD;
  }
  else if (b->getName() == "Add Pin") {
    getGlobalSettings()->_freezeDrawMode = DrawMode::RECT_PIN;
  }
  else if (b->getName() == "Select") {
    getGlobalSettings()->_freezeDrawMode = DrawMode::SELECT_ONLY;
  }
  else if (b->getName() == "Clear Mask") {
    clearMask();
  }
  else if (b->getName() == "Show Mask") {
    showMask();
  }
  else if (b->getName() == "Return to Stage") {
    hideSelection();
  }
  else if (b->getName() == "Show Targets") {
    if (_hideAllBoxes) {
      _hideAllBoxes = false;
      b->setToggleState(true, dontSendNotification);
      _hideAllBoxesButton->setToggleState(false, dontSendNotification);
      repaint();
    }
  }
  else if (b->getName() == "Hide Targets") {
    if (!_hideAllBoxes) {
      _hideAllBoxes = true;
      b->setToggleState(true, dontSendNotification);
      _showAllBoxesButton->setToggleState(false, dontSendNotification);
      repaint();
    }
  }

  repaint();
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

void SceneViewer::showSelection()
{
  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
  showSelection(mc->computeAffectedDevices(_selectedRegion));
}

void SceneViewer::showSelection(DeviceSet selected)
{
  // render out the scene with only the selected devices
  Snapshot* toRender = new Snapshot(getRig());
  auto data = toRender->getRigData();

  for (auto d : data) {
    d.second->getIntensity()->setValAsPercent(0);
    
    if (d.second->paramExists("color")) {
      d.second->getColor()->setRGB(1, 1, 1);
    }
  }

  for (auto id : selected.getIds()) {
    data[id]->getIntensity()->setValAsPercent(1);
  }

  // render
  _selectionRender = renderImage(toRender, getGlobalSettings()->_renderWidth, getGlobalSettings()->_renderHeight);

  _showSelectionMode = true;
  _exitSelectView->setVisible(true);
  delete toRender;
}

void SceneViewer::hideSelection()
{
  _showSelectionMode = false;
  _exitSelectView->setVisible(false);
}

bool SceneViewer::isInSelectionMode()
{
  return _showSelectionMode;
}

void SceneViewer::clearSelection()
{
  _selectedRegion = Rectangle<float>();
}

SceneViewer::ParamShifter::ParamShifter(DeviceSet affected) : _affected(affected) {
  _intens.setName("intens shift");
  _hue.setName("hue shift");
  _sat.setName("sat shift");

  _intens.setSliderStyle(Slider::SliderStyle::LinearBar);
  _hue.setSliderStyle(Slider::SliderStyle::LinearBar);
  _sat.setSliderStyle(Slider::SliderStyle::LinearBar);

  _intens.setRange(-25, 25, 0.01f);
  _hue.setRange(-30, 30, 0.1f);
  _sat.setRange(-25, 25, 0.1f);

  _intens.setValue(0, dontSendNotification);
  _hue.setValue(0, dontSendNotification);
  _sat.setValue(0, dontSendNotification);

  _intens.addListener(this);
  _hue.addListener(this);
  _sat.addListener(this);

  addAndMakeVisible(_intens);
  addAndMakeVisible(_hue);
  addAndMakeVisible(_sat);

  _state = new Snapshot(getRig());
}

SceneViewer::ParamShifter::~ParamShifter()
{
  delete _state;
}

void SceneViewer::ParamShifter::resized()
{
  auto lbounds = getLocalBounds();

  // labels
  lbounds.removeFromLeft(60);

  _intens.setBounds(lbounds.removeFromTop(24).reduced(2));
  _hue.setBounds(lbounds.removeFromTop(24).reduced(2));
  _sat.setBounds(lbounds.removeFromTop(24).reduced(2));
}

void SceneViewer::ParamShifter::paint(Graphics & g)
{
  g.fillAll(Colour(0xff333333));

  auto lbounds = getLocalBounds();
  auto labels = lbounds.removeFromLeft(60);

  g.setColour(Colours::white);
  g.drawFittedText("Intensity", labels.removeFromTop(24).reduced(2), Justification::centredRight, 1);
  g.drawFittedText("Hue", labels.removeFromTop(24).reduced(2), Justification::centredRight, 1);
  g.drawFittedText("Sat", labels.removeFromTop(24).reduced(2), Justification::centredRight, 1);
}

void SceneViewer::ParamShifter::sliderDragEnded(Slider * s)
{
  // Trigger re-render
  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

  if (mc != nullptr) {
    mc->refreshParams();
    mc->refreshAttr();
    mc->redrawResults();
  }

  if (s->getName() == "intens shift" || s->getName() == "hue shift" || s->getName() == "sat shift") {
    s->setValue(0, dontSendNotification);
    mc->arnoldRenderNoPopup();
  }

  delete _state;
  _state = new Snapshot(getRig());
}

void SceneViewer::ParamShifter::sliderValueChanged(Slider * s)
{
  if (s->getName() == "intens shift") {
    // relative intensity change
    float delta = (float)s->getValue() / 100.0f;

    auto data = _state->getRigData();
    for (auto id : _affected.getIds()) {
      float currentIntens = data[id]->getIntensity()->asPercent();
      getRig()->getDevice(id)->getIntensity()->setValAsPercent(currentIntens + delta);
    }
  }
  else if (s->getName() == "hue shift") {
    // relative hue change
    double delta = s->getValue();

    auto data = _state->getRigData();
    for (auto id : _affected.getIds()) {
      if (data[id]->paramExists("color")) {
        Eigen::Vector3d color = data[id]->getColor()->getHSV();
        getRig()->getDevice(id)->getColor()->setHSV(color[0] + delta, color[1], color[2]);
      }
    }
  }
  else if (s->getName() == "sat shift") {
    // relative hue change
    double delta = s->getValue() / 100.0;

    auto data = _state->getRigData();
    for (auto id : _affected.getIds()) {
      if (data[id]->paramExists("color")) {
        Eigen::Vector3d color = data[id]->getColor()->getHSV();
        getRig()->getDevice(id)->getColor()->setHSV(color[0], color[1] + delta, color[2]);
      }
    }
  }
}
