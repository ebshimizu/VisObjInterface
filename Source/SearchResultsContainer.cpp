/*
  ==============================================================================

    AttributeSearchCluster.cpp
    Created: 16 Feb 2016 3:20:25pm
    Author:  falindrith

  ==============================================================================
*/

#include "SearchResultsContainer.h"
#include "AttributeSorting.h"
#include "AttributeSearch.h"
#include "MeanShift.h"
#include "KMeans.h"
#include "SpectralCluster.h"

SearchResultsContainer::SearchResultsContainer()
{
  _sampleId = 0;
  _elemsPerRow = getGlobalSettings()->_clusterElemsPerRow;
  _columnSize = 250;

  _unclusteredResults = new SearchResultList();
  _unclusteredViewer = new Viewport();
  _unclusteredViewer->setViewedComponent(_unclusteredResults);
}

SearchResultsContainer::~SearchResultsContainer()
{
  delete _unclusteredViewer;
}

void SearchResultsContainer::paint(Graphics & g)
{
  g.fillAll(Colour(0xff333333));
}

void SearchResultsContainer::resized()
{
  // the search results container is organized in columns. The leftmost column
  // is unclustered results, and every column to the right is a cluster.
  auto lbounds = getLocalBounds();
  _unclusteredViewer->setBounds(lbounds.removeFromLeft(_columnSize));
  
  // clusters
  for (int i = 0; i < _clusters.size(); i++) {
    _clusters[i]->setBounds(lbounds.removeFromLeft(_columnSize));
  }
}

void SearchResultsContainer::updateSize()
{
  // fixed sized, maximum visible area, width defined by number of clusters
  auto parentBounds = getParentComponent()->getLocalBounds();
  int width = (1 + _clusters.size()) * _columnSize;
  setBounds(0, 0, width, parentBounds.getHeight());
}

void SearchResultsContainer::sort()
{
  string id = getGlobalSettings()->_currentSortMode;

  if (id == "Attribute Default") {
    DefaultSorter sorter;
    sort(&sorter);
  }
  else if (id == "Average Hue") {
    AvgHueSorter sorter;
    sort(&sorter);
  }
  else if (id == "Average Intensity") {
    AvgBrightSorter sorter;
    sort(&sorter);
  }

  // sort child elements
  // sort clusters instead?
}

void SearchResultsContainer::sort(AttributeSorter* s)
{
  _unclusteredResults->sort(s);
  resized();
  repaint();
}

Array<shared_ptr<SearchResultContainer> > SearchResultsContainer::getResults()
{
  return _allResults;
}

bool SearchResultsContainer::addNewResult(SearchResult * r)
{
  {
    // limit number of total results
    if (isFull()) {
      delete r;
      return false;
    }

    // Check to make sure result is sufficiently different
    // Create new result container
    shared_ptr<SearchResultContainer> newResult = shared_ptr<SearchResultContainer>(new SearchResultContainer(r));

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

    // new results get placed into the new results queue
    {
      lock_guard<mutex> lock(_resultsLock);
      for (auto& res : _allResults) {
        if (newResult->avgPixDist(res.get(), true) < getGlobalSettings()->_jndThreshold) {
          return false;
        }
      }

      // also check things in the waiting queue
      for (auto& res : _newResults) {
        if (newResult->avgPixDist(res.get(), true) < getGlobalSettings()->_jndThreshold) {
          return false;
        }
      }
    }

    r->_sampleNo = _sampleId;
    _sampleId++;

    // add result to new results queue
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
    // goes into unclustered for display, all results for record
    for (auto r : _newResults) {
      _unclusteredResults->addResult(r);
      _allResults.add(r);
    }
    _newResults.clear();
  }

  updateSize();
  _unclusteredResults->resized();
  _unclusteredResults->repaint();

  resized();
  repaint();
}

void SearchResultsContainer::clear()
{
  lock_guard<mutex> lock(_resultsLock);

  _allResults.clear();
  _newResults.clear();
  _unclusteredResults->removeAllResults();
  _clusters.clear();

  updateSize();
  resized();
  repaint();
  _sampleId = 0;
}

bool SearchResultsContainer::isFull()
{
  return (_newResults.size() + numResults()) > getGlobalSettings()->_maxReturnedScenes;
}

void SearchResultsContainer::cleanUp(int resultsToKeep)
{
  // just in case we get lingering threads, shouldn't cause they should be stopped
  lock_guard<mutex> lock(_resultsLock);

  // for now we do k-means clustering and keep the result closest to the center
  // first check to see if we even need to filter
  if (resultsToKeep >= numResults())
    return;

  // delete current cluster centers
  _clusters.clear();
  _unclusteredResults->removeAllResults();

  // get the centers and the results closest to the centers
  KMeans km;
  auto centers = km.cluster(resultsToKeep, _allResults, InitMode::FORGY);

  _allResults.clear();

  // save closest results to centers
  for (auto& r : centers) {
    r->setRepresentativeResult();
    _unclusteredResults->addResult(r->getRepresentativeResult());
    _allResults.add(r->getRepresentativeResult());
  }

  updateSize();
  resized();
  repaint();
}

