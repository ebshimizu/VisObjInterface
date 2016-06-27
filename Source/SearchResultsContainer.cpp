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

SearchResultsContainer::SearchResultsContainer()
{
  _sampleId = 0;
  _elemsPerRow = getGlobalSettings()->_clusterElemsPerRow;
  _columnSize = 250;

  _unclusteredResults = new SearchResultList();
  _unclusteredViewer = new Viewport();
  _unclusteredViewer->setViewedComponent(_unclusteredResults);
  addAndMakeVisible(_unclusteredViewer);
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
  auto viewPt = _unclusteredViewer->getViewPosition();
  _unclusteredResults->setWidth(_columnSize - _unclusteredViewer->getScrollBarThickness());
  _unclusteredViewer->setBounds(lbounds.removeFromLeft(_columnSize));
  _unclusteredViewer->setViewPosition(viewPt);

  // clusters
	if (getGlobalSettings()->_clusterDisplay == COLUMNS) {
		for (int i = 0; i < _clusters.size(); i++) {
			_clusters[i]->setBounds(lbounds.removeFromLeft(_columnSize));
		}
	}
	else if (getGlobalSettings()->_clusterDisplay == GRID) {
		for (int i = 0; i < getGlobalSettings()->_numSecondaryClusters; i++) {
			auto row = lbounds.removeFromTop(_columnSize * (9.0 / 16.0));
			for (int j = 0; j < getGlobalSettings()->_numPrimaryClusters; j++) {
				int idx = i * getGlobalSettings()->_numPrimaryClusters + j;
				if (idx < _clusters.size()) {
					_clusters[idx]->setBounds(row.removeFromLeft(_columnSize));
				}
			}
		}
	}
}

void SearchResultsContainer::updateSize(int height)
{
  // fixed sized, maximum visible area, width defined by number of clusters
	int width = 0;
	if (getGlobalSettings()->_clusterDisplay == COLUMNS) {
		width = (1 + _clusters.size()) * _columnSize;
	}
	else if (getGlobalSettings()->_clusterDisplay == GRID) {
		width = (1 + getGlobalSettings()->_numPrimaryClusters) * _columnSize;
		height = (1 + getGlobalSettings()->_numSecondaryClusters) * (_columnSize * (9.0 / 16.0)); // magic number but should be fine
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
}

void SearchResultsContainer::sort(AttributeSorter* s)
{
  _unclusteredResults->sort(s);

  // sort clusters
  for (auto& c : _clusters) {
    c->sort(s);
    c->resized();
  }

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

  updateSize(getLocalBounds().getHeight());
  resized();
  repaint();
  _unclusteredResults->repaint();
}

void SearchResultsContainer::clear()
{
  lock_guard<mutex> lock(_resultsLock);

  _allResults.clear();
  _newResults.clear();
  _unclusteredResults->removeAllResults();
  _clusters.clear();
	_savedResults.clear();

  updateSize(getLocalBounds().getHeight());
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

  updateSize(getLocalBounds().getHeight());
  resized();
  repaint();
}

void SearchResultsContainer::cluster()
{
  lock_guard<mutex> lock(_resultsLock);

	// remove cluster children
	for (auto& c : _clusters) {
		removeChildComponent(getIndexOfChildComponent(c.get()));
	}

  // clear out the clusters, we have everything saved in _allResults
  _clusters.clear();

  // clear out unclustered results
  _unclusteredResults->removeAllResults();

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
				e->setClusterDistance(f(e.get(), ce.get()));
			}
			for (auto& e : c->getChildElements()) {
				e->setClusterDistance(f(e.get(), ce.get()));
			}
			c->getRepresentativeResult()->setClusterDistance(f(c->getRepresentativeResult().get(), ce.get()));
		}

		// calculate cluster stats
		calculateClusterStats();
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

		// do the clustering again with secondary options
		switch (mode2) {
		case KMEANS:
			centers = Clustering::kmeansClustering(_allResults, getGlobalSettings()->_numSecondaryClusters, f2);
			break;
		case MEAN_SHIFT:
			centers = Clustering::meanShiftClustering(_allResults, getGlobalSettings()->_meanShiftBandwidth);
			break;
		case SPECTRAL:
			centers = Clustering::spectralClustering(_allResults, getGlobalSettings()->_numSecondaryClusters, f2);
			break;
		case DIVISIVE:
			centers = Clustering::divisiveKMeansClustering(_allResults, getGlobalSettings()->_numSecondaryClusters, f2);
			break;
		case TDIVISIVE:
			centers = Clustering::thresholdedKMeansClustering(_allResults, getGlobalSettings()->_secondaryDivisiveThreshold, f2);
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
		numRows = centers.size();
		
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
			r->setClusterDistance(NAN);
		}

		// make clusters visible and pick representative scenes
		for (auto& c : _clusters) {
			addAndMakeVisible(c.get());
			c->setRepresentativeResult();
		}
	}

  updateSize(getLocalBounds().getHeight());
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
  updateSize(getLocalBounds().getHeight());
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

  updateSize(getLocalBounds().getHeight());
  _unclusteredResults->resized();
  resized();
  repaint();
  getStatusBar()->setStatusMessage("Load complete.");
}

void SearchResultsContainer::setElemsPerRow(int epr)
{
  _elemsPerRow = epr;
  updateSize(getLocalBounds().getHeight());
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

void SearchResultsContainer::saveClustering()
{
	_savedResults.add(Array<shared_ptr<TopLevelCluster> >(_clusters));
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

		for (auto& c : _clusters) {
			addAndMakeVisible(c.get());
		}

		updateSize(getLocalBounds().getHeight());
		resized();
		repaint();
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
	updateSize(getLocalBounds().getHeight());
	resized();
	repaint();
}
