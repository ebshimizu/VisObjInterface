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

void SearchResultsContainer::display(list<SearchResult*>& results)
{
  for (const auto& c : _results) {
    delete c;
  } 
  _results.clear();

  // Find clusters
  if (getGlobalSettings()->_numDisplayClusters > results.size()) {
    getGlobalSettings()->_numDisplayClusters = results.size();

    MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

    if (mc != nullptr) {
      mc->refreshClusterDisplay();
    }
  }

  vector<Eigen::VectorXd> centers = clusterResults(results, getGlobalSettings()->_numDisplayClusters);
  Array<AttributeSearchResult*> renderBatch;

  // special case for 1 result (the k-means library hates it)
  if (results.size() == 1) {
    SearchResult* s = new SearchResult(**(results.begin()));
    s->_cluster = 0;
    (*(results.begin()))->_cluster = 0;

    AttributeSearchResult* cluster = new AttributeSearchResult(s);
    addAndMakeVisible(cluster);
    _results.add(cluster);
    renderBatch.add(cluster);
  }
  else {
    // create searchresult elements for cluster centers
    int i = 0;
    for (auto& c : centers) {
      SearchResult* s = new SearchResult();
      s->_scene = c;
      //s->_editHistory.add("Cluster Center");
      s->_cluster = i;

      AttributeSearchResult* cluster = new AttributeSearchResult(s);

      addAndMakeVisible(cluster);
      _results.add(cluster);
      renderBatch.add(cluster);
      i++;
    }
  }

  // iterate: for each cluster, put the proper attributeSearchResult into the
  // proper cluster, and also add it to the render list.
  for (const auto& result : results) {
    AttributeSearchResult* res = new AttributeSearchResult(result);

    _results[result->_cluster]->addClusterElement(res);
    renderBatch.add(res);
  }

  _numResults = results.size();

  getRecorder()->log(ACTION, "Results displayed in " + String(centers.size()).toStdString() + " clusters");

  // render cluster centers
  (new SearchResultsRenderer(renderBatch))->runThread();
}

void SearchResultsContainer::recluster()
{
  // Retrieve results from containers
  list<SearchResult*> results;
  vector<AttributeSearchResult*> resultContainers;
  lock_guard<mutex> lock(_resultsLock);

  for (auto r : _results) {
    auto elems = r->getClusterElements();
    for (auto e : elems) {
      results.push_back(e->getSearchResult());
      resultContainers.push_back(e);
    }

    delete r->getSearchResult();
    r->clearSearchResult();
    
    // delete containers if the number of clusters has changed, otherwise keep them
    if (getGlobalSettings()->_numDisplayClusters != _results.size())
      delete r; // Delete the old cluster centers
  }

  // Remove things from results list if cluster number changed
  if (getGlobalSettings()->_numDisplayClusters != _results.size())
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
    if (getGlobalSettings()->_numDisplayClusters != _results.size()) {
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
    lock_guard<mutex> lock(_resultsLock);
    // Check to make sure result is sufficiently different
    // also limit number of total results
    if (_numResults > getGlobalSettings()->_maxReturnedScenes) {
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
    newResult->setImage(img);

    // add result to container
    _newResults.add(newResult);
  }
  return true;
}

void SearchResultsContainer::showNewResults()
{
  {
    lock_guard<mutex> lock(_resultsLock);

    if (_newResults.size() == 0)
      return;

    // integrate new results
    SearchResult* s = new SearchResult();
    AttributeSearchResult* c = new AttributeSearchResult(s);
    for (auto r : _newResults) {
      addAndMakeVisible(r);
      c->addClusterElement(r);
    }
    _newResults.clear();

    addAndMakeVisible(c);
    _results.add(c);
  }

  recluster();
  setHeight(_height);
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

void SearchResultsViewer::display(list<SearchResult*>& results)
{
  _container->display(results);
  _container->resized();
  resized();
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

void SearchResultsViewer::timerCallback()
{
  _container->showNewResults();
}

Array<AttributeSearchResult*> SearchResultsViewer::getResults()
{
  return _container->getResults();
}

bool SearchResultsViewer::addNewResult(SearchResult * r)
{
  return _container->addNewResult(r);
}
