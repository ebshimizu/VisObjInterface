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

SearchResultsRenderer::SearchResultsRenderer(Array<AttributeSearchResult*> results) :
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
  if (size(_results) == 0)
    return;

  int elemWidth = _width / size(_results);
  int elemHeight = _height;

  int i = 0;
  for (const auto& result : _results) {
    result->setBounds(i * elemWidth, 0, elemWidth, elemHeight);
    i++;
  }
}

void SearchResultsContainer::recluster()
{
  // Retrieve results from containers
  list<SearchResult*> results;
  vector<AttributeSearchResult*> resultContainers;
  lock_guard<mutex> lock(_resultsLock);
  bool regenClusterElems = getGlobalSettings()->_numDisplayClusters != _results.size();

  for (auto r : _results) {
    auto elems = r->getClusterElements();
    for (auto e : elems) {
      results.push_back(e->getSearchResult());
      resultContainers.push_back(e);
    }

    delete r->getSearchResult();
    r->clearSearchResult();
    
    // delete containers if the number of clusters has changed, otherwise keep them
    if (regenClusterElems)
      delete r; // Delete the old cluster centers
  }

  // Remove things from results list if cluster number changed
  if (regenClusterElems)
    _results.clear();

  if (getGlobalSettings()->_numDisplayClusters > results.size()) {
    getGlobalSettings()->_numDisplayClusters = results.size();
    MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

    if (mc != nullptr) {
      mc->refreshClusterDisplay();
    }
  }
  
  // Find clusters
  vector<Eigen::VectorXd> centers = clusterResults(results, getGlobalSettings()->_numDisplayClusters);
  Array<AttributeSearchResult*> renderBatch;

  // This time we probably only need to render the cluster centers again
  int i = 0;
  for (auto& c : centers) {
    SearchResult* s = new SearchResult();
    s->_scene = c;
    //s->_editHistory.add("Cluster Center");
    s->_cluster = i;

    AttributeSearchResult* cluster;
    if (regenClusterElems) {
      cluster = new AttributeSearchResult(s);

      addAndMakeVisible(cluster);
      _results.add(cluster);
    }
    else {
      _results[i]->setSearchResult(s);
      cluster = _results[i];
    }

    renderBatch.add(cluster);

    auto p = getAnimationPatch();
    int width = getGlobalSettings()->_renderWidth;
    int height = getGlobalSettings()->_renderHeight;
    p->setDims(width, height);
    p->setSamples(getGlobalSettings()->_stageRenderSamples);

    Image img = Image(Image::ARGB, width, height, true);
    uint8* bufptr = Image::BitmapData(img, Image::BitmapData::readWrite).getPixelPointer(0, 0);

    Snapshot* center = vectorToSnapshot(c);
    p->renderSingleFrameToBuffer(center->getDevices(), bufptr, width, height);
    delete center;
    cluster->setImage(img);

    i++;
  }

  // Reassign results into the proper clusters.
  i = 0;
  for (auto r : results)
  {
    _results[r->_cluster]->addClusterElement(resultContainers[i]);
    i++;
  }

  // redisplay current hovered cluster if there is one
  for (auto r : _results) {
    if (r->_isShowingCluster)
      r->displayCluster();
  }

  _numResults = results.size();

  getRecorder()->log(ACTION, "Results reclusterd in " + String(centers.size()).toStdString() + " clusters");
}

void SearchResultsContainer::setHeight(int height)
{
  _height = height;
  int elemWidth = height * (16.0 / 9.0);
  int elemHeight = height;
  _width = elemWidth * size(_results);
  setBounds(0, 0, _width, _height);
}

void SearchResultsContainer::markDisplayedCluster(AttributeSearchResult * c)
{
  for (auto a : _results) {
    if (c == a) {
      a->_isShowingCluster = true;
    }
    else {
      a->_isShowingCluster = false;
    }
  }
}

Array<AttributeSearchResult*> SearchResultsContainer::getResults()
{
  return _results;
}

bool SearchResultsContainer::addNewResult(SearchResult * r)
{
  {
    // Check to make sure result is sufficiently different
    Eigen::VectorXd features = r->_scene;
    
    {
      lock_guard<mutex> lock(_resultsLock);
      for (auto& centers : _results) {
        for (auto& elem : centers->getClusterElements()) {
          Eigen::VectorXd other = elem->getSearchResult()->_scene;

          if ((features - other).norm() < getGlobalSettings()->_jndThreshold)
            return false;
        }
      }
    }

    // also limit number of total results
    if (isFull()) {
      delete r;
      return false;
    }

    // Create new result container
    AttributeSearchResult* newResult = new AttributeSearchResult(r);

    // render
    auto p = getAnimationPatch();
    int width = getGlobalSettings()->_renderWidth;
    int height = getGlobalSettings()->_renderHeight;
    p->setDims(width, height);
    p->setSamples(getGlobalSettings()->_stageRenderSamples);

    Image img = Image(Image::ARGB, width, height, true);
    uint8* bufptr = Image::BitmapData(img, Image::BitmapData::readWrite).getPixelPointer(0, 0);

    Snapshot* s = vectorToSnapshot(r->_scene);
    p->renderSingleFrameToBuffer(s->getDevices(), bufptr, width, height);
    delete s;
    newResult->setImage(img);

    // add result to container
    {
      lock_guard<mutex> lock(_resultsLock);
      _newResults.add(newResult);
      Lumiverse::Logger::log(INFO, "Adding new result. In queue: " + String(_newResults.size()).toStdString());
    }
  }
  return true;
}