void SearchResultsContainer::cluster()
{
  lock_guard<mutex> lock(_resultsLock);

  // clear out the clusters, we have everything saved in _allResults
  _clusters.clear();

  // clear out unclustered results
  _unclusteredResults->removeAllResults();

  // now that all the current elements are in a single place, run a clustering algorithm
  Array<shared_ptr<TopLevelCluster> > centers;
  string mode = getGlobalSettings()->_clusterMethodName;
  if (mode == "K-Means") {
    centers = kmeansClustering(_allResults, getGlobalSettings()->_numClusters);
  }
  else if (mode == "Mean Shift") {
    centers = meanShiftClustering(_allResults, getGlobalSettings()->_meanShiftBandwidth);
  }
  else if (mode == "Spectral Clustering") {
    centers = spectralClustering(_allResults);
  }

  // add clusters to our list
  for (auto& c : centers) {
    _clusters.add(c);
  }

  // assign elements to top level clusters
  for (auto& e : _allResults) {
    _clusters[e->getSearchResult()->_cluster]->addToCluster(e);
  }

  // assign image to center based on best one in cluster
  for (auto& c : _clusters) {
    c->setRepresentativeResult();
  }

  // calculate cluster stats
  calculateClusterStats();

  updateSize();
  resized();
  repaint();
}

void SearchResultsContainer::saveResults(string filename)
{
  ofstream file;
  file.open(filename, ios::trunc);

  // all results get saved to a human-readable (but maybe not interpretable) csv file
  // format is: id, obj func val, edit history, scene
  for (auto& r : _allResults) {
    auto result = r->getSearchResult();
    file << result->_sampleNo << "," << result->_objFuncVal << "," << r->getTooltip() << ",";

    Eigen::VectorXd scene = result->_scene;
    for (int i = 0; i < scene.size(); i++) {
      file << scene[i];

      if (i < scene.size() - 2)
        file << ",";
    }
    file << "\n";
  }

  file.close();
}

void SearchResultsContainer::loadResults(string filename)
{
  // clear results
  clear();

  ifstream file(filename);
  string line;

  getStatusBar()->setStatusMessage("Loading results...");

  if (file.is_open()) {
    while (getline(file, line)) {
      stringstream lineStream(line);
      string cell;

      SearchResult* r = new SearchResult();
      vector<double> sceneVals;
      string tooltip;

      int i = 0;
      while (getline(lineStream, cell, ',')) {
        if (i == 0) {
          r->_sampleNo = stoi(cell);
        }
        else if (i == 1) {
          r->_objFuncVal = stod(cell);
        }
        else if (i == 2) {
          tooltip = cell;
        }
        else if (i > 2) {
          sceneVals.push_back(stod(cell));
        }

        i++;
      }
      
      // put vector into the scene, generate image and container
      r->_scene.resize(sceneVals.size());
      for (int i = 0; i < sceneVals.size(); i++) {
        r->_scene[i] = sceneVals[i];
      }

      auto newResult = shared_ptr<SearchResultContainer>(new SearchResultContainer(r));
      newResult->setTooltip(tooltip);

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

      _allResults.add(newResult);
      _unclusteredResults->addResult(newResult);
    }
  }

  _unclusteredResults->resized();
  updateSize();
  resized();
  repaint();
  getStatusBar()->setStatusMessage("Load complete.");
}

void SearchResultsContainer::loadTrace(int selected)
{
  clear();

  // retrieve trace vector, prepend starting element
  vector<DebugData> start = getGlobalSettings()->_loadedTraces[-1];
  auto trace = getGlobalSettings()->_loadedTraces[selected];

  for (auto sample : trace) {
    start.push_back(sample);
  }

  getStatusBar()->setStatusMessage("Loading " + String(trace.size()) + " samples");

  // create containers for each element
  for (auto sample : start) {
    SearchResult* r = new SearchResult();
    r->_scene = sample._scene;
    r->_sampleNo = sample._sampleId;
    r->_objFuncVal = sample._f;

    auto newResult = shared_ptr<SearchResultContainer>(new SearchResultContainer(r));
    newResult->setTooltip("Edit: " + sample._editName + "\nAttribute Value: " + String(sample._f) + "\nAcceptance Chance: " + String(sample._a));
    
    // render
    auto p = getAnimationPatch();
    int width = getGlobalSettings()->_renderWidth;
    int height = getGlobalSettings()->_renderHeight;
    p->setDims(width, height);

    Image img = Image(Image::ARGB, width, height, true);
    uint8* bufptr = Image::BitmapData(img, Image::BitmapData::readWrite).getPixelPointer(0, 0);

    Snapshot* s = vectorToSnapshot(r->_scene);
    p->renderSingleFrameToBuffer(s->getDevices(), bufptr, width, height);
    delete s;
    newResult->setImage(img);

    _allResults.add(newResult);
    _unclusteredResults->addResult(newResult);
  }

  updateSize();
  _unclusteredResults->resized();
  resized();
  repaint();
  getStatusBar()->setStatusMessage("Load complete.");
}

void SearchResultsContainer::setElemsPerRow(int epr)
{
  _elemsPerRow = epr;
  updateSize();
}

