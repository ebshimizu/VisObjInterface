/*
  ==============================================================================

    TopLevelCluster.cpp
    Created: 21 Jun 2016 1:40:40pm
    Author:  eshimizu

  ==============================================================================
*/

#include "TopLevelCluster.h"

GridLayout::GridLayout(Array<SearchResultContainer*> components, int cols, float heightRatio) :
  _components(components), _cols(cols), _heightRatio(heightRatio)
{
  for (auto c : _components) {
    addAndMakeVisible(c);
  }
}

GridLayout::~GridLayout()
{
}

void GridLayout::resized()
{
  auto lbounds = getLocalBounds();
  int elemWidth = lbounds.getWidth() / _cols;
  int elemHeight = elemWidth * _heightRatio;

  juce::Rectangle<int> row;
  for (int i = 0; i < _components.size(); i++) {
    if (i % _cols == 0)
      row = lbounds.removeFromTop(elemHeight);

    _components[i]->setBounds(row.removeFromLeft(elemWidth));
  }

}

void GridLayout::paint(Graphics & g)
{
}

void GridLayout::setWidth(int width)
{
  // calculate height
  int elemHeight = (width / _cols) * _heightRatio;

  // calculate number of rows
  int numRows = ceil(_components.size() / (double)_cols);

  // set bounds
  setSize(width, elemHeight * numRows);
  resized();
}

float GridLayout::setHeightRatio(float heightRatio)
{
  _heightRatio = heightRatio;
  resized();
}

void GridLayout::updateComponents(Array<SearchResultContainer*> newComponents)
{
  _components = newComponents;
  resized();
}

TopLevelCluster::TopLevelCluster()
{
  _rep = nullptr;
  _grid = new GridLayout(_results, 1, 9.0 / 16.0);
  _viewer = new Viewport();
  _viewer->setViewedComponent(_grid);
}

TopLevelCluster::~TopLevelCluster()
{
}

void TopLevelCluster::resized()
{
  // proportionally the top element will take up 33%
  auto lbounds = getLocalBounds();

  if (_rep != nullptr) {
    _rep->setBounds(lbounds.removeFromTop(lbounds.getHeight() * 0.33));
  }

  _grid->setWidth(lbounds.getWidth());
  _viewer->setBounds(lbounds);
}

void TopLevelCluster::paint(Graphics & g)
{
  // borders likely need to be drawn eventually
}

void TopLevelCluster::addToCluster(SearchResultContainer * r)
{
  _results.add(r);
  _grid->updateComponents(_results);
}

int TopLevelCluster::numElements()
{
  int sum = 0;
  for (auto& r : _results) {
    sum += r->numResults();
  }
  return sum;
}

int TopLevelCluster::getClusterId()
{
  return _id;
}

void TopLevelCluster::setClusterId(int id)
{
  _id = id;
}

Array<SearchResultContainer*> TopLevelCluster::getChildElements()
{
  return _results;
}

void TopLevelCluster::setRepresentativeResult()
{
  double minVal = DBL_MAX;
  SearchResultContainer* best = nullptr;

  for (auto& r : _results) {
    SearchResult* e = r->getSearchResult();

    if (e->_objFuncVal < minVal) {
      minVal = e->_objFuncVal;
      best = r;
    }
  }

  // copy best
  if (_rep != nullptr) {
    delete _rep;
  }

  _rep = new SearchResultContainer(new SearchResult(*best->getSearchResult()));
}
