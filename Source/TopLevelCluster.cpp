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
  _contents = new SearchResultList();
  _viewer->setViewedComponent(_contents);
  addAndMakeVisible(_viewer);
}

TopLevelCluster::~TopLevelCluster()
{
  delete _viewer;
}

void TopLevelCluster::resized()
{
  // proportionally the top element will take up 33%
  auto lbounds = getLocalBounds();

  if (_rep != nullptr) {
    _rep->setBounds(lbounds.removeFromTop(lbounds.getHeight() * 0.33));
  }

  _contents->setWidth(lbounds.getWidth() - _viewer->getScrollBarThickness());
  _viewer->setBounds(lbounds);
}

void TopLevelCluster::paint(Graphics & g)
{
  // borders likely need to be drawn eventually
}

void TopLevelCluster::addToCluster(shared_ptr<SearchResultContainer> r)
{
  _contents->addResult(r);
  resized();
}

int TopLevelCluster::numElements()
{
  return _contents->size();
}

int TopLevelCluster::getClusterId()
{
  return _id;
}

void TopLevelCluster::setClusterId(int id)
{
  _id = id;
}

Array<shared_ptr<SearchResultContainer> > TopLevelCluster::getChildElements()
{
  Array<shared_ptr<SearchResultContainer> > res;

  for (int i = 0; i < _contents->size(); i++)
    res.add((*_contents)[i]);

  return res;
}

void TopLevelCluster::setRepresentativeResult()
{
  double minVal = DBL_MAX;
  shared_ptr<SearchResultContainer> best = nullptr;

  for (int i = 0; i < _contents->size(); i++) {
    SearchResult* e = (*_contents)[i]->getSearchResult();

    if (e->_objFuncVal < minVal) {
      minVal = e->_objFuncVal;
      best = (*_contents)[i];
    }
  }

  // copy best
  if (_rep != nullptr) {
    _rep = nullptr;
  }

  _rep = shared_ptr<SearchResultContainer>(new SearchResultContainer(new SearchResult(*best->getSearchResult())));
  _rep->setImage(best->getImage());
  _scene = _rep->getSearchResult()->_scene;
  _features = best->getFeatures();
  addAndMakeVisible(_rep.get());
}

shared_ptr<SearchResultContainer> TopLevelCluster::getRepresentativeResult()
{
  return _rep;
}

shared_ptr<SearchResultContainer> TopLevelCluster::constructResultContainer()
{
  auto src = shared_ptr<SearchResultContainer>(new SearchResultContainer(new SearchResult()));
  src->getSearchResult()->_scene = _scene;
  src->setFeatures(_features);

  return src;
}

void TopLevelCluster::sort(AttributeSorter * s)
{
  _contents->sort(s);
}
