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
#include "AttributeSearchCluster.h"

//==============================================================================
AttributeSearchResult::AttributeSearchResult(SearchResult* result) : _result(result)
{
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
  String tt = "";
  bool first = true;
  for (const auto& t : result->_editHistory) {
    if (!first) {
      tt = tt + " -> " + editTypeToString(t);
    }
    else {
      tt = tt + editTypeToString(t);
      first = false;
    }
  }

  tt = tt + "(" + String(-result->_objFuncVal) + ")";
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
  if (_clusterElems.size() > 0)
    return;

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
        mc->refreshParams();
        mc->refreshAttr();
        mc->arnoldRender();
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
}

void AttributeSearchResult::mouseEnter(const MouseEvent & event)
{
  // when the mouse enters one of these components, we want to display the cluster contents
  // (which can be displayed in an AttributeSearchCluster object) in the bottom half
  // of the search results window
  if (_clusterElems.size() > 0) {
    getRecorder()->log(ACTION, "Cluster " + String(_result->_cluster).toStdString() + " hovered");

    AttributeSearchCluster* cluster = new AttributeSearchCluster(_clusterElems);

    // We have to reach all the way up to the main component to do this
    MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
    cluster->sort(&DefaultSorter());
    cluster->sort();

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