int SearchResultsContainer::numResults()
{
  return _allResults.size();
}

SearchResultContainer * SearchResultsContainer::createContainerFor(SearchResult * r)
{
  SearchResultContainer* newResult = new SearchResultContainer(r);

  // render
  auto p = getAnimationPatch();
  int width = getGlobalSettings()->_renderWidth;
  int height = getGlobalSettings()->_renderHeight;
  p->setDims(width, height);

  Image img = Image(Image::ARGB, width, height, true);
  uint8* bufptr = Image::BitmapData(img, Image::BitmapData::readWrite).getPixelPointer(0, 0);

  Snapshot* s = vectorToSnapshot(r->_scene);
  p->renderSingleFrameToBuffer(s->getDevices(), bufptr, width, height);
  delete s;
  newResult->setImage(img);

  return newResult;
}

Array<shared_ptr<TopLevelCluster> > SearchResultsContainer::kmeansClustering(Array<shared_ptr<SearchResultContainer> >& elems, int k)
{
  // mostly this function is a debug one to test that we can place things in
  // the proper GUI components
  if (elems.size() == 0)
    return Array<shared_ptr<TopLevelCluster> >();

  KMeans clusterer;
  return clusterer.cluster(k, elems, InitMode::FORGY);
}

Array<shared_ptr<TopLevelCluster> > SearchResultsContainer::meanShiftClustering(Array<shared_ptr<SearchResultContainer> >& elems, double bandwidth)
{
  // place feature vecs into a vector
  vector<Eigen::VectorXd> features;
  for (auto& e : elems) {
    features.push_back(e->getFeatures());
  }

  function<double(Eigen::VectorXd, Eigen::VectorXd)> distFunc = [](Eigen::VectorXd x, Eigen::VectorXd y) {
    double sum = 0;

    // iterate through vector in groups of 3
    for (int i = 0; i < x.size() / 3; i++) {
      int idx = i * 3;

      sum += sqrt(pow(y[idx] - x[idx], 2) +
        pow(y[idx + 1] - x[idx + 1], 2) +
        pow(y[idx + 2] - x[idx + 2], 2));
    }

    return sum / (x.size() / 3.0);
  };

  MeanShift shifter;
  vector<Eigen::VectorXd> centers = shifter.cluster(features, bandwidth, distFunc);

  Array<shared_ptr<TopLevelCluster> > centerContainers;

  // create containers for centers and add to main container
  int i = 0;
  for (auto& c : centers) {
    auto tlc = shared_ptr<TopLevelCluster>(new TopLevelCluster());
    tlc->_scene = c;
    tlc->setClusterId(i);

    centerContainers.add(tlc);
    i++;
  }

  // add elements to the proper center
  for (int i = 0; i < features.size(); i++) {
    // find closest center
    double minDist = DBL_MAX;
    int minIdx = 0;
    for (int j = 0; j < centers.size(); j++) {
      double dist = distFunc(features[i], centers[j]);
      if (dist < minDist) {
        minDist = dist;
        minIdx = j;
      }
    }

    centerContainers[minIdx]->addToCluster(elems[i]);
  }

  return centerContainers;
}

Array<shared_ptr<TopLevelCluster> > SearchResultsContainer::spectralClustering(Array<shared_ptr<SearchResultContainer> >& elems)
{
  SpectralCluster clusterer;
  auto centers = clusterer.cluster(elems, getGlobalSettings()->_numClusters, getGlobalSettings()->_spectralBandwidth);
  return centers;
}

double SearchResultsContainer::daviesBouldin()
{
  //calculate S for each cluster
  Eigen::VectorXd S;
  S.resize(_clusters.size());

  for (int i = 0; i < _clusters.size(); i++) {
    S[i] = 0;
    // calculate average distance between centroid and other points
    for (auto& r : _clusters[i]->getChildElements()) {
      S[i] += r->dist(_clusters[i]->constructResultContainer().get());
    }
    S[i] = sqrt(S[i] / _clusters[i]->numElements());
  }

  // Calculate R
  Eigen::MatrixXd R;
  R.resize(_clusters.size(), _clusters.size());

  for (int i = 0; i < _clusters.size(); i++) {
    R(i, i) = -1;
    for (int j = i + 1; j < _clusters.size(); j++) {
      double M = _clusters[i]->constructResultContainer()->dist(_clusters[j]->constructResultContainer().get());
      R(i, j) = (S[i] + S[j]) / M;
      R(j, i) = R(i, j);
    }
  }

  Eigen::VectorXd D;
  D.resize(_clusters.size());

  for (int i = 0; i < _clusters.size(); i++) {
    D[i] = R.row(i).maxCoeff();
  }

  // return average of D
  return D.sum() / D.size();
}

void SearchResultsContainer::calculateClusterStats()
{
  // calculate davies-bouldin and save to log
  double db = daviesBouldin();

  getRecorder()->log(SYSTEM, "Clustering Davies-Bouldin Index: " + String(db).toStdString());
  getStatusBar()->setStatusMessage("Clustering finished. k = " + String(_clusters.size()) + ", DB = " + String(db));
}
