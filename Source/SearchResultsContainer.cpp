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
}

SearchResultsContainer::~SearchResultsContainer()
{
  for (auto& r : _results) {
    delete r;
  }

  for (auto& r : _newResults)
    delete r;
}

void SearchResultsContainer::paint(Graphics & g)
{
  g.fillAll(Colour(0xff333333));
}

void SearchResultsContainer::resized()
{
  auto lbounds = getLocalBounds();
  int elemWidth = lbounds.getWidth() / _elemsPerRow;
  int elemHeight = elemWidth * (9.0 / 16.0);

  juce::Rectangle<int> row;
  for (int i = 0; i < _results.size(); i++) {
    if (i % _elemsPerRow == 0)
      row = lbounds.removeFromTop(elemHeight);

    _results[i]->setBounds(row.removeFromLeft(elemWidth));
  }
}

void SearchResultsContainer::setWidth(int width)
{
  int elemWidth = width / _elemsPerRow;
  int elemHeight = elemWidth * (9.0 / 16.0);
  int rows = ceil((float)_results.size() / _elemsPerRow);
  int height = elemHeight * rows;
  setBounds(0, 0, width, height);
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
  for (auto& r : _results) {
    if (r->isClusterCenter()) {
      r->getClusterContainer()->sort();
    }
  }
}

void SearchResultsContainer::sort(AttributeSorter* s)
{
  _results.sort(*s);
  resized();
  repaint();
}

Array<AttributeSearchResult*> SearchResultsContainer::getResults()
{
  return _results;
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

    {
      lock_guard<mutex> lock(_resultsLock);
      for (auto& res : _results) {
        if (newResult->avgPixDist(res) < getGlobalSettings()->_jndThreshold) {
          delete newResult;
          return false;
        }
      }

      // also check things in the waiting queue
      for (auto& res : _newResults) {
        if (newResult->avgPixDist(res) < getGlobalSettings()->_jndThreshold) {
          delete newResult;
          return false;
        }
      }
    }

    r->_sampleNo = _sampleId;
    _sampleId++;

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

    for (auto r : _newResults) {
      addAndMakeVisible(r);
      _results.add(r);
    }
    _newResults.clear();
  }

  setWidth(getLocalBounds().getWidth());
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
  setWidth(getLocalBounds().getWidth());
  resized();
  repaint();
  _sampleId = 0;
}

void SearchResultsContainer::remove()
{
  lock_guard<mutex> lock(_resultsLock);

  _newResults.clear();
  _results.clear();
  setWidth(getLocalBounds().getWidth());
  resized();
  repaint();
  _sampleId = 0;
}

bool SearchResultsContainer::isFull()
{
  return (_newResults.size() + _results.size()) > getGlobalSettings()->_maxReturnedScenes;
}

void SearchResultsContainer::cleanUp(int resultsToKeep)
{
  // just in case we get lingering threads, shouldn't cause they should be stopped
  lock_guard<mutex> lock(_resultsLock);

  // for now we do k-means clustering and keep the result closest to the center
  // first check to see if we even need to filter
  if (resultsToKeep >= numResults())
    return;

  // gather everything back up into a single array
  // and delete current cluster centers
  Array<AttributeSearchResult*> elems;

  for (auto& r : _results) {
    if (r->isClusterCenter()) {
      SearchResultsContainer* c = r->getClusterContainer();

      for (auto& r2 : c->getResults()) {
        // no heirarchy allowed at the moment
        elems.add(r2);
      }

      // remove, don't delete, elements from cluster object before deletion
      c->remove();
      delete r;
    }
    else {
      elems.add(r);
    }
  }

  _results.clear();

  // get the centers and the results closest to the centers
  KMeans km;
  auto centers = km.cluster(resultsToKeep, elems, InitMode::FORGY);

  // save closest results to centers
  for (auto& r : centers) {
    double minVal = DBL_MAX;
    AttributeSearchResult* resToKeep = nullptr;

    // iterate through contents
    for (auto& e : r->getClusterContainer()->getResults()) {
      if (e->getSearchResult()->_objFuncVal < minVal) {
        r->setImage(e->getImage());
        r->getSearchResult()->_sampleNo = e->getSearchResult()->_sampleNo;
        r->getSearchResult()->_objFuncVal = e->getSearchResult()->_objFuncVal;
        minVal = e->getSearchResult()->_objFuncVal;
        resToKeep = e;
      }
    }

    _results.add(resToKeep);
  }

  // delete unused items
  for (auto& r : centers) {
    for (auto& e : r->getClusterContainer()->getResults()) {
      if (!_results.contains(e)) {
        delete e;
      }
    }

    r->getClusterContainer()->remove();
    delete r;
  }

  // add new results
  for (auto r : _results) {
    addAndMakeVisible(r);
  }

  setWidth(getLocalBounds().getWidth());
  resized();
  repaint();
}

