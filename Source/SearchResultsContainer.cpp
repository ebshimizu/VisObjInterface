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
#include "Clustering.h"
#include "MainComponent.h"

#ifndef WIN32
#include <stdio.h>
#endif

//==============================================================================
TopLevelSorter::TopLevelSorter(AttributeSorter* s) : _sorter(s) {

}

TopLevelSorter::~TopLevelSorter()
{
}

int TopLevelSorter::compareElements(shared_ptr<TopLevelCluster> first, shared_ptr<TopLevelCluster> second)
{
	if (first == nullptr)
		return -1;
	else if (second == nullptr)
		return 1;
	else if (first == nullptr && second == nullptr)
		return 0;

	return _sorter->compareElements(first->getRepresentativeResult(), second->getRepresentativeResult());
}

//==============================================================================

SystemExplorer::SystemExplorer(string system) :
  _system(system)
{
  updateAllImages();
}

SystemExplorer::~SystemExplorer()
{
}

void SystemExplorer::addNewResult(shared_ptr<SearchResultContainer> result)
{
  auto nr = shared_ptr<SearchResultContainer>(new SearchResultContainer(shared_ptr<SearchResult>(result->getSearchResult())));
  nr->setSystem(_system);
  addAndMakeVisible(nr.get());
  updateSingleImage(nr);
  _results.add(nr);
  resized();
}

void SystemExplorer::updateAllImages()
{
  for (auto r : _results) {
    updateSingleImage(r);
  }
}

void SystemExplorer::paint(Graphics & g)
{
}

void SystemExplorer::resized()
{
  // resize to fit all elements
  int imgWidth = getHeight();
  int totalWidth = _results.size() * imgWidth;
  setSize(totalWidth, getHeight());

  // now place them
  auto lbounds = getLocalBounds();
  for (auto r : _results) {
    r->setBounds(lbounds.removeFromLeft(imgWidth).reduced(3));
  }
}

void SystemExplorer::updateSingleImage(shared_ptr<SearchResultContainer> result)
{
  // grab the actual snapshot this corresponds to
  shared_ptr<SearchResult> sr = result->getSearchResult();
  Snapshot* cfg = vectorToSnapshot(sr->_scene);

  // set all unlocked intensity non-system devices to 0
  for (auto d : cfg->getRigData()) {
    if (isDeviceParamLocked(d.first, "intensity")) {
      // if locked, grab values and update the snapshot
      d.second->setIntensity(getRig()->getDevice(d.first)->getIntensity()->getVal());
      d.second->setParam("color", getRig()->getDevice(d.first)->getParam("color"));
    }

    else if (d.second->getMetadata("system") != _system) {
      d.second->setIntensity(0);
    }
  }

  // now render the thing
  // we'll render at 25% of full res for now, to test speed
  Image preview = renderImage(cfg, getGlobalSettings()->_renderWidth * 0.25, getGlobalSettings()->_renderHeight * 0.25);
  result->setImage(preview);
}

//==============================================================================

SystemExplorerContainer::SystemExplorerContainer()
{

}

SystemExplorerContainer::~SystemExplorerContainer()
{
  clear();
}

void SystemExplorerContainer::paint(Graphics & g)
{
  g.fillAll(Colour(0xff333333));
}

void SystemExplorerContainer::resized()
{
  int height = _rowHeight * _views.size();
  setSize(getWidth(), height);

  auto lbounds = getLocalBounds();
  for (int i = 0; i < _views.size(); i++) {
    _views[i]->setBounds(lbounds.removeFromTop(_rowHeight));
    _explorers[i]->setSize(_views[i]->getWidth(), _views[i]->getMaximumVisibleHeight());
  }
}

void SystemExplorerContainer::setHeight(int height)
{
  _rowHeight = height;
  resized();
}

void SystemExplorerContainer::addContainer(string system)
{
  SystemExplorer* e = new SystemExplorer(system);
  addAndMakeVisible(e);

  Viewport* ev = new Viewport();
  ev->setViewedComponent(e);
  addAndMakeVisible(ev);

  _views.add(ev);
  _explorers.add(e);
}

void SystemExplorerContainer::addResult(shared_ptr<SearchResultContainer> result)
{
  for (auto e : _explorers) {
    e->addNewResult(result);
  }
}

void SystemExplorerContainer::clear()
{
  for (auto e : _explorers)
    delete e;

  for (auto v : _views)
    delete v;

  _views.clear();
  _explorers.clear();
}

void SystemExplorerContainer::updateImages()
{
  for (auto e : _explorers) {
    e->updateAllImages();
  }
}

//==============================================================================

SearchResultsContainer::SearchResultsContainer()
{
  _sampleId = 0;
  _elemsPerRow = getGlobalSettings()->_clusterElemsPerRow;
  _columnSize = 250;
  _notYetClustered = true;

  _unclusteredResults = new SearchResultList();
  _unclusteredViewer = new Viewport();
  _unclusteredViewer->setViewedComponent(_unclusteredResults);
  addAndMakeVisible(_unclusteredViewer);

  addAndMakeVisible(_views);
  _exploreView = new Viewport();
  _exploreView->setViewedComponent(&_views, false);
  addAndMakeVisible(_exploreView);
}

SearchResultsContainer::~SearchResultsContainer()
{
  delete _unclusteredViewer;
}

void SearchResultsContainer::paint(Graphics & g)
{
  g.fillAll(Colour(0xff333333));

  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
  if (mc->isSearchRunning()) {
    if (_resultsSinceLastSort > 0) {
      getStatusBar()->setStatusMessage(String(_resultsSinceLastSort) + " New Unsorted Result" + ((_resultsSinceLastSort > 1) ? "s" : ""), false, true);
    }
    else {
      getStatusBar()->setStatusMessage("Attribute Search Running...");
    }
  }
}

void SearchResultsContainer::resized()
{
  // the search results container is organized in a column. The leftmost column
  // is unclustered results, and everything to the right is the system viewer
  auto lbounds = getLocalBounds();

  auto viewPt = _unclusteredViewer->getViewPosition();
  _unclusteredResults->setCols(1);
  _unclusteredResults->setWidth(_columnSize - _unclusteredViewer->getScrollBarThickness());
  _unclusteredViewer->setBounds(lbounds.removeFromLeft(_columnSize));
  _unclusteredViewer->setViewPosition(viewPt);

  _exploreView->setBounds(lbounds);
  _views.setHeight(200);
  _views.setSize(_exploreView->getMaximumVisibleWidth(), _exploreView->getMaximumVisibleHeight());
}

