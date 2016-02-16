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

void AttributeSearchCluster::sort()
{
  string id = getGlobalSettings()->_currentSortMode;

  if (id == "Attribute Default")
    sort(&DefaultSorter());
  else if (id == "Average Hue")
    sort(&AvgHueSorter());
}

void AttributeSearchCluster::sort(AttributeSorter* s)
{
  _elems.sort(*s);
  resized();
  repaint();
}