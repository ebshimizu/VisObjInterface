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
AttributeSearchResult::AttributeSearchResult(SearchResult* result) : _result(result)
{
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
  String tt;
  for (const auto& kvp : result->_attrVals) {
    tt = tt + kvp.first + ": " + String(kvp.second) + "\n";
  }

  tt = tt + "Edit History: ";
  for (const auto& t : result->_editHistory) {
    tt = tt + editTypeToString(t) + " ";
  }
  setTooltip(tt);
  _isShowingCluster = false;
}

AttributeSearchResult::~AttributeSearchResult()
{
  if (_result != nullptr)
    delete _result;

  for (auto s : _clusterElems) {
    delete s;
  }
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
}

void AttributeSearchResult::clearSearchResult()
{
  _result = nullptr;
  _clusterElems.clear();
}

void AttributeSearchResult::mouseDown(const MouseEvent & event)
{
  if (event.mods.isRightButtonDown()) {
    PopupMenu m;
    m.addItem(1, "Move to Stage");
    m.addItem(2, "Move to Stage and Render");
    m.addItem(3, "Repeat Search with Selected");

    const int result = m.show();

    if (result == 1) {
      Snapshot* s = vectorToSnapshot(_result->_scene);
      s->loadRig(getRig());
      delete s;
      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

      if (mc != nullptr) {
        mc->refreshParams();
        mc->refreshAttr();
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
        mc->arnoldRender();
      }
    }
    else if (result == 3) {
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
  //if (event.mods.isLeftButtonDown()) {
  //  if (_clusterElems.size() > 0) {
  //    AttributeSearchCluster* cluster = new AttributeSearchCluster(_clusterElems);
  //    Viewport* v = new Viewport();
  //    v->setViewedComponent(cluster, true);

  //    cluster->setWidth(450);
  //    v->setBounds(0, 0, 450, 300);
  //    cluster->setWidth(v->getMaximumVisibleWidth());
  //    CallOutBox& cb = CallOutBox::launchAsynchronously(v, getScreenBounds(), nullptr);
  //  }
  //}
}

void AttributeSearchResult::mouseEnter(const MouseEvent & event)
{
  // when the mouse enters one of these components, we want to display the cluster contents
  // (which can be displayed in an AttributeSearchCluster object) in the bottom half
  // of the search results window
  if (_clusterElems.size() > 0) {
    AttributeSearchCluster* cluster = new AttributeSearchCluster(_clusterElems);

    // We have to reach all the way up to the main component to do this
    MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

    if (mc != nullptr) {
      mc->setBottomSearchComponent(cluster, this);
    }
  }
}

void AttributeSearchResult::setClusterElements(Array<AttributeSearchResult*> elems)
{
  for (auto s : _clusterElems) {
    delete s;
  }
  _clusterElems.clear();

  _clusterElems = elems;
}

void AttributeSearchResult::addClusterElement(AttributeSearchResult * elem)
{
  _clusterElems.add(elem);
}


//==============================================================================
AttributeSearchCluster::AttributeSearchCluster(Array<AttributeSearchResult*> elems) :
  _elems(elems)
{
  for (auto e : elems) {
    addAndMakeVisible(e);
  }
}

AttributeSearchCluster::~AttributeSearchCluster()
{
}

void AttributeSearchCluster::paint(Graphics & g)
{
  g.fillAll(Colour(0xff333333));
}

void AttributeSearchCluster::resized()
{
  auto lbounds = getLocalBounds();
  int elemHeight = lbounds.getHeight();
  int elemWidth = elemHeight * (16.0 / 9.0);

  for (int i = 0; i < _elems.size(); i++) {
    _elems[i]->setBounds(lbounds.removeFromLeft(elemWidth).reduced(1));
  }
}

void AttributeSearchCluster::setHeight(int height)
{
  int elemWidth = height * (16.0 / 9.0);
  int elemHeight = height;
  int width = elemWidth * _elems.size();
  setBounds(0, 0, width, height);
}