void SearchResultsContainer::updateSize(int height, int width)
{
  // fixed sized, maximum visible area, width defined by number of clusters
	if (getGlobalSettings()->_clusterDisplay == COLUMNS) {
    if (!_notYetClustered) {
      width = (1 + _clusters.size()) * _columnSize;
    }
	}
	else if (getGlobalSettings()->_clusterDisplay == GRID) {
		resized();
	}
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
  else if (id == "Side Light Style") {
    StyleSorter sorter(SIDE_LIGHT);
    sort(&sorter);
  }
}

void SearchResultsContainer::sort(AttributeSorter* s)
{
  _unclusteredResults->sort(s);
  _resultsSinceLastSort = 0;
	
  if (_clusters.size() == 0) {
    repaint();
    return;
  }

	// for columns, should be fairly easy, just reorder the columns
	if (getGlobalSettings()->_clusterDisplay == COLUMNS) {
		TopLevelSorter sorter(s);
		_clusters.sort(sorter);
	}
	else if (getGlobalSettings()->_clusterDisplay == GRID) {
		// for grid, a bit more complicated
		// we have to sort columns separately from rows
		int numRows = getGlobalSettings()->_numSecondaryClusters;
		int numCols = getGlobalSettings()->_numPrimaryClusters;

		// we pick representative clusters for each column
		Array<shared_ptr<TopLevelCluster> > colReps;

    // maps cluster id to column index
		map<int, int> colIndexes;

		for (int i = 0; i < numCols; i++) {
			double minScore = DBL_MAX;
			shared_ptr<TopLevelCluster> best;
			for (int j = 0; j < numRows; j++) {
				// find the best top level cluster according to attribute score
				int flatIdx = i + j * numCols;

				if (!_clusters[flatIdx] || _clusters[flatIdx].get()->getRepresentativeResult() == nullptr)
					continue;

				if (_clusters[flatIdx]->getRepresentativeResult()->getSearchResult()->_objFuncVal < minScore) {
					minScore = _clusters[flatIdx]->getRepresentativeResult()->getSearchResult()->_objFuncVal;
					best = _clusters[flatIdx];
				}
			}
			colReps.add(best);
			colIndexes[best->getClusterId()] = i;   // Save column number
		}

		// sort the columns
		TopLevelSorter sorter(s);
		colReps.sort(sorter);

    vector<shared_ptr<TopLevelCluster> > newOrder;
		newOrder.resize(numCols * numRows);

		// put columns in proper spot in array
		for (int i = 0; i < numCols; i++) {
			int oldCol = colIndexes[colReps[i]->getClusterId()];
			int destCol = i;

			// move
			for (int j = 0; j < numRows; j++) {
				int flatIdx = oldCol + j * numCols;
				int newFlatIdx = destCol + j * numCols;
				newOrder[newFlatIdx] = _clusters[flatIdx];
			}
		}

    // sort rows
    // basically the same thing but now across rows
    Array<shared_ptr<TopLevelCluster> > rowReps;
    map<int, int> rowIndexes;

    for (int i = 0; i < numRows; i++) {
      double minScore = DBL_MAX;
      shared_ptr<TopLevelCluster> best;
      for (int j = 0; j < numCols; j++) {
        // find the best top level cluster according to attribute score
        int flatIdx = i * numCols + j;

        if (!newOrder[flatIdx] || newOrder[flatIdx].get()->getRepresentativeResult() == nullptr)
          continue;

        if (newOrder[flatIdx]->getRepresentativeResult()->getSearchResult()->_objFuncVal < minScore) {
          minScore = newOrder[flatIdx]->getRepresentativeResult()->getSearchResult()->_objFuncVal;
          best = newOrder[flatIdx];
        }
      }
      rowReps.add(best);
      rowIndexes[best->getClusterId()] = i;   // Save row number
    }

    // sort the rows
    rowReps.sort(sorter);

    vector<shared_ptr<TopLevelCluster> > newRowOrder;
    newRowOrder.resize(numCols * numRows);

    // put rows in proper place
    for (int i = 0; i < numRows; i++) {
      int oldRow = rowIndexes[rowReps[i]->getClusterId()];
      int destRow = i;

      // move
      for (int j = 0; j < numCols; j++) {
        int flatIdx = oldRow * numCols + j;
        int newFlatIdx = destRow * numCols + j;
        newRowOrder[newFlatIdx] = newOrder[flatIdx];
      }
    }

    // copy back to clusters
    _clusters.clear();
    for (auto c : newRowOrder) {
      _clusters.add(c);
    }
	}

  // sort clusters
  for (int i = 0; i < _clusters.size(); i++) {
    if (_clusters[i] == nullptr) {
      // we gotta fill in the blanks in the event of a missing cluster
      _clusters[i] = shared_ptr<TopLevelCluster>(new TopLevelCluster());
    }
    _clusters[i]->sort(s);
    _clusters[i]->resized();
  }

  resized();
  repaint();
}

void SearchResultsContainer::initForSearch()
{
  _views.clear();

  set<string> systems = getUnlockedSystems();

  for (auto s : systems) {
    _views.addContainer(s);
  }

  resized();
}

Array<shared_ptr<SearchResultContainer> > SearchResultsContainer::getResults()
{
  return _allResults;
}

