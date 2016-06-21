/*
  ==============================================================================

    TopLevelCluster.cpp
    Created: 21 Jun 2016 1:40:40pm
    Author:  eshimizu

  ==============================================================================
*/

#include "TopLevelCluster.h"

TopLevelCluster::TopLevelCluster()
{
  _rep = nullptr;
  _viewer = new Viewport();
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

  _viewer->setBounds(lbounds);
}

void TopLevelCluster::paint(Graphics & g)
{
  // borders likely need to be drawn eventually
}

void TopLevelCluster::addToCluster(SearchResultContainer * r)
{
  _results.add(r);
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

shared_ptr<SearchResultContainer> TopLevelCluster::constructResultContainer()
{
  auto src = shared_ptr<SearchResultContainer>(new SearchResultContainer(new SearchResult()));
  src->getSearchResult()->_scene = _scene;
  src->setFeatures(_features);

  return src;
}
