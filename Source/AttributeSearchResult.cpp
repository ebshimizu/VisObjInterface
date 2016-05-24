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
  Snapshot* b = new Snapshot(getRig(), nullptr);
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
  String tt = "";
  bool first = true;
  for (const auto& t : result->_editHistory) {
    if (!first) {
      tt = tt + " -> " + t->_name;
    }
    else {
      tt = tt + t->_name;
      first = false;
    }
  }

  tt = tt + " (" + String(-result->_objFuncVal) + ")";
  setTooltip(tt);
  _isShowingCluster = false;
}

AttributeSearchResult::~AttributeSearchResult()
{
  if (_result != nullptr)
    delete _result;
}

void AttributeSearchResult::paint (Graphics& g)
{
  if (_isShowingCluster) {
    g.fillAll(Colours::yellow);
  }
  else {
    g.fillAll(Colour(0xff333333));
  }

  auto lbounds = getLocalBounds();
  lbounds.reduce(2, 2);
  g.drawImageWithin(_render, lbounds.getX(), lbounds.getY(), lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);
}

void AttributeSearchResult::resized()
{
}

void AttributeSearchResult::setImage(Image img)
{
  _render = img;

  if (getGlobalSettings()->_grayscaleMode)
    _render.desaturate();
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
  else if (event.mods.isLeftButtonDown()) {
    // popup window to allow blending of current scene with search scene
    SearchResultBlender* popupComponent = new SearchResultBlender(_result);
    popupComponent->setSize(300, 45);
    CallOutBox& cb = CallOutBox::launchAsynchronously(popupComponent, getScreenBounds(), nullptr);
  }
}

void AttributeSearchResult::mouseEnter(const MouseEvent & event)
{
  getGlobalSettings()->_showThumbnailImg = true;
  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
  mc->setThumbImage(_render);
  mc->repaint();
}

void AttributeSearchResult::mouseExit(const MouseEvent & event)
{
  getGlobalSettings()->_showThumbnailImg = false;
  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
  mc->repaint();
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