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

void HistoryPanel::paint (Graphics& g)
{

}

void HistoryPanel::resized()
{
  // this object assumes you're putting the history in a vertical arrangement
  // and assumes your image aspect ratio is 16:9
  // images will scale to fit properly though
  // it also sets its own height.
  auto lbounds = getLocalBounds();
  int elemHeight = lbounds.getWidth() * (9.0 / 16.0);

  for (auto h : _history) {
    h->setBounds(lbounds.removeFromBottom(elemHeight).reduced(2));
  }
  for (auto r : _redo) {
    r->setBounds(lbounds.removeFromBottom(elemHeight).reduced(2));
  }
}

void HistoryPanel::addHistoryItem(HistoryEntry * h, int idx)
{
  _history.insert(idx, h);
  addAndMakeVisible(h);
  setWidth(getLocalBounds().getWidth());
  repaint();
}

HistoryEntry * HistoryPanel::removeHistoryItem(int idx)
{
  HistoryEntry* h = nullptr;
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
  HistoryEntry* h = nullptr;
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

void HistoryPanel::addRedoItem(HistoryEntry * h, int idx)
{
  _redo.insert(idx, h);
  addAndMakeVisible(h);
  setWidth(getLocalBounds().getWidth());
  repaint();
}

HistoryEntry * HistoryPanel::removeRedoItem(int idx)
{
  HistoryEntry* h = nullptr;
  if (idx < 0) {
    h = _redo.remove(_redo.size() - 1);
  }
  else {
    h = _redo.remove(idx);
  }

  setWidth(getLocalBounds().getWidth());
  repaint();
  return h;
}

void HistoryPanel::deleteRedoItem(int idx)
{
  HistoryEntry* h = nullptr;
  if (idx < 0) {
    h = _redo.remove(_history.size() - 1);
  }
  else {
    h = _redo.remove(idx);
  }

  delete h;
  setWidth(getLocalBounds().getWidth());
  repaint();
}

void HistoryPanel::clearRedo()
{
  for (auto h : _redo) {
    delete h;
  }
  _redo.clear();
}


void HistoryPanel::setWidth(int width)
{
  int elemHeight = (int) (width * (9.0 / 16.0));
  int height = (_history.size() + _redo.size()) * elemHeight;
  setBounds(0, 0, width, height);
}
