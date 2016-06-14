/*
  ==============================================================================

    AttributeSearchResult.cpp
    Created: 7 Jan 2016 4:59:12pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeSearchResult.h"
#include "MainComponent.h"

//==============================================================================
SearchResultBlender::SearchResultBlender(SearchResult * s) : _target(s)
{
  Snapshot* b = new Snapshot(getRig());
  _base = snapshotToVector(b);
  delete b;

  _blender.setRange(0, 1, 0.01);
  _blender.setSliderStyle(Slider::LinearBar);
  _blender.setValue(0, dontSendNotification);
  _blender.addListener(this);
  addAndMakeVisible(_blender);
}

SearchResultBlender::~SearchResultBlender()
{
}

void SearchResultBlender::paint(Graphics & g)
{
  auto lbounds = getLocalBounds();
  g.setColour(Colours::white);
  g.setFont(12);
  g.drawFittedText("Blend", lbounds.removeFromTop(20), Justification::centred, 1);
}

void SearchResultBlender::resized()
{
  auto lbounds = getLocalBounds();
  lbounds.removeFromTop(20);
  _blender.setBounds(lbounds);
}

void SearchResultBlender::sliderValueChanged(Slider * s)
{
  float a = s->getValue();

  // lerp values
  Eigen::VectorXd interp = (1 - a) * _base + a * _target->_scene;

  // Update rig
  Snapshot* newScene = vectorToSnapshot(interp);
  newScene->loadRig(getRig());
  delete newScene;

  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

  if (mc != nullptr) {
    mc->arnoldRenderNoPopup();
    mc->refreshParams();
    mc->refreshAttr();
    getRecorder()->log(ACTION, "User interpolated scene (" + String(a).toStdString() + "%)");
  }
}


//==============================================================================
AttributeSearchResult::AttributeSearchResult(SearchResult* result) : _result(result)
{
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
  regenToolTip();
  _isHovered = false;

  _clusterContents = new SearchResultsContainer();

  // magic number alert
  _clusterContents->setWidth(820);
  _clusterContents->setElemsPerRow(4);
}

AttributeSearchResult::~AttributeSearchResult()
{
  if (_result != nullptr)
    delete _result;

  delete _clusterContents;
}

void AttributeSearchResult::regenToolTip()
{
  String tt = "";
  bool first = true;
  for (const auto& t : _result->_editHistory) {
    if (!first) {
      tt = tt + " -> " + t->_name;
    }
    else {
      tt = tt + t->_name;
      first = false;
    }
  }

  tt = tt + " (" + String(-_result->_objFuncVal) + ")";
  setTooltip(tt);
}

void AttributeSearchResult::paint (Graphics& g)
{
  if (_isHovered) {
    g.fillAll(Colours::yellow);
  }
  else {
    g.fillAll(Colour(0xff333333));
  }

  auto lbounds = getLocalBounds();
  lbounds.reduce(2, 2);
  g.drawImageWithin(_render, lbounds.getX(), lbounds.getY(), lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);

  auto textArea = lbounds.removeFromTop(25);
  textArea.removeFromLeft(10);
  g.setColour(Colours::black);
  g.setFont(14);
  g.drawText(String(_result->_sampleNo), textArea, Justification::centredLeft, false);

  g.setColour(Colours::white);
  g.setFont(12);
  g.drawText(String(_result->_sampleNo), textArea, Justification::centredLeft, false);
}

void AttributeSearchResult::resized()
{
}

void AttributeSearchResult::setImage(Image img)
{
  _render = img;

  if (getGlobalSettings()->_grayscaleMode)
    _render.desaturate();

  // create feature vector
  Image scaled = _render.rescaled(100, 100);
  _features.resize(100 * 100 * 3);
  for (int y = 0; y < scaled.getHeight(); y++) {
    for (int x = 0; x < scaled.getWidth(); x++) {
      int idx = (y * scaled.getWidth() + x) * 3;
      auto px = scaled.getPixelAt(x, y);
      Eigen::Vector3d Lab = rgbToLab(px.getRed() / 255.0, px.getGreen() / 255.0, px.getBlue() / 255.0);

      _features[idx] = Lab[0];
      _features[idx + 1] = Lab[1];
      _features[idx + 2] = Lab[2];
    }
  }
}

void AttributeSearchResult::clearSearchResult()
{
  if (_result != nullptr)
    delete _result;

  _result = nullptr;
}

void AttributeSearchResult::mouseDown(const MouseEvent & event)
{
  if (event.mods.isRightButtonDown()) {
    PopupMenu m;
    m.addItem(1, "Move to Stage");
    m.addItem(2, "Repeat Search with Selected");

    const int result = m.show();

    if (result == 1) {
      Snapshot* s = vectorToSnapshot(_result->_scene);
      s->loadRig(getRig());
      delete s;

      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

      if (mc != nullptr) {
        mc->arnoldRender();
        mc->refreshParams();
        mc->refreshAttr();
        getRecorder()->log(ACTION, "User picked scene for stage: " + getTooltip().toStdString());
      }
    }
    else if (result == 2) {
      Snapshot* s = vectorToSnapshot(_result->_scene);
      s->loadRig(getRig());
      delete s;
      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

      if (mc != nullptr) {
        mc->refreshParams();
        mc->refreshAttr();
        mc->search();
      }
    }
  }
  else if (event.mods.isLeftButtonDown() && event.mods.isCommandDown()) {
    // popup window to allow blending of current scene with search scene
    SearchResultBlender* popupComponent = new SearchResultBlender(_result);
    popupComponent->setSize(300, 45);
    CallOutBox& cb = CallOutBox::launchAsynchronously(popupComponent, getScreenBounds(), nullptr);
  }
  else if (event.mods.isLeftButtonDown()) {
    if (isClusterCenter()) {
      Viewport* vp = new Viewport();
      vp->setViewedComponent(_clusterContents, false);
      vp->setSize(_clusterContents->getWidth() + vp->getScrollBarThickness(), 300);

      CallOutBox& cb = CallOutBox::launchAsynchronously(vp, getScreenBounds(), nullptr);
    }
  }
}

void AttributeSearchResult::mouseEnter(const MouseEvent & event)
{
  _isHovered = true;
  getGlobalSettings()->_showThumbnailImg = true;
  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
  mc->setThumbImage(_render);
  mc->repaint();
  _clusterContents->repaint();
  getParentComponent()->repaint();
}

void AttributeSearchResult::mouseExit(const MouseEvent & event)
{
  _isHovered = false;
  getGlobalSettings()->_showThumbnailImg = false;
  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
  mc->repaint();
  _clusterContents->repaint();
  getParentComponent();
}

int AttributeSearchResult::compareElements(AttributeSearchResult * first, AttributeSearchResult * second)
{
  double firstScore = first->getSearchResult()->_objFuncVal;
  double secondScore = second->getSearchResult()->_objFuncVal;

  if (firstScore < secondScore)
    return -1;
  if (firstScore > secondScore)
    return 1;
  else
    return 0;
}

Eigen::VectorXd AttributeSearchResult::getFeatures()
{
  return _features;
}

void AttributeSearchResult::setFeatures(Eigen::VectorXd features)
{
  _features = features;
}

bool AttributeSearchResult::isClusterCenter()
{
  return _clusterContents->getResults().size() > 0;
}

SearchResultsContainer * AttributeSearchResult::getClusterContainer()
{
  return _clusterContents;
}

void AttributeSearchResult::addToCluster(AttributeSearchResult * elem)
{
  _clusterContents->appendNewResult(elem);

}

Eigen::Vector3d AttributeSearchResult::rgbToLab(double r, double g, double b)
{
  Eigen::Vector3d xyz = ColorUtils::convRGBtoXYZ(r, g, b, sRGB);
  return ColorUtils::convXYZtoLab(xyz, refWhites[D65] / 100.0);
}

double AttributeSearchResult::dist(AttributeSearchResult * y)
{
  return dist(y->getFeatures());
}

double AttributeSearchResult::dist(Eigen::VectorXd & y)
{
  double sum = 0;

  // iterate through pixels in groups of 3
  for (int i = 0; i < _features.size() / 3; i++) {
    int idx = i * 3;

    sum += sqrt(pow(y[idx] - _features[idx], 2) +
                pow(y[idx + 1] - _features[idx + 1], 2) +
                pow(y[idx + 2] - _features[idx + 2], 2));
  }

  return sum / ((double)_render.getWidth() * _render.getHeight());
}