bool SearchResultsContainer::addNewResult(shared_ptr<SearchResult> r, int callingThreadId, DistanceMetric metric, bool force)
{
  auto start = chrono::high_resolution_clock::now();
  getGlobalSettings()->_timings[callingThreadId]._numResults += 1;

  {
    // limit number of total results
    if (isFull()) {
      return false;
    }

    // Check to make sure result is sufficiently different
    // Create new result container
    shared_ptr<SearchResultContainer> newResult = shared_ptr<SearchResultContainer>(new SearchResultContainer(r));

    // render the comparison image
    auto p = getAnimationPatch();
    int width = 100;
    int height = 100;
    p->setDims(width, height);
    p->setSamples(getGlobalSettings()->_stageRenderSamples);

    Image img = Image(Image::ARGB, width, height, true);
    uint8* bufptr = Image::BitmapData(img, Image::BitmapData::readWrite).getPixelPointer(0, 0);

    Snapshot* s = vectorToSnapshot(r->_scene);
    
    auto renderStart = chrono::high_resolution_clock::now();
    p->renderSingleFrameToBuffer(s->getDevices(), bufptr, width, height);
    getGlobalSettings()->_timings[callingThreadId]._addResultRenderTime += chrono::duration<float>(chrono::high_resolution_clock::now() - renderStart).count();

    newResult->setImage(img);

    // new results get placed into the new results queue
    bool earlyExit = false;
    shared_ptr<SearchResultContainer> duplicate;
    if (!force) {
      auto evalStart = chrono::high_resolution_clock::now();
      lock_guard<mutex> lock(_resultsLock);
      for (auto& res : _allResults) {
        if (newResult->dist(res.get(), metric) < getGlobalSettings()->_jndThreshold) {
          earlyExit = true;
          duplicate = res;
          break;
        }
      }

      // also check things in the waiting queue
      for (auto& res : _newResults) {
        if (newResult->dist(res.get(), metric) < getGlobalSettings()->_jndThreshold) {
          earlyExit = true;
          duplicate = res;
          break;
        }
      }
      getGlobalSettings()->_timings[callingThreadId]._addResultEvalTime += chrono::duration<float>(chrono::high_resolution_clock::now() - evalStart).count();
    }

    if (!earlyExit) {
      r->_sampleNo = _sampleId;
      _sampleId++;

      // add result to new results queue

      // do the fullsize render here - actually now 25% render
      width = getGlobalSettings()->_renderWidth * 0.25;
      height = getGlobalSettings()->_renderHeight * 0.25;
      p->setDims(width, height);

      Image fullSizeImg = Image(Image::ARGB, width, height, true);
      uint8* fullBufptr = Image::BitmapData(fullSizeImg, Image::BitmapData::readWrite).getPixelPointer(0, 0);

      auto renderStart = chrono::high_resolution_clock::now();
      p->renderSingleFrameToBuffer(s->getDevices(), fullBufptr, width, height);
      getGlobalSettings()->_timings[callingThreadId]._addResultRenderTime += chrono::duration<float>(chrono::high_resolution_clock::now() - renderStart).count();

      newResult->setImage(fullSizeImg);
      {
        lock_guard<mutex> lock(_resultsLock);
        _newResults.add(newResult);

        // some results have some extra info we have to deal with here
        if (r->_extraData.count("LM Terminal") > 0) {
          _terminalScenes[(int)_terminalScenes.size() + 1] = newResult;
        }
        if (r->_extraData.count("Local Sample") > 0) {
          // Local sample has the parent terminal scene id as the value
          _localSampleCounts[r->_extraData["Local Sample"].getIntValue()] += 1;
        }

        Lumiverse::Logger::log(INFO, "Adding new result. In queue: " + String(_newResults.size()).toStdString());
        _resultsSinceLastSort += 1;
      }
    }
    else {
      lock_guard<mutex> lock(_resultsLock);
      int numDupes = String(duplicate->getSearchResult()->_extraData["duplicates"]).getIntValue();
      duplicate->getSearchResult()->_extraData["duplicates"] = String(numDupes + 1);
      duplicate->regenToolTip();
    }

    getGlobalSettings()->_timings[callingThreadId]._addResultTime += chrono::duration<float>(chrono::high_resolution_clock::now() - start).count();
    delete s;
    return !earlyExit;
  }
}

bool SearchResultsContainer::addNewResult(shared_ptr<SearchResult> r, int callingThreadId, DistanceMetric metric,
  bool force, Array<shared_ptr<SearchResultContainer>>& _currentResult)
{
  bool res = addNewResult(r, callingThreadId, metric, force);

  _currentResult.clear();

  // copy current all results to current result
  {
    lock_guard<mutex> lock(_resultsLock);
    _currentResult.addArray(_allResults);
  }

  return res;
}

void SearchResultsContainer::showNewResults()
{
  {
    lock_guard<mutex> lock(_resultsLock);

    //getStatusBar()->setStatusMessage("Adding " + String(_newResults.size()) + " results...");

    if (_newResults.size() == 0)
      return;

    // integrate new results
    // goes into unclustered for display, all results for record
    for (auto r : _newResults) {
      _unclusteredResults->addResult(r);
      _allResults.add(r);
      _views.addResult(r);
    }
    _newResults.clear();

    if (getGlobalSettings()->_continuousSort) {
      sort();
    }
  }

  updateSize(getLocalBounds().getHeight(), getLocalBounds().getWidth());
  resized();
  repaint();
  _unclusteredResults->repaint();
}

void SearchResultsContainer::clear()
{
  lock_guard<mutex> lock(_resultsLock);

	_terminalScenes.clear();
	_localSampleCounts.clear();
  _allResults.clear();
  _newResults.clear();
  _unclusteredResults->removeAllResults();
  _clusters.clear();
	_savedResults.clear();
  _recentered.clear();

  _notYetClustered = true;
  updateSize(getLocalBounds().getHeight(), getLocalBounds().getWidth());
  resized();
  repaint();
  _sampleId = 0;
  _resultsSinceLastSort = 0;
  _views.clear();
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

	// remove cluster children
	for (auto& c : _clusters) {
		removeChildComponent(getIndexOfChildComponent(c.get()));
	}

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

  updateSize(getLocalBounds().getHeight(), getLocalBounds().getWidth());
  resized();
  repaint();
}

