/*
  ==============================================================================

    AttributeSearchCluster.cpp
    Created: 16 Feb 2016 3:20:25pm
    Author:  falindrith

  ==============================================================================
*/

#include "AttributeSearchCluster.h"
#include "AttributeSorting.h"

AttributeSearchCluster::AttributeSearchCluster(Array<AttributeSearchResult*> elems) :
  _elems(elems)
{
  for (auto e : _elems) {
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
  int elemWidth = lbounds.getWidth() / getGlobalSettings()->_clusterElemsPerRow;
  int elemHeight = elemWidth * (9.0 / 16.0);

  Rectangle<int> row;
  for (int i = 0; i < _elems.size(); i++) {
    if (i % getGlobalSettings()->_clusterElemsPerRow == 0)
      row = lbounds.removeFromTop(elemHeight);

    _elems[i]->setBounds(row.removeFromLeft(elemWidth).reduced(1));
  }
}

void AttributeSearchCluster::setWidth(int width)
{
  int elemWidth = width / getGlobalSettings()->_clusterElemsPerRow;
  int elemHeight = elemWidth * (9.0 / 16.0);
  int rows = ceil((float)_elems.size() / getGlobalSettings()->_clusterElemsPerRow);
  int height = elemHeight * rows;
  setBounds(0, 0, width, height);
}

void AttributeSearchCluster::sort()
{
  string id = getGlobalSettings()->_currentSortMode;

  if (id == "Attribute Default")
    sort(&DefaultSorter());
  else if (id == "Average Hue")
    sort(&AvgHueSorter());
  else if (id == "Key Hue")
    sort(&KeyHueSorter());
  else if (id == "Average Intensity")
    sort(&AvgBrightSorter());
  else if (id == "Key Intensity")
    sort(&KeyBrightSorter());
  else if (id == "Key Azimuth Angle")
    sort(&KeyAzmSorter());

}

void AttributeSearchCluster::sort(AttributeSorter* s)
{
  _elems.sort(*s);
  resized();
  repaint();
}