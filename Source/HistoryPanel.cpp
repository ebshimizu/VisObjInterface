/*
  ==============================================================================

    HistoryPanel.cpp
    Created: 5 Feb 2016 4:47:40pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "HistoryPanel.h"

//==============================================================================
HistoryPanel::HistoryPanel()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

HistoryPanel::~HistoryPanel()
{
  for (auto h : _history) {
    delete h;
  }
}

void HistoryPanel::paint (Graphics& /* g */)
{

}

void HistoryPanel::resized()
{
  // this object assumes you're putting the history in a vertical arrangement
  // and assumes your image aspect ratio is 16:9
  // images will scale to fit properly though
  // it also sets its own height.
  auto lbounds = getLocalBounds();
  int elemHeight = (int) (lbounds.getWidth() * (9.0 / 16.0));

  for (auto h : _history) {
    h->setBounds(lbounds.removeFromBottom(elemHeight).reduced(2));
  }
}

void HistoryPanel::addHistoryItem(SearchResultContainer* h, int idx)
{
  _history.insert(idx, h);
  addAndMakeVisible(h);
  setWidth(getLocalBounds().getWidth());
  repaint();
}

SearchResultContainer* HistoryPanel::removeHistoryItem(int idx)
{
  SearchResultContainer* h = nullptr;
  if (idx < 0) {
    h = _history.remove(_history.size() - 1);
  }
  else {
    h = _history.remove(idx);
  }

  setWidth(getLocalBounds().getWidth());
  repaint();
  return h;
}

void HistoryPanel::deleteHistoryItem(int idx)
{
  SearchResultContainer* h = nullptr;
  if (idx < 0) {
    h = _history.remove(_history.size() - 1);
  }
  else {
    h = _history.remove(idx);
  }

  delete h;
  setWidth(getLocalBounds().getWidth());
  repaint();
}

void HistoryPanel::clearAllHistory()
{
	for (auto& h : _history) {
		delete h;
	}
	_history.clear();
}

void HistoryPanel::setWidth(int width)
{
  int elemHeight = (int) (width * (9.0 / 16.0));
  int height = (_history.size()) * elemHeight;
  setBounds(0, 0, width, height);
}

int HistoryPanel::getHistorySize()
{
  return _history.size();
}