void SearchResultsContainer::cluster()
{
  lock_guard<mutex> lock(_resultsLock);

  // check that something actually exists to be clustered
  if (_allResults.size() == 0)
    return;

  // if the distance metric expects at least one light to be selected as key light, make sure that
  // is true first, otherwise abort now
  if (getGlobalSettings()->_primaryClusterMetric == KEYPARAM || getGlobalSettings()->_secondaryClusterMetric == KEYPARAM) {
    if (getGlobalSettings()->_keyIds.size() == 0) {
      getStatusBar()->setStatusMessage("Clustering Failed! Must designate at least one light as the key light.", true);
      return;
    }
  }

	// remove cluster children
	for (auto& c : _clusters) {
		removeChildComponent(getIndexOfChildComponent(c.get()));
	}

  // clear out the clusters, we have everything saved in _allResults
  _clusters.clear();

  // clear out unclustered results
  _unclusteredResults->removeAllResults();

	// logging: record settings at time of cluster
	SearchMetadata currentState = createSearchMetadata();

  // now that all the current elements are in a single place, run a clustering algorithm
  Array<shared_ptr<TopLevelCluster> > centers;
  ClusterMethod mode = getGlobalSettings()->_primaryClusterMethod;
  bool invert = (getGlobalSettings()->_primaryFocusArea == BACKGROUND) ? true : false;
  bool overrideMask = (getGlobalSettings()->_primaryFocusArea == ALL_IMAGE) ? true : false;
  DistanceMetric dm = getGlobalSettings()->_primaryClusterMetric;

  distFuncType f = [invert, overrideMask, dm](SearchResultContainer* x, SearchResultContainer* y) {
    return x->dist(y, dm, overrideMask, invert);
  };

  switch (mode) {
  case KMEANS:
    centers = Clustering::kmeansClustering(_allResults, getGlobalSettings()->_numPrimaryClusters, f);
    break;
  case MEAN_SHIFT:
    centers = Clustering::meanShiftClustering(_allResults, getGlobalSettings()->_meanShiftBandwidth);
    break;
  case SPECTRAL:
    centers = Clustering::spectralClustering(_allResults, getGlobalSettings()->_numPrimaryClusters, f);
    break;
  case DIVISIVE:
    centers = Clustering::divisiveKMeansClustering(_allResults, getGlobalSettings()->_numPrimaryClusters, f);
    break;
  case TDIVISIVE:
    centers = Clustering::thresholdedKMeansClustering(_allResults, getGlobalSettings()->_primaryDivisiveThreshold, f);
    break;
  case STYLE:
    centers = Clustering::styleClustering(_allResults);
    break;
  default:
    break;
  }

	if (getGlobalSettings()->_clusterDisplay == COLUMNS) {
		// add clusters to our list
		for (auto& c : centers) {
			_clusters.add(c);

			// run subclustering
			c->cluster();

			// assign image to center based on best one in cluster
			c->setRepresentativeResult();

			// when the pointer goes out of scope, hopefully juce picks up on that automatically...
			addAndMakeVisible(c.get());

			// calculate distance to center
			auto ce = c->getContainer();
			for (auto& e : c->getAllChildElements()) {
				e->getSearchResult()->_extraData["Distance to Center"] = String(f(e.get(), ce.get()));
			}
			for (auto& e : c->getChildElements()) {
				e->getSearchResult()->_extraData["Distance to Center"] = String(f(e.get(), ce.get()));
			}
			c->getRepresentativeResult()->getSearchResult()->_extraData["Distance to Center"] = String(f(c->getRepresentativeResult().get(), ce.get()));
		}

		// calculate cluster stats
		//calculateClusterStats();

		saveClusterStats(f, currentState);
	}
	else if (getGlobalSettings()->_clusterDisplay == GRID) {
		// need to do a second clustering here, but first we extract the details of this
		// first clustering
		int numCols, numRows;
		map<int, int> elemToCluster1;
		for (auto& r : _allResults) {
			// elems should be assigned to the right centers here
			elemToCluster1[r->getSearchResult()->_sampleNo] = r->getSearchResult()->_cluster;
		}
		numCols = centers.size();

		ClusterMethod mode2 = getGlobalSettings()->_secondaryClusterMethod;
		bool invert2 = (getGlobalSettings()->_secondaryFocusArea == BACKGROUND) ? true : false;
		bool overrideMask2 = (getGlobalSettings()->_secondaryFocusArea == ALL_IMAGE) ? true : false;
		DistanceMetric dm2 = getGlobalSettings()->_secondaryClusterMetric;

		distFuncType f2 = [invert2, overrideMask2, dm2](SearchResultContainer* x, SearchResultContainer* y) {
			return x->dist(y, dm2, overrideMask2, invert2);
		};

		Array<shared_ptr<TopLevelCluster> > centers2;
		// do the clustering again with secondary options
		switch (mode2) {
		case KMEANS:
			centers2 = Clustering::kmeansClustering(_allResults, getGlobalSettings()->_numSecondaryClusters, f2);
			break;
		case MEAN_SHIFT:
			centers2 = Clustering::meanShiftClustering(_allResults, getGlobalSettings()->_meanShiftBandwidth);
			break;
		case SPECTRAL:
			centers2 = Clustering::spectralClustering(_allResults, getGlobalSettings()->_numSecondaryClusters, f2);
			break;
		case DIVISIVE:
			centers2 = Clustering::divisiveKMeansClustering(_allResults, getGlobalSettings()->_numSecondaryClusters, f2);
			break;
		case TDIVISIVE:
			centers2 = Clustering::thresholdedKMeansClustering(_allResults, getGlobalSettings()->_secondaryDivisiveThreshold, f2);
			break;
    case STYLE:
      centers2 = Clustering::styleClustering(_allResults);
      break;
		default:
			break;
		}
	 
		// save this clustering too
		map<int, int> elemToCluster2;
		for (auto& r : _allResults) {
			// elems should be assigned to the right centers here
			elemToCluster2[r->getSearchResult()->_sampleNo] = r->getSearchResult()->_cluster;
		}
		numRows = centers2.size();
		
		// create a new top level item and add to centers for each grid space
		for (int i = 0; i < numRows; i++) {
			for (int j = 0; j < numCols; j++) {
				auto center = shared_ptr<TopLevelCluster>(new TopLevelCluster());
				center->setClusterId(i * numCols + j);
				_clusters.add(center);
			}
		}

		// for each element, assign to the proper cluster based on the two cluster operations perfomed
		for (auto& r : _allResults) {
			int sampleId = r->getSearchResult()->_sampleNo;
			int clusterId = elemToCluster2[sampleId] * numCols + elemToCluster1[sampleId];
			_clusters[clusterId]->addToCluster(r);
		}

		// make clusters visible and pick representative scene, and then
		// remove all elements from that cluster.
		for (auto& c : _clusters) {
			addAndMakeVisible(c.get());
			c->setRepresentativeResult();
		}

		// logging: save stats
		saveClusterStats(f, f2, currentState, centers, centers2);
	}

  _resultsSinceLastSort = 0;
  updateSize(getLocalBounds().getHeight(), getLocalBounds().getWidth());
  resized();
  repaint();

  _notYetClustered = false;
  getStatusBar()->setStatusMessage("Clustering Complete.");
}

