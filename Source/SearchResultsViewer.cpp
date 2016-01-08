/*
  ==============================================================================

    SearchResultsViewer.cpp
    Created: 15 Dec 2015 5:07:58pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SearchResultsViewer.h"

SearchResultsContainer::SearchResultsContainer()
{

}

SearchResultsContainer::~SearchResultsContainer()
{
  for (const auto& c : _results) {
    delete c;
  }
}

void SearchResultsContainer::paint(Graphics & g)
{
  g.fillAll(Colour(0xff333333));
}

void SearchResultsContainer::resized()
{
  int elemWidth = _width / _resultsPerRow;
  int elemHeight = elemWidth * (9.0 / 16.0);

  int i = 0;
  for (const auto& result : _results) {
    int r = i / 8;
    int c = i % 8;

    result->setBounds(c * elemWidth, r * elemHeight, elemWidth, elemHeight);
    i++;
  }
}

void SearchResultsContainer::display(vector<SearchResult*> results)
{
  for (const auto& c : _results) {
    delete c;
  } 

  for (const auto& result : results) {
    AttributeSearchResult* res = new AttributeSearchResult(result);
    addAndMakeVisible(res);

    _results.add(res);
  }

  setWidth(_width);
}

void SearchResultsContainer::setWidth(int width)
{
  _width = width;
  int rows = (int)(size(_results) / _resultsPerRow) + 1;
  int elemWidth = _width / _resultsPerRow;
  int elemHeight = elemWidth * (9.0 / 16.0);
  _height = rows * elemHeight;
  setBounds(0, 0, _width, _height);
  resized();
  repaint();
}

//==============================================================================
SearchResultsViewer::SearchResultsViewer()
{
  _container = new SearchResultsContainer();
  _viewer = new Viewport();
  _viewer->setViewedComponent(_container);
  addAndMakeVisible(_viewer);
}

SearchResultsViewer::~SearchResultsViewer()
{
  delete _container;
  delete _viewer;
}

void SearchResultsViewer::paint (Graphics& g)
{
  /* This demo code just fills the component's background and
     draws some placeholder text to get you started.

     You should replace everything in this method with your own
     drawing code..
  */

  g.fillAll(Colour(0xff333333));
}

void SearchResultsViewer::resized()
{
  auto bounds = getLocalBounds();

  _viewer->setBounds(bounds);
  _container->setWidth(_viewer->getMaximumVisibleWidth());
}

void SearchResultsViewer::display(vector<SearchResult*> results)
{
  _container->display(results);
}
