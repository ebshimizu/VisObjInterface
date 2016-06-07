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

SearchResultsContainer::SearchResultsContainer()
{
  _sampleId = 0;
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
  int elemWidth = lbounds.getWidth() / getGlobalSettings()->_clusterElemsPerRow;
  int elemHeight = elemWidth * (9.0 / 16.0);

  juce::Rectangle<int> row;
  for (int i = 0; i < _results.size(); i++) {
    if (i % getGlobalSettings()->_clusterElemsPerRow == 0)
      row = lbounds.removeFromTop(elemHeight);

    _results[i]->setBounds(row.removeFromLeft(elemWidth));
  }
}

void SearchResultsContainer::setWidth(int width)
{
  int elemWidth = width / getGlobalSettings()->_clusterElemsPerRow;
  int elemHeight = elemWidth * (9.0 / 16.0);
  int rows = ceil((float)_results.size() / getGlobalSettings()->_clusterElemsPerRow);
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
    // Check to make sure result is sufficiently different
    Eigen::VectorXd features = r->_scene;

    {
      lock_guard<mutex> lock(_resultsLock);
      for (auto& r : _results) {
        Eigen::VectorXd other = r->getSearchResult()->_scene;

        if ((features - other).norm() < getGlobalSettings()->_jndThreshold)
          return false;
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