void SearchResultsContainer::showNewResults()
{
  {
    lock_guard<mutex> lock(_resultsLock);

    getStatusBar()->setStatusMessage("Adding " + String(_newResults.size()) + " results...");

    if (_newResults.size() == 0)
      return;

    // integrate new results
    // We'd like to add the new results to an existing cluster and then recluster everything
    // while maintaining current clustering settings.
    
    if (_results.size() == 0) {
      SearchResult* s = new SearchResult();
      AttributeSearchResult* newCluster = new AttributeSearchResult(s);
      addAndMakeVisible(newCluster);
      _results.add(newCluster);
    }

    AttributeSearchResult* c = _results[0];

    for (auto r : _newResults) {
      addAndMakeVisible(r);
      c->addClusterElement(r);
    }
    _newResults.clear();
  }

  recluster();
  setHeight(_height);
  resized();
  repaint();
}

void SearchResultsContainer::clear()
{
  lock_guard<mutex> lock(_resultsLock);

  for (auto r : _results) {
    delete r;
  }
  for (auto r : _newResults) {
    delete r;
  }

  _newResults.clear();
  _results.clear();
  _numResults = _results.size();
  setHeight(_height);
  resized();
  repaint();
}

bool SearchResultsContainer::isFull()
{
  return _numResults > getGlobalSettings()->_maxReturnedScenes;
}

//==============================================================================
SearchResultsViewer::SearchResultsViewer()
{
  _container = new SearchResultsContainer();
  _viewer = new Viewport();
  _viewer->setViewedComponent(_container);
  addAndMakeVisible(_viewer);

  _displayedCluster = nullptr;
  _detailViewer = new Viewport();
  addAndMakeVisible(_detailViewer);

  _history = new HistoryPanel();
  _historyViewer = new Viewport();
  _historyViewer->setViewedComponent(_history);
  addAndMakeVisible(_historyViewer);
}

SearchResultsViewer::~SearchResultsViewer()
{
  delete _container;
  delete _viewer;
  delete _displayedCluster;
  delete _detailViewer;
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
  int halfBounds = min(200, lbounds.getHeight() / 2);

  _viewer->setBounds(lbounds.removeFromTop(halfBounds));
  _container->setHeight(halfBounds - 20);

  _detailViewer->setBounds(lbounds);
  if (_displayedCluster != nullptr)
    _displayedCluster->setWidth(lbounds.getWidth() - 20); // magic numbers are bad but eh
}

void SearchResultsViewer::redisplay()
{
  _container->recluster();
  _container->resized();
  resized();
}

void SearchResultsViewer::sortDisplayedCluster()
{
  if (_displayedCluster != nullptr) {
    _displayedCluster->sort();
  }
}

void SearchResultsViewer::setBotComponent(Component * c, Component* source)
{
  auto cl = dynamic_cast<AttributeSearchCluster*>(c);
  auto s = dynamic_cast<AttributeSearchResult*>(source);
  if (cl != nullptr) {
    _displayedCluster = cl;
    _detailViewer->setViewedComponent(_displayedCluster, true);

    // Mark the triggering component so it can outline itself
    _container->markDisplayedCluster(s);

    auto lbounds = getLocalBounds();
    lbounds.removeFromRight(302);
    int halfBounds = min(200, lbounds.getHeight() / 2);
    lbounds.removeFromTop(halfBounds);

    _detailViewer->setBounds(lbounds);
    if (_displayedCluster != nullptr)
      _displayedCluster->setWidth(lbounds.getWidth() - 20); // magic numbers are bad but eh;

    repaint();
  }
}

void SearchResultsViewer::showNewResults()
{
  _container->showNewResults();
}

void SearchResultsViewer::clearContainer()
{
  _container->clear();
}

bool SearchResultsViewer::isFull()
{
  return _container->isFull();
}

Array<AttributeSearchResult*> SearchResultsViewer::getResults()
{
  return _container->getResults();
}

bool SearchResultsViewer::addNewResult(SearchResult * r)
{
  return _container->addNewResult(r);
}
