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
  int width = (int) (getGlobalSettings()->_renderWidth * getGlobalSettings()->_thumbnailRenderScale);
  int height = (int) (getGlobalSettings()->_renderHeight * getGlobalSettings()->_thumbnailRenderScale);
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
}

SearchResultsViewer::~SearchResultsViewer()
{
  delete _viewer;
}

void SearchResultsViewer::paint (Graphics& g)
{
  g.fillAll(Colour(0xff333333));
}

void SearchResultsViewer::resized()
{
  auto lbounds = getLocalBounds();

  //_historyViewer->setBounds(lbounds.removeFromRight(300));
  //_history->setWidth(_historyViewer->getMaximumVisibleWidth());

  // preserve viewer position
  Point<int> pos = _viewer->getViewPosition();
  _results->updateSize(lbounds.getHeight() - _viewer->getScrollBarThickness(), lbounds.getWidth());
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

void SearchResultsViewer::saveClusters()
{
	_results->saveClustering();
}

void SearchResultsViewer::loadClusters(int i)
{
	_results->loadClustering(i);
}

int SearchResultsViewer::numSavedClusters()
{
	return _results->numSavedClusters();
}

void SearchResultsViewer::clearClusters()
{
	_results->clearClusters();
}

void SearchResultsViewer::updateImages()
{
  _results->updateAllImages();
}

SearchResultsContainer * SearchResultsViewer::getContainer()
{
  return _results;
}

Array<shared_ptr<SearchResultContainer>> SearchResultsViewer::getKCenters(int k, DistanceMetric metric)
{
  return _results->getKCenters(k, metric);
}

shared_ptr<SearchResultContainer> SearchResultsViewer::getBestUnexploitedResult()
{
  return _results->getBestUnexploitedResult();
}

const Array<shared_ptr<SearchResultContainer>>& SearchResultsViewer::getAllResults()
{
  return _results->getAllResults();
}

map<int, shared_ptr<SearchResultContainer>>& SearchResultsViewer::getTerminalScenes()
{
	return _results->getTerminalScenes();
}

map<int, int>& SearchResultsViewer::getLocalSampleCounts()
{
	return _results->getLocalSampleCounts();
}

shared_ptr<SearchResultContainer> SearchResultsViewer::getLastSample()
{
	return _results->getLastSample();
}

bool SearchResultsViewer::addNewResult(shared_ptr<SearchResult> r, int callingThreadId, DistanceMetric metric, bool force)
{
  return _results->addNewResult(r, callingThreadId, metric, force);
}

bool SearchResultsViewer::addNewResult(shared_ptr<SearchResult> r, int callingThreadId, DistanceMetric metric, bool force, Array<shared_ptr<SearchResultContainer>>& _currentResults)
{
  return _results->addNewResult(r, callingThreadId, metric, force, _currentResults);
}
