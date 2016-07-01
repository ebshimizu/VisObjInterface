/*
  ==============================================================================

    SearchResultList.cpp
    Created: 21 Jun 2016 4:55:23pm
    Author:  eshimizu

  ==============================================================================
*/

#include "SearchResultList.h"

SearchResultList::SearchResultList()
{
  _cols = 1;
  _heightRatio = 9.0 / 16.0;
}

SearchResultList::SearchResultList(int cols, float heightRatio) :
  _cols(cols), _heightRatio(heightRatio)
{
}

SearchResultList::~SearchResultList()
{
}

void SearchResultList::addResult(shared_ptr<SearchResultContainer> result)
{
  addAndMakeVisible(result.get());
  _contents.add(result);
}

void SearchResultList::removeResult(int index)
{
  auto r = _contents.remove(index);
  removeChildComponent(getIndexOfChildComponent(r.get()));
}

Array<shared_ptr<SearchResultContainer>> SearchResultList::removeAllResults()
{
  removeAllChildren();
  Array<shared_ptr<SearchResultContainer> > results;

  // recursively look through children to find leaf nodes (don't need cluster centers)
  for (auto& r : _contents) {
    if (r->isClusterCenter()) {
      results.addArray(r->getResults());
    }
    else {
      results.add(r);
    }
  }

  _contents.clear();
  return results;
}

Array<shared_ptr<SearchResultContainer>> SearchResultList::getAllResults()
{
  return Array<shared_ptr<SearchResultContainer>>(_contents);
}

shared_ptr<SearchResultContainer> SearchResultList::operator[](int i)
{
  return _contents[i];
}

int SearchResultList::size()
{
  return _contents.size();
}

int SearchResultList::numElements()
{
  int count = 0;
  for (auto r : _contents) {
    count += r->numResults();
  }
  return count;
}

void SearchResultList::resized()
{
  // column layout
  auto lbounds = getLocalBounds();
  int elemWidth = lbounds.getWidth() / _cols;
  int elemHeight = elemWidth * _heightRatio;

  // place elements into rows
  Rectangle<int> row;
  for (int i = 0; i < _contents.size(); i++) {
    if (i % _cols == 0) {
      row = lbounds.removeFromTop(elemHeight);
    }

    _contents[i]->setBounds(row.removeFromLeft(elemWidth));
  }
}

void SearchResultList::paint(Graphics & g)
{
}

void SearchResultList::setWidth(int width)
{
  int elemHeight = (int) ((width / _cols) * _heightRatio);
  int rows = (int)ceil((float)_contents.size() / _cols);
  int height = rows * elemHeight;
  setBounds(0, 0, width, height);
  resized();
}

void SearchResultList::setCols(int cols)
{
  _cols = cols;
  setWidth(getLocalBounds().getWidth());
}

void SearchResultList::sort(AttributeSorter * s)
{
  _contents.sort(*s);

  // nested sort
  for (int i = 0; i < _contents.size(); i++) {
    _contents[i]->sort(s);
  }

  resized();
  repaint();
}
