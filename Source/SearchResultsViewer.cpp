/*
  ==============================================================================

    SearchResultsViewer.cpp
    Created: 15 Dec 2015 5:07:58pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SearchResultsViewer.h"
#include "MainComponent.h"

SearchResultsRenderer::SearchResultsRenderer(Array<SearchResultContainer*> results) :
  ThreadWithProgressWindow("Rendering Thumbnails...", true, true), _results(results)
{
}

SearchResultsRenderer::~SearchResultsRenderer()
{
}

void SearchResultsRenderer::run() {
  setProgress(-1);
  int numScenes = _results.size();

  // Set render options
  auto p = getAnimationPatch();
  int width = getGlobalSettings()->_renderWidth * getGlobalSettings()->_thumbnailRenderScale;
  int height = getGlobalSettings()->_renderHeight * getGlobalSettings()->_thumbnailRenderScale;
  p->setDims(width, height);
  p->setSamples(getGlobalSettings()->_thumbnailRenderSamples);

  int i = 0;
  for (auto r : _results) {
    if (threadShouldExit())
      return;

    setProgress((float) i / (float)numScenes);
    setStatusMessage(String(i) + "/" + String(numScenes));

    Image img = Image(Image::ARGB, width, height, true);
    uint8* bufptr = Image::BitmapData(img, Image::BitmapData::readWrite).getPixelPointer(0, 0);

    Snapshot* s = vectorToSnapshot(r->getSearchResult()->_scene);
    p->renderSingleFrameToBuffer(s->getDevices(), bufptr, width, height);
    delete s;

    r->setImage(img);
    i++;
  }

  setProgress(1);
}

void SearchResultsRenderer::threadComplete(bool userPressedCancel)
{
  if (userPressedCancel) {
    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
      "Render Thumbnails",
      "Render canceled");
  }

  auto p = getAnimationPatch();
  int width = getGlobalSettings()->_renderWidth;
  int height = getGlobalSettings()->_renderHeight;
  p->setDims(width, height);
  p->setSamples(getGlobalSettings()->_stageRenderSamples);

  delete this;
}

//==============================================================================
SearchResultsViewer::SearchResultsViewer()
{
  _results = new SearchResultsContainer();
  _viewer = new Viewport();
  _viewer->setViewedComponent(_results);
  addAndMakeVisible(_viewer);

  _history = new HistoryPanel();
  _historyViewer = new Viewport();
  _historyViewer->setViewedComponent(_history);
  addAndMakeVisible(_historyViewer);
}

SearchResultsViewer::~SearchResultsViewer()
{
  delete _viewer;
  delete _history;
  delete _historyViewer;
}

void SearchResultsViewer::paint (Graphics& g)
{
  /* This demo code just fills the component's background and
     draws some placeholder text to get you started.

     You should replace everything in this method with your own
     drawing code..
  */

  g.fillAll(Colour(0xff333333));

  g.setColour(Colours::grey);
  auto lbounds = getLocalBounds();
  lbounds.removeFromRight(300);
  g.fillRect(lbounds.removeFromRight(2));
}

void SearchResultsViewer::resized()
{
  auto lbounds = getLocalBounds();

  _historyViewer->setBounds(lbounds.removeFromRight(300));
  _history->setWidth(_historyViewer->getMaximumVisibleWidth());

  lbounds.removeFromRight(2);

  // preserve viewer position
  Point<int> pos = _viewer->getViewPosition();
  _results->setWidth(lbounds.getWidth() - _viewer->getScrollBarThickness());
  _viewer->setViewPosition(pos);

  _viewer->setBounds(lbounds);
}

void SearchResultsViewer::cluster()
{
  _results->cluster();
  resized();
}

void SearchResultsViewer::sort()
{
  _results->sort();
}

void SearchResultsViewer::showNewResults()
{
  // preserve view position
  Point<int> pos = _viewer->getViewPosition();
  _results->showNewResults();
  _viewer->setViewPosition(pos);
}

void SearchResultsViewer::clearContainer()
{
  _results->clear();
}

bool SearchResultsViewer::isFull()
{
  return _results->isFull();
}

void SearchResultsViewer::cleanUp(int resultsToKeep)
{
  _results->cleanUp(resultsToKeep);
}

void SearchResultsViewer::saveResults(string filename)
{
  _results->saveResults(filename);
}

void SearchResultsViewer::loadResults(string filename)
{
  _results->loadResults(filename);
}

void SearchResultsViewer::loadTrace(int selected)
{
  _results->loadTrace(selected);
}

Array<SearchResultContainer*> SearchResultsViewer::getResults()
{
  return _results->getResults();
}

bool SearchResultsViewer::addNewResult(SearchResult * r)
{
  return _results->addNewResult(r);
}