void SearchResultsContainer::saveResults(string filename)
{
  ofstream file;
  file.open(filename, ios::trunc);

  // all results get saved to a human-readable (but maybe not interpretable) csv file
  // format is: id, obj func val, edit history, scene
  for (auto& r : _allResults) {
    auto result = r->getSearchResult();
    file << result->_sampleNo << "," << result->_objFuncVal << ",";
    
    for (auto e : result->_editHistory) {
      file << e->_name << " -> ";
    }
    file << "END,";

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

      shared_ptr<SearchResult> r = shared_ptr<SearchResult>(new SearchResult());
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
      for (i = 0; i < sceneVals.size(); i++) {
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
  updateSize(getLocalBounds().getHeight(), getLocalBounds().getWidth());
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
    shared_ptr<SearchResult> r = shared_ptr<SearchResult>(new SearchResult());
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

  updateSize(getLocalBounds().getHeight(), getLocalBounds().getWidth());
  _unclusteredResults->resized();
  resized();
  repaint();
  getStatusBar()->setStatusMessage("Load complete.");
}

void SearchResultsContainer::setElemsPerRow(int epr)
{
  _elemsPerRow = epr;
  updateSize(getLocalBounds().getHeight(), getLocalBounds().getWidth());
}

int SearchResultsContainer::numResults()
{
  return _allResults.size();
}

SearchResultContainer * SearchResultsContainer::createContainerFor(shared_ptr<SearchResult> r)
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

double SearchResultsContainer::daviesBouldin()
{
  //calculate S for each cluster
  Eigen::VectorXd S;
  S.resize(_clusters.size());

  for (int i = 0; i < _clusters.size(); i++) {
    S[i] = 0;
    // calculate average distance between centroid and other points
    for (auto& r : _clusters[i]->getChildElements()) {
      S[i] += r->dist(_clusters[i]->getContainer().get(), getGlobalSettings()->_primaryClusterMetric);
    }
    S[i] = sqrt(S[i] / _clusters[i]->numElements());
  }

  // Calculate R
  Eigen::MatrixXd R;
  R.resize(_clusters.size(), _clusters.size());

  for (int i = 0; i < _clusters.size(); i++) {
    R(i, i) = -1;
    for (int j = i + 1; j < _clusters.size(); j++) {
      double M = _clusters[i]->getContainer()->dist(_clusters[j]->getContainer().get(), getGlobalSettings()->_primaryClusterMetric);
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

void SearchResultsContainer::saveClusterStats(function<double(SearchResultContainer*, SearchResultContainer*)> f, SearchMetadata md)
{
	setSessionName();
	string filename = getGlobalSettings()->_logRootDir + "/clusters/" + getGlobalSettings()->_sessionName;

	// save pairwise distances
	double maxDist;
	int x, y;
	map<int, map<int, double> > pairwiseDists = getPairwiseDist(f, maxDist, x, y);

	// write to file, we convert to csv here for viewing in excel or similar
	ofstream distFile(filename + "_dists.csv", ios::trunc);

	for (int i = 0; i < pairwiseDists.size(); i++) {
		for (int j = 0; j < pairwiseDists.size(); j++) {
			if (j > 0)
				distFile << ",";

			distFile << pairwiseDists[i][j];
		}
		distFile << "\n";
	}

	distFile.close();

	// write other stats
	ofstream statsFile(filename + "_stats.txt", ios::trunc);

	writeMetadata(statsFile, md);
	statsFile << "Data Set Diameter: " << maxDist << " (" << x << "," << y << ")\n";

	// calculate center diameters
	statsFile << "\nCluster Center Stats\n";
	statsFile << "ID\tElements\tDiameter\tAvg. dist\n";
	for (auto& c : _clusters) {
		c->calculateStats(pairwiseDists);
		statsFile << c->getClusterId() << "\t" << c->clusterSize() << "\t" << c->getDiameter() << "\t";

		// calculate average distance to cluster center.
		double avg = 0;
		for (auto& r : c->getAllChildElements()) {
			double dist = f(c->getContainer().get(), r.get());
			avg += dist;
			r->getSearchResult()->_extraData["Distance to Primary Center"] = String(dist);
		}
		avg /= c->clusterSize();
		statsFile << avg << "\n";
	}

	// individual element stats
	statsFile << "\nElement Stats\n";
	statsFile << "ID\tDist. to Center\n";
	for (auto& r : _allResults) {
		statsFile << r->getSearchResult()->_sampleNo << "\t" << r->getSearchResult()->_extraData["Distance to Primary Center"];
	}

	statsFile.close();
}

void SearchResultsContainer::saveClusterStats(function<double(SearchResultContainer*, SearchResultContainer*)> f,
	function<double(SearchResultContainer*, SearchResultContainer*)> f2, SearchMetadata md,
	Array<shared_ptr<TopLevelCluster>>& centers1, Array<shared_ptr<TopLevelCluster>>& centers2)
{
	setSessionName();
	string filename = getGlobalSettings()->_logRootDir + "/clusters/" + getGlobalSettings()->_sessionName;

	// save pairwise distances
	double maxDist;
	int x, y;
	map<int, map<int, double> > pairwiseDists = getPairwiseDist(f, maxDist, x, y);

	// pairwise dists for f2 
	double maxDist2;
	int x2, y2;
	map<int, map<int, double> > pairwiseDists2 = getPairwiseDist(f2, maxDist2, x2, y2);

	// pairwise unmasked distamces for pp avg lab
	double ppmaxDist;
	int ppx, ppy;
	distFuncType ppf = [](SearchResultContainer* x, SearchResultContainer* y) {
		return x->dist(y, PPAVGLAB, true, false);
	};
	map<int, map<int, double> > ppPairwiseDist = getPairwiseDist(ppf, ppmaxDist, ppx, ppy);

	// write to file, we convert to csv here for viewing in excel or similar
	ofstream distFile(filename + "_dists1.csv", ios::trunc);

	for (int i = 0; i < pairwiseDists.size(); i++) {
		for (int j = 0; j < pairwiseDists.size(); j++) {
			if (j > 0)
				distFile << ",";

			distFile << pairwiseDists[i][j];
		}
		distFile << "\n";
	}

	distFile.close();

	ofstream distFile2(filename + "_dists2.csv", ios::trunc);

	for (int i = 0; i < pairwiseDists2.size(); i++) {
		for (int j = 0; j < pairwiseDists2.size(); j++) {
			if (j > 0)
				distFile2 << ",";

			distFile2 << pairwiseDists2[i][j];
		}
		distFile2 << "\n";
	}

	distFile2.close();

	// write other stats
	ofstream statsFile(filename + "_stats.txt", ios::trunc);
	map<int, double> d1, d2, dpp, c1d, c2d, avgd1, avgd2, sdd1, sdd2;

	writeMetadata(statsFile, md);
	statsFile << "Data Set Diameter (primary axis): " << maxDist << " (" << x << "," << y << ")\n";
	statsFile << "Data Set Diameter (secondary axis): " << maxDist2 << " (" << x2 << "," << y2 << ")\n";
	// calculate center diameters
	statsFile << "\nCluster Center Stats\n";
	statsFile << "ID\tCount\tf1 Dia\t\tf2 Dia\t\tpp Dia\t\tf1 Avg\t\tf2 Avg\t\tpp Avg\t\tf1 var\t\tf2 var\t\tpp var\t\tf1 sd\t\tf2 sd\t\tpp sd\n";
	for (auto& c : _clusters) {
		statsFile << c->getClusterId() << "\t" << c->clusterSize() << "\t";

		// empty clusters are skipped
		if (c->clusterSize() == 0) {
			statsFile << "\n";
			continue;
		}

		// center stats are a bit involved here. we need 3 different distances
		// they are already pairwise computed
		c->calculateStats(pairwiseDists);
		double f1d = c->getDiameter();
		c->calculateStats(pairwiseDists2);
		double f2d = c->getDiameter();
		c->calculateStats(ppPairwiseDist);
		double ppd = c->getDiameter();

		// ok so here we have to actually compute the cluster centers cause they
		// are not computed in the pairwise grid mode
		auto container = c->getContainer();
		Eigen::VectorXd feats;
		feats.resize(_allResults[0]->getFeatures().size());
		feats.setZero();
		container->getSearchResult()->_scene.resize(_allResults[0]->getSearchResult()->_scene.size());
		container->getSearchResult()->_scene.setZero();
		for (auto& r : c->getAllChildElements()) {
			feats += r->getFeatures();
			container->getSearchResult()->_scene += r->getSearchResult()->_scene;
		}
		feats /= c->clusterSize();
		container->setFeatures(feats);
		container->getSearchResult()->_scene /= c->clusterSize();
		container->updateMask();

		// calculate average distance to cluster center.
		double avg1 = 0;
		double avg2 = 0;
		double ppavg = 0;
		for (auto& r : c->getAllChildElements()) {
			double dist1 = f(container.get(), r.get());
			double dist2 = f2(container.get(), r.get());
			double ppdist = ppf(container.get(), r.get());
			d1[r->getSearchResult()->_sampleNo] = dist1;
			d2[r->getSearchResult()->_sampleNo] = dist2;
			dpp[r->getSearchResult()->_sampleNo] = ppdist;
			avg1 += dist1;
			avg2 += dist2;
			ppavg += ppdist;
			r->getSearchResult()->_cluster = c->getClusterId();
		}
		avg1 /= c->clusterSize();
		avg2 /= c->clusterSize();
		avgd1[c->getClusterId()] = avg1;
		avgd2[c->getClusterId()] = avg2;
		ppavg /= c->clusterSize();

		// standard deviation
		double dev1 = 0;
		double dev2 = 0;
		double ppdev = 0;
		for (auto& r : c->getAllChildElements()) {
			int id = r->getSearchResult()->_sampleNo;
			dev1 += pow(d1[id] - avg1, 2);
			dev2 += pow(d2[id] - avg2, 2);
			ppdev += pow(dpp[id] - ppavg, 2);
		}
		dev1 /= c->clusterSize();
		dev2 /= c->clusterSize();
		sdd1[c->getClusterId()] = sqrt(dev1);
		sdd2[c->getClusterId()] = sqrt(dev2);
		ppdev /= c->clusterSize();

		char fmt[1000];
#ifndef WIN32
        snprintf(fmt, 990, "%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\n",
                  f1d, f2d, ppd, avg1, avg2, ppavg, dev1, dev2, ppdev, pow(dev1, 0.5), pow(dev2, 0.5), pow(ppdev, 0.5));
#else
		sprintf_s(fmt, 990, "%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\n",
			f1d, f2d, ppd, avg1, avg2, ppavg, dev1, dev2, ppdev, pow(dev1, 0.5), pow(dev2, 0.5), pow(ppdev, 0.5));
#endif
        statsFile << fmt;
	}

	// primary clustering stats
	statsFile << "\nPrimary Cluster Stats\n";
	statsFile << "ID\tCount\tDiam\tAvg.\tVar\tsd\n";
	map<int, double> c1avg, c1sd;
	for (auto& c : centers1) {
		c->calculateStats(pairwiseDists);
		statsFile << c->getClusterId() << "\t" << c->clusterSize() << "\t" << c->getDiameter() << "\t";
		c->getContainer()->updateMask();

		// calculate average distance to cluster center.
		double avg = 0;
		for (auto& r : c->getAllChildElements()) {
			double dist = f(c->getContainer().get(), r.get());
			c1d[r->getSearchResult()->_sampleNo] = dist;
			avg += dist;
		}
		avg /= c->clusterSize();
		c1avg[c->getClusterId()] = avg;
		statsFile << avg << "\t";

		// standard deviation
		double var = 0;
		for (auto& r : c->getAllChildElements()) {
			var += pow(avg - c1d[r->getSearchResult()->_sampleNo], 2);
		}
		var /= c->clusterSize();
		c1sd[c->getClusterId()] = sqrt(var);
		statsFile << var << "\t" << sqrt(var) << "\n";
	}

	// secondary clustering stats
	statsFile << "\nSecondary Cluster Stats\n";
	statsFile << "ID\tCount\tDiam\tAvg.\tVar\tsd\n";
	map<int, double> c2avg, c2sd;
	for (auto& c : centers2) {
		c->calculateStats(pairwiseDists);
		statsFile << c->getClusterId() << "\t" << c->clusterSize() << "\t" << c->getDiameter() << "\t";
		c->getContainer()->updateMask();

		// calculate average distance to cluster center.
		double avg = 0;
		for (auto& r : c->getAllChildElements()) {
			double dist = f2(c->getContainer().get(), r.get());
			c2d[r->getSearchResult()->_sampleNo] = dist;
			avg += dist;
		}
		avg /= c->clusterSize();
		c2avg[c->getClusterId()] = avg;
		statsFile << avg << "\t";

		// standard deviation
		double var = 0;
		for (auto& r : c->getAllChildElements()) {
			var += pow(avg - c2d[r->getSearchResult()->_sampleNo], 2);
		}
		var /= c->clusterSize();
		c2sd[c->getClusterId()] = sqrt(var);
		statsFile << var << "\t" << sqrt(var) << "\n";
	}

	// individual element stats
	statsFile << "\nElement Stats\n";
	statsFile << "ID\tPrimary\tSecond\t\tf1 to C\t\tf2 to C\t\tpp to C\t\tDist c1\t\tDist c2\t\tCf1 sig\t\tCf2 sig\t\tf1 sigm\t\tf2 sigm\n";
	for (auto& r : _allResults) {
		int id = r->getSearchResult()->_sampleNo;
		int cid = r->getSearchResult()->_cluster;
		int c1id = cid % centers1.size();
		int c2id = cid / centers1.size();

		// calculate how many sds element is from various things
		double csdf1 = abs(d1[id] - sdd1[cid]) / avgd1[cid];
		double csdf2 = abs(d2[id] - sdd2[cid]) / avgd2[cid];
		double c1sds = abs(c1d[id] - c1avg[c1id]) / c1sd[c1id];
		double c2sds = abs(c2d[id] - c2avg[c2id]) / c2sd[c2id];

		char fmt[1000];
#ifndef WIN32
        snprintf(fmt, 990, "%8i%8i%8i\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f",
                  id, c1id, c2id, d1[id], d2[id], dpp[id], c1d[id], c2d[id], csdf1, csdf2, c1sds, c2sds);
#else
		sprintf_s(fmt, 990, "%8i%8i%8i\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f\t%8.5f",
			id, c1id, c2id, d1[id], d2[id], dpp[id], c1d[id], c2d[id], csdf1, csdf2, c1sds, c2sds);
#endif
		statsFile << fmt;

		r->getSearchResult()->_extraData["Distance to Primary Center"] = String(d1[id]);
		r->getSearchResult()->_extraData["Distance to Secondary Center"] = String(d2[id]);
		r->getSearchResult()->_extraData["Primary Distance to Group Center"] = String(c1d[id]);
		r->getSearchResult()->_extraData["Secondary Distance to Group Center"] = String(c2d[id]);
		r->getSearchResult()->_extraData["Per-Pixel Distance to Group Center"] = String(ppf(r.get(), _clusters[cid]->getContainer().get()));

		if (c1sds >= 2.0 || c2sds >= 2.0) {
			// flag and move
			r->getSearchResult()->_extraData["Outlier"] = "True";

			// move element to different cluster based on per-pixel difference
			// but only move if the element is not the only thing in its cluster
			if (_clusters[cid]->numElements() > 1) {
				double min = FLT_MAX;
				int minId = 0;
				for (int i = 0; i < _allResults.size(); i++) {
					int otherId = _allResults[i]->getSearchResult()->_sampleNo;
					if (id == otherId)
						continue;

					if (ppPairwiseDist[id][otherId] < min) {
						minId = i;
						min = ppPairwiseDist[id][otherId];
					}
				}

				// remove from old cluster
				_clusters[cid]->removeFromCluster(r);
				r->getSearchResult()->_cluster = _allResults[minId]->getSearchResult()->_cluster;
				_clusters[r->getSearchResult()->_cluster]->addToCluster(r);
				statsFile << "\tMOVED TO " << r->getSearchResult()->_cluster;
			}
		}

		statsFile << "\n";
		r->regenToolTip();
	}

	// per-pixel standard deviation
	ppsd(filename);

	// reselect representative result
	for (auto& c : _clusters) {
		c->setRepresentativeResult();
	}

	statsFile.close();
}

void SearchResultsContainer::saveClustering()
{
	_savedResults.add(Array<shared_ptr<TopLevelCluster> >(_clusters));

	_savedMetadata.add(createSearchMetadata());
}

SearchMetadata SearchResultsContainer::createSearchMetadata()
{
	SearchMetadata md;
	md._mode = getGlobalSettings()->_clusterDisplay;
	md._primaryArea = getGlobalSettings()->_primaryFocusArea;
	md._primaryClusters = getGlobalSettings()->_numPrimaryClusters;
	md._primaryMethod = getGlobalSettings()->_primaryClusterMethod;
	md._primaryMetric = getGlobalSettings()->_primaryClusterMetric;
	md._primaryThreshold = getGlobalSettings()->_primaryDivisiveThreshold;
	md._secondaryArea = getGlobalSettings()->_secondaryFocusArea;
	md._secondaryClusters = getGlobalSettings()->_numSecondaryClusters;
	md._secondaryMethod = getGlobalSettings()->_secondaryClusterMethod;
	md._secondaryMetric = getGlobalSettings()->_secondaryClusterMetric;
	md._secondaryThreshold = getGlobalSettings()->_secondaryDivisiveThreshold;
	
	return md;
}

void SearchResultsContainer::loadClustering(int idx)
{
	if (idx < _savedResults.size()) {
		// remove cluster children
		for (auto& c : _clusters) {
			removeChildComponent(getIndexOfChildComponent(c.get()));
		}

		_clusters.clear();

		_clusters = _savedResults[idx];

		SearchMetadata md = _savedMetadata[idx];
		getGlobalSettings()->_clusterDisplay = md._mode;
		getGlobalSettings()->_primaryFocusArea = md._primaryArea;
		getGlobalSettings()->_numPrimaryClusters = md._primaryClusters;
		getGlobalSettings()->_primaryClusterMethod = md._primaryMethod;
		getGlobalSettings()->_primaryClusterMetric = md._primaryMetric;
		getGlobalSettings()->_primaryDivisiveThreshold = md._primaryThreshold;
		getGlobalSettings()->_secondaryFocusArea = md._secondaryArea;
		getGlobalSettings()->_numSecondaryClusters = md._secondaryClusters;
		getGlobalSettings()->_secondaryClusterMethod = md._secondaryMethod;
		getGlobalSettings()->_secondaryClusterMetric = md._secondaryMetric;
		getGlobalSettings()->_secondaryDivisiveThreshold = md._secondaryThreshold;

		for (auto& c : _clusters) {
			addAndMakeVisible(c.get());
		}

		updateSize(getLocalBounds().getHeight(), getLocalBounds().getWidth());
		resized();
		repaint();
		getApplicationCommandManager()->invokeDirectly(command::REFRESH_SETTINGS, true);
	}
}

int SearchResultsContainer::numSavedClusters()
{
	return _savedResults.size();
}

void SearchResultsContainer::clearClusters()
{
  lock_guard<mutex> lock(_resultsLock);

	// remove cluster children
	for (auto& c : _clusters) {
		removeChildComponent(getIndexOfChildComponent(c.get()));
	}

	_clusters.clear();
	_unclusteredResults->removeAllResults();

	// add things to unclustered again
	for (auto& r : _allResults) {
		_unclusteredResults->addResult(r);
	}

	_unclusteredResults->resized();
	updateSize(getLocalBounds().getHeight(), getLocalBounds().getWidth());
	resized();
	repaint();
}

shared_ptr<SearchResultContainer> SearchResultsContainer::getLastSample()
{
	lock_guard<mutex> lock(_resultsLock);

	if (_newResults.size() != 0) {
		return _newResults.getLast();
	}
	else {
		return _allResults.getLast();
	}
}

const Array<shared_ptr<SearchResultContainer>>& SearchResultsContainer::getAllResults()
{
  return _allResults;
}

shared_ptr<SearchResultContainer> SearchResultsContainer::getBestUnexploitedResult()
{
  // basically we just sort an all results proxy (so no changes are visible)
  // and pick the best one not already exploited
  Array<shared_ptr<SearchResultContainer> > allProxy(_allResults);

  // we also want to encourage variation, so it's possible all the
  // good results are clustered together with similar values
	default_random_engine gen(std::random_device{}());
	uniform_real_distribution<double> udist(0.0, _allResults.size() * 0.1);
  int target = (int)(udist(gen));

  {
    // lock
    lock_guard<mutex> lock(_resultsLock);
    
    allProxy.addArray(_newResults);
    DefaultSorter sorter;
    allProxy.sort(sorter);

    for (int i = 0; i < allProxy.size(); i++) {
      if (allProxy[i]->getSearchResult()->_extraData.count("Exploited") == 0) {
        if (target <= 0) {
          allProxy[i]->getSearchResult()->_extraData["Exploited"] = "true";
          return allProxy[i];
        }
        else {
          target--;
        }
      }
    }
  }

  return nullptr;
}

Array<shared_ptr<SearchResultContainer>> SearchResultsContainer::getKCenters(int k, DistanceMetric metric)
{
  // Copy the results array to have a local copy to cluster since
  // adjustments could happen to all results array
  Array<shared_ptr<SearchResultContainer> > all;
  {
    lock_guard<mutex> lock(_resultsLock);
    all.addArray(_allResults);
  }

  // cluster on top 50%
  DefaultSorter sorter;
  all.sort(sorter);
  all.removeRange(all.size() * 0.50, all.size());

  distFuncType f = [metric](SearchResultContainer* x, SearchResultContainer* y) {
    return x->dist(y, metric, false, false);
  };

  Array<shared_ptr<SearchResultContainer> > tmpSeenPts;
  {
    lock_guard<mutex> lock(_resultsLock);
    tmpSeenPts.addArray(_recentered);
  }

  // calculate stats for cost scaling and cone radius terms
  // calculate average pairwise distance
  double c = 4;
  double r = 0;
  int count = 0;

  for (int i = 0; i < tmpSeenPts.size(); i++) {
    for (int j = i + 1; j < tmpSeenPts.size(); j++) {
      r += tmpSeenPts[i]->dist(tmpSeenPts[j].get(), metric, false, false);
      count++;
    }
  }

  if (count != 0)
    r /= count;

  function<double(shared_ptr<SearchResultContainer>)> rep = [&tmpSeenPts, r, c, metric](shared_ptr<SearchResultContainer> s) {
    return repulsionTerm(s.get(), tmpSeenPts, c, r, metric);
  };

  // theoretically running this shouldn't affect the current clustering that much?
  // correction: using this form won't affect current clusters. when the actual clustering happens
  // the results are added as children to TopLevelContainers which makes the heirarchy get all
  // sorts of confused. ths version of clustering prevents that
  Array<shared_ptr<SearchResultContainer> > centers = Clustering::kmeansBestRepClustering(all, k, f, rep);

  {
    lock_guard<mutex> lock(_resultsLock);

    for (auto center : centers) {
      _recentered.addIfNotAlreadyThere(center);
    }
  }

  return centers;
}

void SearchResultsContainer::updateAllImages()
{
  _views.updateImages();
  repaint();
}

void SearchResultsContainer::writeMetadata(std::ofstream &statsFile, SearchMetadata &md)
{
	statsFile << "Cluster mode: " << md._mode << "\n";
	statsFile << "Primary Clusters: " << md._primaryClusters << "\n";
	statsFile << "Primary Area: " << md._primaryArea << "\n";
	statsFile << "Primary Distance Metric: " << md._primaryMetric << "\n";
	statsFile << "Primary Cluster Method: " << md._primaryMethod << "\n";
	statsFile << "Secondary Clusters: " << md._secondaryClusters << "\n";
	statsFile << "Secondary Area: " << md._secondaryArea << "\n";
	statsFile << "Secondary Distance Metric: " << md._secondaryMetric << "\n";
	statsFile << "Secondary Cluster Method: " << md._secondaryMethod << "\n";

}

map<int, map<int, double>> SearchResultsContainer::getPairwiseDist(function<double(SearchResultContainer*, SearchResultContainer*)> f,
	double & maxDist, int & x, int & y)
{
	maxDist = 0;
	map<int, map<int, double> > pairwiseDists;
	for (int i = 0; i < _allResults.size(); i++) {
		int xid = _allResults[i]->getSearchResult()->_sampleNo;
		pairwiseDists[xid][xid] = 0;
		for (int j = i + 1; j < _allResults.size(); j++) {
			int yid = _allResults[j]->getSearchResult()->_sampleNo;
			double dist = f(_allResults[i].get(), _allResults[j].get());
			pairwiseDists[xid][yid] = dist;
			pairwiseDists[yid][xid] = dist;
			if (dist > maxDist) {
				maxDist = dist;
				x = xid;
				y = yid;
			}
		}
	}

	return pairwiseDists;
}

void SearchResultsContainer::ppsd(String prefix)
{
	// computes the per-pixel standard deviation from the sample images
	// this is fairly compute intensive
	// works on a downscaled version of the image (64 x 64)

	Eigen::VectorXd avg;
	avg.resize(_allResults[0]->getFeatures().size());
	avg.setZero();
	for (auto& r : _allResults) {
		avg += r->getFeatures();
	}
	avg /= _allResults.size();

	// ok so we have the average, now time to compute distances
	Eigen::VectorXd var;
	var.resize(avg.size() / 6);
	var.setZero();

	double maxVar = 0;
	for (auto& r : _allResults) {
		Eigen::VectorXd feats = r->getFeatures();
		for (int i = 0; i < avg.size() / 6; i++) {
			Eigen::Vector3d avgPx(avg[i * 6], avg[i * 6 + 1], avg[i * 6 + 2]);
			Eigen::Vector3d fPx(feats[i * 6], feats[i * 6 + 1], feats[i * 6 + 2]);
			double v = (avgPx - fPx).squaredNorm();
			var[i] += v;
			if (v > maxVar) {
				maxVar = v;
			}
		}
	}

	Image varOut(Image::PixelFormat::RGB, 64, 64, true);
	Image sdOut(Image::PixelFormat::RGB, 64, 64, true);

	for (int i = 0; i < var.size(); i++) {
		int x = i % 64;
		int y = i / 64;
		uint8 varpx = (uint8) ((var[i] / maxVar) * 255);
		uint8 sdpx = (uint8) ((sqrt(var[i]) / sqrt(maxVar)) * 255);
		varOut.setPixelAt(x, y, Colour(varpx, varpx, varpx));
		sdOut.setPixelAt(x, y, Colour(sdpx, sdpx, sdpx));
	}

	File vimg;
	vimg = vimg.getCurrentWorkingDirectory().getChildFile(String(prefix + "_var.png"));
	File sdimg;
	sdimg = sdimg.getCurrentWorkingDirectory().getChildFile(String(prefix + "_sd.png"));

	FileOutputStream os(vimg);
	FileOutputStream os2(sdimg);
	PNGImageFormat pngif;
	pngif.writeImageToStream(varOut, os);
	pngif.writeImageToStream(sdOut, os2);
}