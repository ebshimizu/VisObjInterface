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

void AttributeSearchResult::mouseDown(const MouseEvent & event)
{
  if (event.mods.isRightButtonDown()) {
    PopupMenu m;
    m.addItem(1, "Move to Stage");
    m.addItem(2, "Move to Stage and Render");
    m.addItem(3, "Repeat Search with Selected");

    const int result = m.show();

    if (result == 1) {
      _result->_scene->loadRig(getRig());
      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

      if (mc != nullptr) {
        mc->refreshParams();
        mc->refreshAttr();
      }
    }
    else if (result == 2) {
      _result->_scene->loadRig(getRig());

      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

      if (mc != nullptr) {
        mc->refreshParams();
        mc->refreshAttr();
        mc->arnoldRender();
      }
    }
    else if (result == 3) {
      _result->_scene->loadRig(getRig());
      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

      if (mc != nullptr) {
        mc->refreshParams();
        mc->refreshAttr();
        mc->search();
      }
    }
  }
  if (event.mods.isLeftButtonDown()) {
    if (_clusterElems.size() > 0) {
      AttributeSearchCluster* cluster = new AttributeSearchCluster(_clusterElems);
      Viewport* v = new Viewport();
      v->setViewedComponent(cluster, true);

      cluster->setWidth(450);
      v->setBounds(0, 0, 450, 300);
      cluster->setWidth(v->getMaximumVisibleWidth());
      CallOutBox& cb = CallOutBox::launchAsynchronously(v, getScreenBounds(), nullptr);
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
  lbounds.reduce(1, 1);

  int elemWidth = lbounds.getWidth() / _elemsPerRow;
  int rows = ceil((_elems.size() / (float)_elemsPerRow));
  int elemHeight = lbounds.getHeight() / rows;

  for (int r = 0; r < rows; r++) {
    auto rbounds = lbounds.removeFromTop(elemHeight);
    for (int c = 0; c < _elemsPerRow; c++) {
      if (r * _elemsPerRow + c >= _elems.size())
        break;

      _elems[r * _elemsPerRow + c]->setBounds(rbounds.removeFromLeft(elemWidth).reduced(1));
    }
  }
}

void AttributeSearchCluster::setWidth(int width)
{
  int rows = (int)(size(_elems) / _elemsPerRow) + 1;
  int elemWidth = width /_elemsPerRow;
  int elemHeight = elemWidth * (9.0 / 16.0);
  int height = rows * elemHeight;
  setBounds(0, 0, width, height);
}