void SearchResultsContainer::cluster()
{
  lock_guard<mutex> lock(_resultsLock);

  Array<AttributeSearchResult*> elems;

  // gather everything back up into a single array
  // and delete current cluster centers
  for (auto& r : _results) {
    if (r->isClusterCenter()) {
      SearchResultsContainer* c = r->getClusterContainer();

      for (auto& r2 : c->getResults()) {
        // no heirarchy allowed at the moment
        elems.add(r2);
      }

      // remove, don't delete, elements from cluster object before deletion
      c->remove();
      delete r;
    }
    else {
      elems.add(r);
    }
  }

  _results.clear();

  // now that all the current elements are in a single place, run a clustering algorithm
  Array<AttributeSearchResult*> centers;
  string mode = getGlobalSettings()->_clusterMethodName;
  if (mode == "K-Means") {
    centers = kmeansClustering(elems, getGlobalSettings()->_numClusters);
  }
  else if (mode == "Mean Shift") {
    centers = meanShiftClustering(elems, getGlobalSettings()->_meanShiftBandwidth);
  }
  else if (mode == "Spectral Clustering") {
    centers = spectralClustering(elems);
  }

  // assign image to center based on best one in cluster
  // and insert into results
  for (auto& r : centers) {
    double minVal = DBL_MAX;
    
    if (r->isClusterCenter()) {
      // iterate through contents
      for (auto& e : r->getClusterContainer()->getResults()) {
        if (e->getSearchResult()->_objFuncVal < minVal) {
          r->setImage(e->getImage());
          r->getSearchResult()->_sampleNo = e->getSearchResult()->_sampleNo;
          r->getSearchResult()->_objFuncVal = e->getSearchResult()->_objFuncVal;
          minVal = e->getSearchResult()->_objFuncVal;
        }
      }

      r->regenToolTip();
      addAndMakeVisible(r);
      _results.add(r);
    }
    else {
      delete r;
    }
  }

  // calculate cluster stats
  calculateClusterStats();

  setWidth(getLocalBounds().getWidth());
  resized();
  repaint();
}

void SearchResultsContainer::saveResults(string filename)
{
  ofstream file;
  file.open(filename, ios::trunc);

  // all results get saved to a human-readable (but maybe not interpretable) csv file
  // format is: id, obj func val, edit history, scene
  for (auto& r : _results) {
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

      auto newResult = new AttributeSearchResult(r);
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

      addAndMakeVisible(newResult);
      _results.add(newResult);
    }
  }

  setWidth(getLocalBounds().getWidth());
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

    AttributeSearchResult* newResult = new AttributeSearchResult(r);
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

    addAndMakeVisible(newResult);
    _results.add(newResult);
  }

  setWidth(getLocalBounds().getWidth());
  resized();
  repaint();
  getStatusBar()->setStatusMessage("Load complete.");
}

void SearchResultsContainer::appendNewResult(AttributeSearchResult * r)
{
  lock_guard<mutex> lock(_resultsLock);
  addAndMakeVisible(r);
  _results.add(r);
  setWidth(getLocalBounds().getWidth());
  resized();
  repaint();
}

void SearchResultsContainer::setElemsPerRow(int epr)
{
  _elemsPerRow = epr;
  setWidth(getLocalBounds().getWidth());
}

int SearchResultsContainer::numResults()
{
  int count;
  for (auto& r : _results) {
    if (r->isClusterCenter()) {
      // no nested clusters, so can do this
      count += r->getClusterContainer()->getResults().size();
    }
    else {
      count++;
    }
  }

  return count;
}

AttributeSearchResult * SearchResultsContainer::createContainerFor(SearchResult * r)
{
  AttributeSearchResult* newResult = new AttributeSearchResult(r);

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

Array<AttributeSearchResult *> SearchResultsContainer::kmeansClustering(Array<AttributeSearchResult*>& elems, int k)
{
  // mostly this function is a debug one to test that we can place things in
  // the proper GUI components
  if (elems.size() == 0)
    return Array<AttributeSearchResult*>();

  KMeans clusterer;
  return clusterer.cluster(k, elems, InitMode::FORGY);
}

Array<AttributeSearchResult*> SearchResultsContainer::meanShiftClustering(Array<AttributeSearchResult*>& elems, double bandwidth)
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

  Array<AttributeSearchResult*> centerContainers;

  // create containers for centers and add to main container
  int i = 0;
  for (auto& c : centers) {
    SearchResult* r = new SearchResult();
    r->_scene = c;
    r->_cluster = i;
    r->_sampleNo = i;
    AttributeSearchResult* newResult = new AttributeSearchResult(r);

    centerContainers.add(newResult);
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

Array<AttributeSearchResult*> SearchResultsContainer::spectralClustering(Array<AttributeSearchResult*>& elems)
{
  function<double(AttributeSearchResult*, AttributeSearchResult*)> f = [](AttributeSearchResult* x, AttributeSearchResult* y) {
    return (x->getFeatures() - y->getFeatures()).norm();
  };
  SpectralCluster clusterer;

  auto centers = clusterer.cluster(elems, getGlobalSettings()->_numClusters, getGlobalSettings()->_spectralBandwidth);

  return centers;
}

double SearchResultsContainer::daviesBouldin()
{
  //calculate S for each cluster
  Eigen::VectorXd S;
  S.resize(_results.size());

  for (int i = 0; i < _results.size(); i++) {
    S[i] = 0;
    // calculate average distance between centroid and other points
    for (auto& r : _results[i]->getClusterContainer()->getResults()) {
      S[i] += r->dist(_results[i]);
    }
    S[i] = sqrt(S[i] / _results[i]->getClusterContainer()->getResults().size());
  }

  // Calculate R
  Eigen::MatrixXd R;
  R.resize(_results.size(), _results.size());

  for (int i = 0; i < _results.size(); i++) {
    R(i, i) = -1;
    for (int j = i + 1; j < _results.size(); j++) {
      double M = _results[i]->dist(_results[j]);
      R(i, j) = (S[i] + S[j]) / M;
      R(j, i) = R(i, j);
    }
  }

  Eigen::VectorXd D;
  D.resize(_results.size());

  for (int i = 0; i < _results.size(); i++) {
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
  getStatusBar()->setStatusMessage("Clustering finished. k = " + String(_results.size()) + ", DB = " + String(db));
}
