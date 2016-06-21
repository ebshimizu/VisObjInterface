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
  Array<shared_ptr<SearchResultContainer> > copy(_contents);

  _contents.clear();
  return copy;
}

shared_ptr<SearchResultContainer> SearchResultList::operator[](int i)
{
  return _contents[i];
}

int SearchResultList::size()
{
  return _contents.size();
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
  int elemHeight = (width / _cols) * _heightRatio;
  int height = ceil(elemHeight * (_contents.size() / _cols));
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
  resized();
  repaint();
}
