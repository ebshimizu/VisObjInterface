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
#include <dlib/clustering.h>

// kmeans typedefs
typedef dlib::matrix<double, 0, 1> sampleType;
typedef dlib::linear_kernel<sampleType> kernelType;

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

  if (id == "Attribute Default")
    sort(&DefaultSorter());
  else if (id == "Average Hue")
    sort(&AvgHueSorter());
  else if (id == "Key Hue")
    sort(&KeyHueSorter());
  else if (id == "Average Intensity")
    sort(&AvgBrightSorter());
  else if (id == "Key Intensity")
    sort(&KeyBrightSorter());
  else if (id == "Key Azimuth Angle")
    sort(&KeyAzmSorter());

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
    Eigen::VectorXd features = newResult->getFeatures();

    {
      lock_guard<mutex> lock(_resultsLock);
      for (auto& res : _results) {
        Eigen::VectorXd other = res->getFeatures();

        if ((features - other).norm() < getGlobalSettings()->_jndThreshold) {
          delete newResult;
          return false;
        }
      }

      // also check things in the waiting queue
      for (auto& res : _newResults) {
        Eigen::VectorXd other = res->getFeatures();

        if ((features - other).norm() < getGlobalSettings()->_jndThreshold) {
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
  _numResults = _results.size();
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
  _numResults = _results.size();
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
  if (resultsToKeep >= _results.size())
    return;

  // get list of cluster results
  list<SearchResult*> results;
  for (auto& r : _results) {
    results.push_back(r->getSearchResult());
  }

  // get the centers and the results closest to the centers
  auto centers = clusterResults(results, resultsToKeep);
  auto keep = getClosestScenesToCenters(results, centers);

  // Copy results into new results list
  // Reason: deleting search result containers deletes the search result too
  Array<AttributeSearchResult*> newResults;
  for (auto& r : keep) {
    auto newResult = new AttributeSearchResult(new SearchResult(*r));

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
    newResults.add(newResult);
  }

  // delete old results
  for (auto& r : _results) {
    delete r;
  }

  _results = newResults;
  setWidth(getLocalBounds().getWidth());
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

  // assign image to center based on best one in cluster
  // and insert into results
  for (auto& r : centers) {
    double minVal = DBL_MAX;

    // iterate through contents
    for (auto& e : r->getClusterContainer()->getResults()) {
      if (e->getSearchResult()->_objFuncVal < minVal) {
        r->setImage(e->getImage());
        r->getSearchResult()->_sampleNo = e->getSearchResult()->_sampleNo;
        minVal = e->getSearchResult()->_objFuncVal;
      }
    }

    addAndMakeVisible(r);
    _results.add(r);
  }

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

  _numResults = _results.size();
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

  _numResults = _results.size();
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
  _numResults = _results.size();
  setWidth(getLocalBounds().getWidth());
  resized();
  repaint();
}

void SearchResultsContainer::setElemsPerRow(int epr)
{
  _elemsPerRow = epr;
  setWidth(getLocalBounds().getWidth());
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
  
  if (k <= 1) {
    // average of all elements is center
    Eigen::VectorXd avg;
    avg.resize(elems.getFirst()->getSearchResult()->_scene.size());
    avg.setZero();

    for (auto e : elems) {
      avg += e->getSearchResult()->_scene;
      e->getSearchResult()->_cluster = 0;
    }
    avg /= (double)elems.size();

    SearchResult* r = new SearchResult();
    r->_cluster = 0;
    r->_scene = avg;

    Array<AttributeSearchResult*> ret;
    AttributeSearchResult* newResult = new AttributeSearchResult(r);

    // add all scenes to this cluster
    for (auto& e : elems) {
      newResult->addToCluster(e);
    }

    ret.add(newResult);
    return ret;
  }
  else if (elems.size() <= k) {
    return elems;
  }
  else {
    // kmeans setup
    dlib::kcentroid<kernelType> kkmeansKernel(kernelType(), 0.001);
    dlib::kkmeans<kernelType> kern(kkmeansKernel);

    vector<sampleType> samples;
    for (auto& e : elems) {
      samples.push_back(dlib::mat(e->getSearchResult()->_scene));
    }
    vector<sampleType> centers;

    // use dlib to get the initial centers
    dlib::pick_initial_centers(k, centers, samples, kern.get_kernel());

    // Run kmeans
    dlib::find_clusters_using_kmeans(samples, centers);

    // create search result objects
    Array<AttributeSearchResult*> ret;
    for (int i = 0; i < centers.size(); i++) {
      Eigen::VectorXd center;
      center.resize(centers[i].nr());

      for (int j = 0; j < centers[i].nr(); j++) {
        center[j] = centers[i](j);
      }

      SearchResult* r = new SearchResult();
      r->_cluster = i;
      r->_scene = center;

      ret.add(createContainerFor(r));
    }

    // assign results to clusters
    for (int i = 0; i < elems.size(); i++) {
      unsigned long center = dlib::nearest_center(centers, samples[i]);
      ret[center]->addToCluster(elems[i]);
    }

    return ret;
  }
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
  vector<Eigen::VectorXd> centers = shifter.cluster(features, 8, distFunc);

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
