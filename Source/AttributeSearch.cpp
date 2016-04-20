/*
  ==============================================================================

    AttributeSearch.cpp
    Created: 6 Jan 2016 11:25:42am
    Author:  falindrith

  ==============================================================================
*/

#include "AttributeSearch.h"
#include "MeanShift.h"
#include <list>
#include <algorithm>
//#include <vld.h>

SearchResult::SearchResult() { }

SearchResult::SearchResult(const SearchResult & other) :
  _scene(other._scene), _editHistory(other._editHistory), _objFuncVal(other._objFuncVal)
{
}

SearchResult::~SearchResult() {
}


// Search functions
// ==============================================================================

list<SearchResult*> attributeSearch(map<string, AttributeControllerBase*>& active, int editDepth)
{
  // If there's no active attribute, just leave
  if (active.size() == 0)
    return list<SearchResult*>();

  string log = "Search attribute params: ";
  for (auto& kvp : active) {
    log = log + " {" + kvp.first + " : ";
    if (kvp.second->getStatus() == A_MORE)
      log = log + "MORE";
    if (kvp.second->getStatus() == A_LESS)
      log = log + "LESS";
    if (kvp.second->getStatus() == A_EQUAL)
      log = log + "SAME";
    if (kvp.second->getStatus() == A_EXPLORE)
      log = log + "EXPLORE";
    log = log + "}";
  }
  getRecorder()->log(ACTION, log);

  AttributeSearchThread* t = new AttributeSearchThread(active, editDepth);
  t->runThread();

  list<SearchResult*> scenes = t->getResults();
  delete t;

  return scenes;
}

vector<Eigen::VectorXd> clusterResults(list<SearchResult*> results, int c)
{
  if (results.size() == 0) {
    // return empty list if no results
    return vector<Eigen::VectorXd>();
  }

  // Special case for 1 requested cluster
  if (c <= 1) {
    // 1 cluster, center is average of all other elements
    Eigen::VectorXd avg;
    avg.resize((*results.begin())->_scene.size());
    avg.setZero();

    for (auto r : results) {
      avg += r->_scene;
      r->_cluster = 0;
    }
    avg /= (double)results.size();
    
    vector<Eigen::VectorXd> ret;
    ret.push_back(avg);
    return ret;
  }
  // another special case for if the number of scenes is less than the number of desired centers
  if (results.size() <= c) {
    vector<Eigen::VectorXd> ret;
    int i = 0;
    for (auto r : results) {
      r->_cluster = i;
      ret.push_back(r->_scene);
      i++;
    }
    return ret;
  }
  else {
    // kmeans setup
    dlib::kcentroid<kernelType> kkmeansKernel(kernelType(), 0.001);
    dlib::kkmeans<kernelType> k(kkmeansKernel);

    vector<sampleType> samples;
    for (auto result : results) {
      samples.push_back(dlib::mat(result->_scene));
    }
    vector<sampleType> centers;

    // Start at or number of clusters specified by centers
    int numCenters = (c < 0) ? 2 : c;

    // Make sure user gave us something reasonable for centers
    if (results.size() < numCenters)
      return vector<Eigen::VectorXd>();

    double msd = INFINITY;

    while (msd > getGlobalSettings()->_clusterDistThreshold) {
      if (numCenters == samples.size()) {
        vector<Eigen::VectorXd> clusterCenters;
        int i = 0;
        for (auto c : results) {
          clusterCenters.push_back(c->_scene);
          c->_cluster = i;
          i++;
        }
        return clusterCenters;
      }

      // clear vectors
      centers.clear();

      // use dlib to get the initial centers
      dlib::pick_initial_centers(numCenters, centers, samples, k.get_kernel());

      // Run kmeans
      dlib::find_clusters_using_kmeans(samples, centers);

      // assign results to clusters and compute distance
      double sumDist = 0;
      int i = 0;
      for (auto r : results) {
        unsigned long center = dlib::nearest_center(centers, samples[i]);
        r->_cluster = center;

        // get the center and compute the distance
        auto centroid = centers[center];
        sumDist += length(centroid - samples[i]);
        i++;
      }

      msd = sumDist / results.size();
      numCenters++;

      // Immediately return if user specified an exact number of clusters
      if (c > 0)
        break;
    }

    // convert centers to eigen representation
    vector<Eigen::VectorXd> clusterCenters;
    for (auto c : centers)
    {
      Eigen::VectorXd eCenter;
      eCenter.resize(c.nr());

      for (int i = 0; i < c.nr(); i++) {
        eCenter[i] = c(i);
      }

      clusterCenters.push_back(eCenter);
    }

    return clusterCenters;
  }
}

vector<Eigen::VectorXd> clusterResults(list<Eigen::VectorXd> results, int c) {
  // kmeans setup
  dlib::kcentroid<kernelType> kkmeansKernel(kernelType(), 0.001);
  dlib::kkmeans<kernelType> k(kkmeansKernel);

  vector<sampleType> samples;
  for (auto result : results) {
    samples.push_back(dlib::mat(result));
  }
  vector<sampleType> centers;

  // Start at or number of clusters specified by centers
  int numCenters = (c < 0) ? 2 : c;

  // Make sure user gave us something reasonable for centers
  if (results.size() < numCenters)
    return vector<Eigen::VectorXd>();

  double msd = INFINITY;

  while (msd > getGlobalSettings()->_clusterDistThreshold) {
    if (numCenters == samples.size()) {
      vector<Eigen::VectorXd> clusterCenters;
      int i = 0;
      for (auto c : results) {
        clusterCenters.push_back(c);
        i++;
      }
      return clusterCenters;
    }

    // clear vectors
    centers.clear();

    // use dlib to get the initial centers
    dlib::pick_initial_centers(numCenters, centers, samples, k.get_kernel());

    // Run kmeans
    dlib::find_clusters_using_kmeans(samples, centers);

    // assign results to clusters and compute distance
    double sumDist = 0;
    for (int i = 0; i < results.size(); i++) {
      unsigned long center = dlib::nearest_center(centers, samples[i]);

      // get the center and compute the distance
      auto centroid = centers[center];
      sumDist += length(centroid - samples[i]);
    }

    msd = sumDist / results.size();
    numCenters++;

    // Immediately return if user specified an exact number of clusters
    if (c > 0)
      break;
  }

  // convert centers to eigen representation
  vector<Eigen::VectorXd> clusterCenters;
  for (auto c : centers)
  {
    Eigen::VectorXd eCenter;
    eCenter.resize(c.nr());

    for (int i = 0; i < c.nr(); i++) {
      eCenter[i] = c(i);
    }

    clusterCenters.push_back(eCenter);
  }

  return clusterCenters;
}

void filterResults(list<mcmcSample>& results, double t)
{
  // starting at the first element
  for (auto it = results.begin(); it != results.end(); it++) {
    // See how close all other elements are
    for (auto it2 = results.begin(); it2 != results.end(); ) {
      if (it == it2) {
        it2++;
        continue;
      }

      double dist = (it->first - it2->first).norm();

      // delete element if it's too close
      if (dist < t) {
        it2 = results.erase(it2);
      }
      else {
        it2++;
      }
    }
  }
}

void filterResults(list<SearchResult*>& results, double t)
{
  // starting at the first element
  for (auto it = results.begin(); it != results.end(); it++) {
    // See how close all other elements are
    for (auto it2 = results.begin(); it2 != results.end(); ) {
      if (it == it2) {
        it2++;
        continue;
      }

      double dist = ((*it)->_scene - (*it2)->_scene).norm();

      // delete element if it's too close
      if (dist < t) {
        delete *it2;
        it2 = results.erase(it2);
      }
      else {
        it2++;
      }
    }
  }
}

void filterResults(list<Eigen::VectorXd>& results, double t)
{
  // starting at the first element
  for (auto it = results.begin(); it != results.end(); it++) {
    // See how close all other elements are
    for (auto it2 = results.begin(); it2 != results.end(); ) {
      if (it == it2) {
        it2++;
        continue;
      }

      double dist = (*it - *it2).norm();

      // delete element if it's too close
      if (dist < t) {
        it2 = results.erase(it2);
      }
      else {
        it2++;
      }
    }
  }
}

void filterWeightedResults(list<SearchResult*>& results, double t) {
  // similar to filterResults, but this time the result that has a lower attribute
  // value is discarded instead.

  // starting at the first element
  for (auto it = results.begin(); it != results.end(); ) {
    // See how close all other elements are
    bool erase = false;
    for (auto it2 = results.begin(); it2 != results.end(); ) {
      if (it == it2) {
        it2++;
        continue;
      }

      double dist = ((*it)->_scene - (*it2)->_scene).norm();

      // delete element with lower score if it's too close
      // remember objFuncVals have lower scores being better
      if (dist < t) {
        if ((*it)->_objFuncVal < (*it2)->_objFuncVal) {
          delete *it2;
          it2 = results.erase(it2);
        }
        else {
          delete *it;
          erase = true;
          break;
        }
      }
      else {
        it2++;
      }
    }

    if (erase) {
      it = results.erase(it);
    }
    else {
      it++;
    }
  }
}

list<SearchResult*> getClosestScenesToCenters(list<SearchResult*>& results, vector<Eigen::VectorXd>& centers)
{
  vector<multimap<double, SearchResult*> > res;
  for (int i = 0; i < centers.size(); i++)
    res.push_back(multimap<double, SearchResult*>());

  // Place scenes sorted by distance to center into their clusters
  for (auto r : results) {
    double dist = (r->_scene - centers[r->_cluster]).norm();
    res[r->_cluster].insert(pair<double, SearchResult*>(dist, r));
  }

  // put first element in to results vector, delete the rest
  list<SearchResult*> filteredResults;
  for (auto c : res) {
    bool first = true;
    for (auto kvp : c) {
      if (first) {
        filteredResults.push_back(kvp.second);
        first = false;
      }
      else {
        delete kvp.second;
      }
    }
  }

  return filteredResults;
}

Eigen::VectorXd snapshotToVector(Snapshot * s)
{
  // Param order: Intensity, polar, azimuth, R, G, B, Softness (penumbra angle)
  // Device order: Alphabetical
  auto& devices = s->getRigData();
  int numFeats = 7;
  Eigen::VectorXd features;
  features.resize(numFeats * devices.size());
  
  int idx = 0;

  // This is a std::map which is always sorted, and thus always traversed
  // in ascending sort order. We'll use this to construct and reconstruct
  // vectors reliably without knowing a lot of details about the lights
  for (const auto& d : devices) {
    int base = idx * numFeats;
    features[base] = d.second->getParam<LumiverseFloat>("intensity")->asPercent();
    
    if (d.second->paramExists("polar"))
      features[base + 1] = d.second->getParam<LumiverseOrientation>("polar")->asPercent();
    else
      features[base + 1] = 0;

    if (d.second->paramExists("azimuth"))
      features[base + 2] = d.second->getParam<LumiverseOrientation>("azimuth")->asPercent();
    else
      features[base + 2] = 0;

    features[base + 3] = d.second->getParam<LumiverseColor>("color")->getColorChannel("Red");
    features[base + 4] = d.second->getParam<LumiverseColor>("color")->getColorChannel("Green");
    features[base + 5] = d.second->getParam<LumiverseColor>("color")->getColorChannel("Blue");
    features[base + 6] = d.second->getParam<LumiverseFloat>("penumbraAngle")->asPercent();
    idx++;
  }

  return features;
}

Snapshot * vectorToSnapshot(Eigen::VectorXd v)
{
  Snapshot* s = new Snapshot(getRig(), nullptr);
  auto devices = s->getRigData();
  int numFeats = 7;

  int idx = 0;

  for (const auto& d : devices) {
    int base = idx * numFeats;
    d.second->getParam<LumiverseFloat>("intensity")->setValAsPercent(v[base]);

    if (d.second->paramExists("polar"))
      d.second->getParam<LumiverseOrientation>("polar")->setValAsPercent(v[base + 1]);
    if (d.second->paramExists("azimuth"))
      d.second->getParam<LumiverseOrientation>("azimuth")->setValAsPercent(v[base + 2]);

    d.second->getParam<LumiverseColor>("color")->setColorChannel("Red", v[base + 3]);
    d.second->getParam<LumiverseColor>("color")->setColorChannel("Green", v[base + 4]);
    d.second->getParam<LumiverseColor>("color")->setColorChannel("Blue", v[base + 5]);
    d.second->getParam<LumiverseFloat>("penumbraAngle")->setValAsPercent(v[base + 6]);
    idx++;
  }

  return s;
}

String vectorToString(Eigen::VectorXd v)
{
  String ret = "";

  bool first = true;
  for (int i = 0; i < v.size(); i++) {
    if (!first) {
      ret += ", ";
    }
    ret += String(v[i]);
    first = false;
  }

  return ret;
}

//=============================================================================

AttributeSearchThread::AttributeSearchThread(map<string, AttributeControllerBase*> active, int editDepth) :
  ThreadWithProgressWindow("Searching", true, true, 2000), _active(active), _editDepth(editDepth)
{
  Rig* rig = getRig();
  _original = new Snapshot(rig, nullptr);

  // flag for special casing searching for the same of a single attribute
  if (_active.size() == 1 && _active.begin()->second->getStatus() == A_EQUAL) {
    _singleSame = true;
  }
  else {
    _singleSame = false;
  }

  setProgress(-1);
}

AttributeSearchThread::~AttributeSearchThread()
{
  // don't delete anything, other objects need the results allocated here.
  // except for internally used variables only
  delete _original;
}

void AttributeSearchThread::run()
{
  // Clear the results set for a clean first run
  _results.clear();
  SearchResult* start = new SearchResult();
  start->_scene = snapshotToVector(_original);
  _results.push_back(start);

  // Check the active set and disable any exploration attributes
  // for the first run
  map<string, AttributeControllerBase*> original = _active;
  vector<string> toRemove;
  for (const auto& attr : _active) {
    if (attr.second->getStatus() == A_EXPLORE) {
      toRemove.push_back(attr.first);
    }
  }

  for (const auto& e : toRemove) {
    _active.erase(e);
  }

  runStandardSearch();

  // If we actually had attributes that were marked as explore, do the next step
  // otherwise we're actually done
  if (toRemove.size() > 0) {
    // restore full active set of attributes
    _active = original;
    map<string, AttributeConstraint> originalConstraints;
    
    // mark all non-explore attributes as same
    for (const auto& attr : _active) {
      if (attr.second->getStatus() != A_EXPLORE) {
        originalConstraints[attr.first] = attr.second->getStatus();
        attr.second->setStatus(A_EQUAL);
      }
    }

    // Explore again, using results from the search as starting points
    runExploreSearch();

    // restore defaults before exit (consistency with user buttons)
    for (const auto& attr : original) {
      attr.second->setStatus(originalConstraints[attr.first]);
    }
  }

  for (auto& r : _results) {
    getGlobalSettings()->_fxs.push_back(r->_objFuncVal);
    getGlobalSettings()->_as.push_back(0);
    getGlobalSettings()->_editNames.push_back("FINAL");
  }

  getGlobalSettings()->dumpDiagnosticData();
}

void AttributeSearchThread::threadComplete(bool userPressedCancel)
{
  if (userPressedCancel) {
    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
      "Exploratory Search",
      "Search canceled");
  }
}

void AttributeSearchThread::runStandardSearch()
{
  // edit generation
  generateEdits(false);

  // objective function for combined set of active attributes.
  attrObjFunc f = [this](Snapshot* s) {
    double sum = 0;
    for (const auto& kvp : _active) {
      if (kvp.second->getStatus() == A_LESS)
        sum += kvp.second->evaluateScene(s);
      else if (kvp.second->getStatus() == A_MORE)
        sum -= kvp.second->evaluateScene(s);
      else if (kvp.second->getStatus() == A_EQUAL)
        sum += pow(kvp.second->evaluateScene(s) - kvp.second->evaluateScene(_original), 2);
    }

    return sum;
  };

  if (_singleSame) {
    // TODO: Reimplement single same for arbitrary number of devices
    // Reassign starting scene is working with same for a single attribute
    //Snapshot* sStart = new Snapshot(*_original);
    //for (auto c : editConstraints[ALL]) {
    //  if (!isParamLocked(c, ALL, sStart, )) {
    //    double dx = numericDeriv(c, ALL, sStart, f);
    //    double x = getDeviceValue(c, sStart);
    //    setDeviceValue(c, ALL, x - dx * 0.5, sStart);
    //  }
    //}
    //start->_scene = snapshotToVector(sStart);
    //delete sStart;
  }

  // For non-semantic attributes
  if (_active.size() == 1) {
    if (_active.begin()->second->isNonSemantic()) {
      setProgress(-1);
      setStatusMessage("Generating Non-Semantic Scenes");
      list<Snapshot*> results = _active.begin()->second->nonSemanticSearch();
      _results.clear();

      for (auto s : results) {
        SearchResult* res = new SearchResult();
        res->_editHistory.add("Non-Semantic " + _active.begin()->first);
        res->_scene = snapshotToVector(s);
        res->_objFuncVal = -_active.begin()->second->evaluateScene(s);
        delete s;
        _results.push_back(res);
      }

      // eliminate near duplicate scenes
      filterResults(_results, 0.001);
      setProgress(1);
      return;
    }
  }

  // save the original attribute function valuee
  _fc = f(_original);

  for (int i = 0; i < _editDepth; i++) {
    setProgress((float)i / _editDepth);

    // actually get the results with the current set of edits
    list<SearchResult*> newResults = runSingleLevelSearch(_results, i, f);

    // delete old results
    for (auto r : _results)
    {
      delete r;
    }
    _results.clear();

    // DEBUG - export set of points and vals for visualization
    if (getGlobalSettings()->_exportTraces) {
      exportSearchResults(newResults, i);
    }

    // cluster and filter for final run
    if (i == _editDepth - 1) {
      getRecorder()->log(SYSTEM, "Number of initial results at end of level " + String(i).toStdString() + ": " + String(newResults.size()).toStdString());

      _results = filterSearchResults(newResults);
      
      // DEBUG - export set of points and vals for visualization after filter
      if (getGlobalSettings()->_exportTraces) {
        exportSearchResults(_results, i, "filtered", true);
      }

      getRecorder()->log(SYSTEM, "Number of results at end of level " + String(i).toStdString() + ": " + String(_results.size()).toStdString());
    }
    else {
      getRecorder()->log(SYSTEM, "Number of initial results at end of level " + String(i).toStdString() + ": " + String(newResults.size()).toStdString());

      // Here's what we want from filtering:
      // -Reasonable sized set of points that both
      // --reasonably span the current space of possible modes of results
      // --are also better than the starting position
      // We will attempt to use mean shift clustering for this
      _results = filterSearchResults(newResults);

      // DEBUG - export set of points and vals for visualization after filter
      if (getGlobalSettings()->_exportTraces) {
        exportSearchResults(_results, i, "filtered", true);
      }
      
      getRecorder()->log(SYSTEM, "Number of results at end of level " + String(i).toStdString() + ": " + String(_results.size()).toStdString());
    }

    for (const auto& r : _results) {
      getGlobalSettings()->_selectedSamples.push_back(r->_sampleNo);
    }

    if (threadShouldExit()) {
      // delete results before cancelling search
      for (auto r : _results)
      {
        delete r;
      }
      _results.clear();

      return;
    }
  }

  setProgress(1);
}

void AttributeSearchThread::runExploreSearch() {
  // The exploratory search is basically the same as the normal search except for a few
  // key differences:
  // - The evaluation function evaluates itself at every start scene, not just _original
  // - The acceptance criteria now accepts scenes within a certain tolerance (i.e. sufficiently similar)
  // - Edits generated are pulled from the attributes' set of exploratory edits
  // edit generation
  generateEdits(true);

  // here the original objective function value changes on each run in the search results

  for (int i = 0; i < _editDepth; i++) {
    setProgress((float)i / _editDepth);
    list<SearchResult*> newResults = runSingleLevelExploreSearch(_results, i);

    // delete old results
    for (auto r : _results)
    {
      delete r;
    }
    _results.clear();

    // DEBUG - export set of points and vals for visualization
    if (getGlobalSettings()->_exportTraces) {
      exportSearchResults(newResults, i);
    }

    // cluster and filter for final run
    if (i == _editDepth - 1) {
      getRecorder()->log(SYSTEM, "Number of initial results: " + String(newResults.size()).toStdString());

      double thresh = getGlobalSettings()->_jndThreshold;
      filterResults(newResults, thresh);
      _results = newResults;

      // additional filtering if too many results are still present
      while (_results.size() > getGlobalSettings()->_maxReturnedScenes) {
        thresh += getGlobalSettings()->_jndInc;
        filterResults(_results, thresh);
      }

      // DEBUG - export set of points and vals for visualization after filter
      if (getGlobalSettings()->_exportTraces) {
        exportSearchResults(_results, i, "filtered", true);
      }

      getRecorder()->log(SYSTEM, "JND Threshold at end of Search: " + String(thresh).toStdString());
    }
    else {
      double thresh = getGlobalSettings()->_jndThreshold;
      filterResults(newResults, thresh);
      _results = newResults;

      // additional filtering if too many results are still present
      while (_results.size() > getGlobalSettings()->_numEditScenes) {
        thresh += getGlobalSettings()->_jndInc;
        filterResults(_results, thresh);
      }

      // DEBUG - export set of points and vals for visualization after filter
      if (getGlobalSettings()->_exportTraces) {
        exportSearchResults(_results, i, "filtered", true);
      }

      //auto centers = clusterResults(newResults, getGlobalSettings()->_numEditScenes);
      //auto filtered = getClosestScenesToCenters(newResults, centers);
      //_results = filtered;
    }

    for (const auto& r : _results) {
      getGlobalSettings()->_selectedSamples.push_back(r->_sampleNo);
    }

    if (threadShouldExit()) {
      // delete results before cancelling search
      for (auto r : _results)
      {
        delete r;
      }
      _results.clear();

      return;
    }
  }

  setProgress(1);
}

void AttributeSearchThread::generateEdits(bool explore)
{
  // here we dynamically create all of the edits used by the search algorithm for all
  // levels of the search. This is the function to change if we want to change
  // how the search goes through the lighting space.
  _edits.clear();
  _lockedParams.clear();

  // in order for a parameter to be autolocked, it must be present in the autolock list
  // for all active attributes
  map<string, int> lockedParams;
  for (auto& a : _active) {
    for (auto& p : a.second->_autoLockParams) {
      if (lockedParams.count(p) == 0) {
        lockedParams[p] = 1;
      }
      else {
        lockedParams[p] += 1;
      }
    }
  }

  for (auto& p : lockedParams) {
    if (p.second == _active.size()) {
      _lockedParams.insert(p.first);
    }
  }

  // for exploratory search, we ask the attributes what to do and then return
  if (explore) {
    for (auto& a : _active) {
      if (a.second->getStatus() == A_EXPLORE) {
        auto edits = a.second->getExploreEdits();
        for (auto& e : edits) {
          _edits[e.first] = e.second;
        }
      }
    }

    return;
  }

  // The search assumes two things: the existence of a metadata field called 'system'
  // and the existence of a metadata field called 'area' on every device. It does not
  // care what you call the things in these fields, but it does care that they exist.
  // These two fields are the primitives for how the system sets up its lights.
  // A system is a group of lights that achieve the same logical effect (i.e. all fill lights).
  // An area is a common focal point for a set of lights. Lights from multiple systems can be
  // in the same area.
  // It is up to the user to decide how to split lights into groups and systems.
  set<string> systems = getRig()->getMetadataValues("system");
  set<string> areas = getRig()->getMetadataValues("area");

  // Create all devices edit types
  generateDefaultEdits("*");
  generateColorEdits("");

  // Create edits for each system
  for (const auto& s : systems) {
    generateDefaultEdits("$system=" + s);
  }

  // Create edits for each area
  for (const auto& a : areas) {
    generateDefaultEdits("$area=" + a);
    generateColorEdits(a);
  }

  // Create edits for each system within an area
  //for (const auto& s : systems) {
  //  for (const auto& a : areas) {
  //    generateDefaultEdits("$area=" + a + "[$system=" + s + "]");
  //  }
  //}

  // Special edit types
  // left blank for now.
  // may be used for user specified edits? May do cross-system/area edits?
}

void AttributeSearchThread::generateDefaultEdits(string select)
{
  vector<EditConstraint> allParams;
  // looks a bit arbitrary, but see definition of EditParam. Includes all params except RGB.
  for (int i = 0; i <= 6; i++) {
    allParams.push_back(EditConstraint(select, (EditParam)i, D_ALL));
  }
  _edits[select + "_all"] = allParams;

  if (_lockedParams.count("intensity") == 0) {
    // Intensity
    vector<EditConstraint> intens;
    intens.push_back(EditConstraint(select, INTENSITY, D_ALL));
    _edits[select + "_intens"] = intens;

    // Uniform intensity
    vector<EditConstraint> uintens;
    uintens.push_back(EditConstraint(select, INTENSITY, D_UNIFORM));
    _edits[select + "_uniformIntens"] = uintens;

    // Joint intensity
    vector<EditConstraint> jintens;
    jintens.push_back(EditConstraint(select, INTENSITY, D_JOINT));
    _edits[select + "_jointIntens"] = jintens;
  }

  if (_lockedParams.count("color") == 0) {
    // Hue
    vector<EditConstraint> hue;
    hue.push_back(EditConstraint(select, HUE, D_ALL));
    _edits[select + "_hue"] = hue;

    // Color
    vector<EditConstraint> color;
    color.push_back(EditConstraint(select, HUE, D_ALL));
    color.push_back(EditConstraint(select, SAT, D_ALL));
    color.push_back(EditConstraint(select, VALUE, D_ALL));
    _edits[select + "_color"] = color;

    // Joint Hue
    vector<EditConstraint> jhue;
    jhue.push_back(EditConstraint(select, HUE, D_JOINT));
    _edits[select + "_jointHue"] = jhue;

    // Uniform color
    vector<EditConstraint> ucolor;
    ucolor.push_back(EditConstraint(select, HUE, D_UNIFORM));
    ucolor.push_back(EditConstraint(select, SAT, D_UNIFORM));
    ucolor.push_back(EditConstraint(select, VALUE, D_UNIFORM));
    _edits[select + "_uniformColor"] = ucolor;
  }

  if (_lockedParams.count("polar") == 0 && _lockedParams.count("azimuth") == 0) {
    // Position
    vector<EditConstraint> position;
    position.push_back(EditConstraint(select, POLAR, D_ALL));
    position.push_back(EditConstraint(select, AZIMUTH, D_ALL));
    _edits[select + "_pos"] = position;
  }

  if (_lockedParams.count("penumbraAngle") == 0) {
    // Softness
    vector<EditConstraint> soft;
    soft.push_back(EditConstraint(select, SOFT, D_ALL));
    _edits[select + "_soft"] = soft;
  }
}

void AttributeSearchThread::generateColorEdits(string area)
{
  // instantly abort if color is locked
  if (_lockedParams.count("color"))
    return;

  set<string> systems = getRig()->getMetadataValues("system");
  vector<string> sys;
  for (const auto& s : systems)
    sys.push_back(s);

  string areaq = (area == "") ? "" : "$area=" + area;  

  // If 1 or 0 systems, return
  if (sys.size() < 2)
    return;

  // pairs
  for (int i = 0; i < sys.size(); i++) {
    for (int j = i + 1; j < sys.size(); j++) {
      vector<EditConstraint> comp;
      comp.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", HUE, D_COMPLEMENTARY_COLOR));
      comp.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", SAT, D_COMPLEMENTARY_COLOR));
      comp.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", VALUE, D_COMPLEMENTARY_COLOR));
      comp.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", HUE, D_COMPLEMENTARY_COLOR));
      comp.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", SAT, D_COMPLEMENTARY_COLOR));
      comp.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", VALUE, D_COMPLEMENTARY_COLOR));
      _edits[area + " - " + sys[i] + "+" + sys[j] + " Complementary Color"] = comp;
    }
  }

  if (sys.size() >= 3) {
    // triads
    for (int i = 0; i < sys.size(); i++) {
      for (int j = i + 1; j < sys.size(); j++) {
        for (int k = j + 1; k < sys.size(); k++) {
          vector<EditConstraint> comp;
          comp.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", HUE, D_TRIADIC_COLOR));
          comp.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", SAT, D_TRIADIC_COLOR));
          comp.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", VALUE, D_TRIADIC_COLOR));
          comp.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", HUE, D_TRIADIC_COLOR));
          comp.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", SAT, D_TRIADIC_COLOR));
          comp.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", VALUE, D_TRIADIC_COLOR));
          comp.push_back(EditConstraint(areaq + "[$system=" + sys[k] + "]", HUE, D_TRIADIC_COLOR));
          comp.push_back(EditConstraint(areaq + "[$system=" + sys[k] + "]", SAT, D_TRIADIC_COLOR));
          comp.push_back(EditConstraint(areaq + "[$system=" + sys[k] + "]", VALUE, D_TRIADIC_COLOR));
          _edits[area + " - " + sys[i] + "+" + sys[j] + "+" + sys[k] + " Triadic Color"] = comp;

          vector<EditConstraint> comp2;
          comp2.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", HUE, D_SPLIT_COMPLEMENTARY_COLOR));
          comp2.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", SAT, D_SPLIT_COMPLEMENTARY_COLOR));
          comp2.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", VALUE, D_SPLIT_COMPLEMENTARY_COLOR));
          comp2.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", HUE, D_SPLIT_COMPLEMENTARY_COLOR));
          comp2.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", SAT, D_SPLIT_COMPLEMENTARY_COLOR));
          comp2.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", VALUE, D_SPLIT_COMPLEMENTARY_COLOR));
          comp2.push_back(EditConstraint(areaq + "[$system=" + sys[k] + "]", HUE, D_SPLIT_COMPLEMENTARY_COLOR));
          comp2.push_back(EditConstraint(areaq + "[$system=" + sys[k] + "]", SAT, D_SPLIT_COMPLEMENTARY_COLOR));
          comp2.push_back(EditConstraint(areaq + "[$system=" + sys[k] + "]", VALUE, D_SPLIT_COMPLEMENTARY_COLOR));
          _edits[area + " - " + sys[i] + "+" + sys[j] + "+" + sys[k] + " Split Complementary Color"] = comp2;

          vector<EditConstraint> comp3;
          comp3.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", HUE, D_ANALOGOUS_COLOR));
          comp3.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", SAT, D_ANALOGOUS_COLOR));
          comp3.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", VALUE, D_ANALOGOUS_COLOR));
          comp3.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", HUE, D_ANALOGOUS_COLOR));
          comp3.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", SAT, D_ANALOGOUS_COLOR));
          comp3.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", VALUE, D_ANALOGOUS_COLOR));
          comp3.push_back(EditConstraint(areaq + "[$system=" + sys[k] + "]", HUE, D_ANALOGOUS_COLOR));
          comp3.push_back(EditConstraint(areaq + "[$system=" + sys[k] + "]", SAT, D_ANALOGOUS_COLOR));
          comp3.push_back(EditConstraint(areaq + "[$system=" + sys[k] + "]", VALUE, D_ANALOGOUS_COLOR));
          _edits[area + " - " + sys[i] + "+" + sys[j] + "+" + sys[k] + " Analogous Color"] = comp3;
        }
      }
    }
  }

  if (sys.size() >= 4) {
    // quartets
    for (int i = 0; i < sys.size(); i++) {
      for (int j = i + 1; j < sys.size(); j++) {
        for (int k = j + 1; k < sys.size(); k++) {
          for (int l = k + 1; l < sys.size(); l++) {
            vector<EditConstraint> comp;
            comp.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", HUE, D_TETRADIC_COLOR));
            comp.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", SAT, D_TETRADIC_COLOR));
            comp.push_back(EditConstraint(areaq + "[$system=" + sys[i] + "]", VALUE, D_TETRADIC_COLOR));
            comp.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", HUE, D_TETRADIC_COLOR));
            comp.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", SAT, D_TETRADIC_COLOR));
            comp.push_back(EditConstraint(areaq + "[$system=" + sys[j] + "]", VALUE, D_TETRADIC_COLOR));
            comp.push_back(EditConstraint(areaq + "[$system=" + sys[k] + "]", HUE, D_TETRADIC_COLOR));
            comp.push_back(EditConstraint(areaq + "[$system=" + sys[k] + "]", SAT, D_TETRADIC_COLOR));
            comp.push_back(EditConstraint(areaq + "[$system=" + sys[k] + "]", VALUE, D_TETRADIC_COLOR));
            comp.push_back(EditConstraint(areaq + "[$system=" + sys[l] + "]", HUE, D_TETRADIC_COLOR));
            comp.push_back(EditConstraint(areaq + "[$system=" + sys[l] + "]", SAT, D_TETRADIC_COLOR));
            comp.push_back(EditConstraint(areaq + "[$system=" + sys[l] + "]", VALUE, D_TETRADIC_COLOR));
            _edits[area + " - " + sys[i] + "+" + sys[j] + "+" + sys[k] + + "+" + sys[l] + " Tetradic Color"] = comp;
          }
        }
      }
    }
  }
}

list<SearchResult*> AttributeSearchThread::runSingleLevelSearch(list<SearchResult*> startScenes, int level, attrObjFunc f)
{
  list<SearchResult*> searchResults;

  float seqPct = ((float)level / _editDepth);

  int totalOps = startScenes.size() * _edits.size();
  int opCt = 0;

  int i = 0, j = 0;

  // For each scene in the initial set
  for (const auto& scene : startScenes) {
    // For each edit, get a list of scenes returned and just add it to the overall list.
    j = 0;

    // save diagnostic data
    Snapshot* d = vectorToSnapshot(scene->_scene);
    getGlobalSettings()->_fxs.push_back(f(d));
    getGlobalSettings()->_as.push_back(1);
    getGlobalSettings()->_editNames.push_back("START");
    delete d;

    for (const auto& edits : _edits) {
      opCt++;
      setStatusMessage("Level " + String(level) + "\nScene " + String(i+1) + "/" + String(startScenes.size()) + "\nRunning Edit " + String(j+1) + "/" + String(_edits.size()));
      list<mcmcSample> editScenes = performEdit(edits.second, vectorToSnapshot(scene->_scene), f, edits.first);
      
      if (threadShouldExit())
        return list<SearchResult*>();

      for (auto s : editScenes) {
        SearchResult* r = new SearchResult();
        r->_scene = s.first;
        r->_editHistory.addArray(scene->_editHistory);
        r->_editHistory.add(edits.first);
        r->_sampleNo = s.second;
        Snapshot* sn = vectorToSnapshot(s.first);
        r->_objFuncVal = f(sn);
        delete sn;

        // We evaluate the function value on demand and just save the function itself
        searchResults.push_back(r);
      }

      setProgress(seqPct + ((float)opCt/(float)totalOps)* (1.0/_editDepth));
      j++;
    }
    i++;
  }

  return searchResults;
}

list<SearchResult*> AttributeSearchThread::runSingleLevelExploreSearch(list<SearchResult*> startScenes, int level)
{
  list<SearchResult*> searchResults;

  float seqPct = ((float)level / _editDepth);

  int totalOps = startScenes.size() * _edits.size();
  int opCt = 0;

  int i = 0, j = 0;

  // For each scene in the initial set
  for (const auto& scene : startScenes) {
    // For the exploratory search, we pull the base scenes out and use them as the start point
    // also at this point all non-explore functions are guaranteed to be equal, so 
    // we just have this one case
    Snapshot* initScene = vectorToSnapshot(scene->_scene);
    attrObjFunc f = [this, initScene](Snapshot* s) {
      double sum = 0;
      for (const auto& kvp : _active) {
        if (kvp.second->getStatus() == A_EQUAL)
          sum += pow(kvp.second->evaluateScene(s) - kvp.second->evaluateScene(initScene), 2);
      }

      return sum;
    };

    // this should be 0
    _fc = f(initScene);

    // For each edit, get a list of scenes returned and just add it to the overall list.
    j = 0;
    for (const auto& edits : _edits) {
      opCt++;
      setStatusMessage("Level " + String(level) + "\nScene " + String(i + 1) + "/" + String(startScenes.size()) + "\nRunning Edit " + String(j + 1) + "/" + String(_edits.size()));
      list<mcmcSample> editScenes = performEdit(edits.second, vectorToSnapshot(scene->_scene), f, edits.first, false);

      if (threadShouldExit())
        return list<SearchResult*>();

      for (auto s : editScenes) {
        SearchResult* r = new SearchResult();
        r->_scene = s.first;
        r->_editHistory.addArray(scene->_editHistory);
        r->_editHistory.add(edits.first);
        r->_sampleNo = s.second;
        Snapshot* sn = vectorToSnapshot(s.first);
        r->_objFuncVal = f(sn);
        delete sn;

        // We evaluate the function value on demand and just save the function itself
        searchResults.push_back(r);
      }

      setProgress(seqPct + ((float)opCt / (float)totalOps)* (1.0 / _editDepth));
      j++;
    }
    delete initScene;
    i++;
  }

  return searchResults;
}

list<mcmcSample> AttributeSearchThread::performEdit(vector<EditConstraint> edit, Snapshot* orig, attrObjFunc f, string name, bool acceptStd) {
  // Determine accept parameters
  double targetAcceptRate = 0.5;  // +/- 5%
  double sigma = getGlobalSettings()->_accceptBandwidth;
  // limit number of tuning iterations
  //for (int i = 0; i < 20; i++) {
  //  auto res = doMCMC(t, orig, f, 100, sigma, false);
  //  double acceptRate = res.second / 100.0;
    
  //  if (abs(acceptRate - targetAcceptRate) <= 0.05) {
  //    break;
  //  }
    
  //  if (acceptRate > targetAcceptRate)
  //    sigma -= 0.005;
  //  if (acceptRate < targetAcceptRate)
  //    sigma += 0.01;

  //  if (sigma <= 0) {
      // if we can't solve the problem with sigma, for now we'll just set it to default and continue
  //    sigma = 0.05;
  //    break;
  // }
  //}

  auto res = doMCMC(edit, orig, f, getGlobalSettings()->_maxMCMCIters, sigma, true, name, acceptStd);
  delete orig;
  return res.first;
}

pair<list<mcmcSample>, int> AttributeSearchThread::doMCMC(vector<EditConstraint> edit, Snapshot * start,
  attrObjFunc f, int iters, double sigma, bool saveSamples, string name, bool acceptStd)
{
  // duplicate initial state
  Snapshot* s = new Snapshot(*start);

  // Set up return list
  list<mcmcSample> results;

  // RNG
  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
  default_random_engine gen(seed1);
  normal_distribution<double> gdist(0, getGlobalSettings()->_editStepSize);  // start with sdev 2
  uniform_real_distribution<double> udist(0.0, 1.0);

  // Constants
  double minEditDist = getGlobalSettings()->_minEditDist;
  int maxIters = iters;

  // Set up relevant feature vector
  // Length of feature vector determined by type and number of devices in the rig
  int vecSize = getVecLength(edit, start);
  Eigen::VectorXd x;
  x.resize(vecSize);

  // Parameter restrictions
  vector<bool> canEdit;
  unordered_map<string, bool> canEditDevice;  // For uniform and joint, says which devices can actually be edited.
  bool cantEditAll = true;
  EditNumDevices lastNumDevices; // For color schemes. Color schemes are assumed to all have the same quantity.

  // This map is now necessary to track which devices correspond to which parameters in the
  // vector used in the search space, and tells the system how to modify the parameter during
  // search
  map<int, DeviceInfo> deviceLookup;

  // map used in certain cases to cache device set queries.
  map<int, set<Device*> > queryCache;

  int i = 0;
  for (auto& c : edit) {
    // Add all device parameters individually to the list
    auto devices = getRig()->select(c._select).getDevices();

    if (c._qty == D_ALL) {
      for (auto d : devices) {
        x[i] = getDeviceValue(c, s, d->getId());
        deviceLookup[i] = DeviceInfo(c, d->getId());
        bool lock = isParamLocked(c, d->getId());
        canEdit.push_back(!lock);

        cantEditAll = cantEditAll & lock;
        i++;
      }
    }
    // Joint adds a delta to every light's parameter, while leaving the
    // starting value unaffected.
    else if (c._qty == D_JOINT) {
      x[i] = 0;
      deviceLookup[i] = DeviceInfo(c, string());
      queryCache[i] = devices;
      i++;

      // If all affected params are locked, we can't edit this.
      bool canEditOneJoint = false;
      // Check for locks
      for (auto d : devices) {
        bool lock = isParamLocked(c, d->getId());
        canEditDevice[d->getId()] = !lock;
        canEditOneJoint = canEditOneJoint | !lock;
      }
      canEdit.push_back(canEditOneJoint);
      cantEditAll = cantEditAll & !canEditOneJoint;
    }
    // Uniform sets every parameter to the same value
    // color schemes follow uniform feature vector rules
    else if (c._qty == D_UNIFORM || c._qty == D_ANALOGOUS_COLOR ||
             c._qty == D_COMPLEMENTARY_COLOR || c._qty == D_TRIADIC_COLOR ||
             c._qty == D_TETRADIC_COLOR || c._qty == D_SPLIT_COMPLEMENTARY_COLOR)
    {
      // start at average val
      double avg = 0;
      int count = 0;
      for (auto d : devices) {
        avg += getDeviceValue(c, s, d->getId());
        count++;
      }
      avg /= (double)count;
      x[i] = avg;
      deviceLookup[i] = DeviceInfo(c, string());
      queryCache[i] = devices;
      i++;

      // If all affected params are locked, we can't edit this.
      bool canEditOneJoint = false;
      // Check for locks
      for (auto d : devices) {
        bool lock = isParamLocked(c, d->getId());
        canEditDevice[d->getId()] = !lock;
        canEditOneJoint = canEditOneJoint | !lock;
      }
      canEdit.push_back(canEditOneJoint);
      cantEditAll = cantEditAll & !canEditOneJoint;

      lastNumDevices = c._qty;
    }
  }

  // if we can't actually edit any parameters at all just exit now
  if (cantEditAll) {
    delete s;
    return pair<list<mcmcSample>, int>(results, 0);
  }

  // iteration setup
  Snapshot* sp = new Snapshot(*start);
  double fx = f(s);
  double T = getGlobalSettings()->_T;

  // diagnostics
  int accepted = 0;

  for (int i = 0; i < maxIters; i++) {
    // generate candidate x'
    Eigen::VectorXd xp = x;

    // if we're doing a color scheme op, generate xp with a special function
    // implicitly here we assume an edit using a color scheme op has only color scheme ops
    // as part of it
    if (lastNumDevices == D_ANALOGOUS_COLOR ||
        lastNumDevices == D_COMPLEMENTARY_COLOR ||
        lastNumDevices == D_TRIADIC_COLOR ||
        lastNumDevices == D_TETRADIC_COLOR ||
        lastNumDevices == D_SPLIT_COMPLEMENTARY_COLOR) {
      getNewColorScheme(xp, lastNumDevices, gdist, gen);
    }

    // displace by gaussian dist
    for (int j = 0; j < xp.size(); j++) {
      if (canEdit[j]) {
        // make sure we're doing a non-color scheme edit before tweaking the feature vector
        if (lastNumDevices != D_ANALOGOUS_COLOR ||
            lastNumDevices != D_COMPLEMENTARY_COLOR ||
            lastNumDevices != D_TRIADIC_COLOR ||
            lastNumDevices != D_TETRADIC_COLOR ||
            lastNumDevices != D_SPLIT_COMPLEMENTARY_COLOR) {
          xp[j] += gdist(gen);
        }

        if (deviceLookup[j]._c._qty == D_JOINT) {
          // Joint adds the delta (xp[j]) to the start value to get the new value
          // for all devices affected by the joint param
          auto& devices = queryCache[j];
          for (auto& d : devices) {
            if (canEditDevice[d->getId()]) {
              double initVal = getDeviceValue(deviceLookup[j]._c, start, d->getId());
              setDeviceValue(DeviceInfo(deviceLookup[j]._c, d->getId()), xp[j] + initVal, sp);
            }
          }
        }
        else if (deviceLookup[j]._c._qty == D_UNIFORM ||
          deviceLookup[j]._c._qty == D_COMPLEMENTARY_COLOR ||
          deviceLookup[j]._c._qty == D_ANALOGOUS_COLOR ||
          deviceLookup[j]._c._qty == D_TRIADIC_COLOR ||
          deviceLookup[j]._c._qty == D_TETRADIC_COLOR ||
          deviceLookup[j]._c._qty == D_SPLIT_COMPLEMENTARY_COLOR)
        {
          // Uniform and color schemes take the same value and apply it to every light;
          auto& devices = queryCache[j];
          for (auto& d : devices) {
            if (canEditDevice[d->getId()]) {
              xp[j] = setDeviceValue(DeviceInfo(deviceLookup[j]._c, d->getId()), xp[j], sp);
            }
          }
        }
        else {
          // The next line acts as a physically based clamp function of sorts.
          // It updates the lighting scene and also returns the value of the parameter after the update.
          xp[j] = setDeviceValue(deviceLookup[j], xp[j], sp);
        }
      }
    }

    // check for acceptance
    double fxp = f(sp);
    double diff = abs(fxp - fx);
    double a = min(exp((1 / T) * (fx - fxp)), 1.0);

    // Standard acceptance mode, if better auto accept and add to list
    // if satisfies different enough criteria
    if (acceptStd) {
      //if (getGlobalSettings()->_randomMode || fxp < fx)
      //  a = 1;
      //else {
        // Rescale a based on normal distribution with a std dev decided on by
        // tuning (or in this case, compeltely arbitrarily for now)
      //  a = 1 - (0.5 * erfc(-diff / (sqrt(2) * sigma)) - 0.5 * erfc(-diff / (sqrt(2) * -sigma)));
      //}

      // accept if a >= 1 or with probability a
      if (a >= 1 || udist(gen) <= a) {
        unsigned int sampleId = getGlobalSettings()->getSampleID();
        if (_singleSame) {
          if (saveSamples && abs(fxp - _fc) < 2) {
            // save sample in list
            results.push_back(mcmcSample(snapshotToVector(sp), sampleId));
          }
        }
        else {
          if (saveSamples && fxp < _fc && abs(fxp - _fc) >= minEditDist) {
            // save sample in list
            results.push_back(mcmcSample(xp, sampleId));
          }
        }
        // update x
        x = xp;
        accepted++;
        fx = fxp;

        // diagnostics
        getGlobalSettings()->_fxs.push_back(fxp);
        getGlobalSettings()->_as.push_back(a);
        getGlobalSettings()->_editNames.push_back(name);
      }
    }
    // Bandwidth acceptance mode, autoaccept if new value within tolerance,
    // chance of accepting if out of tolerance range
    else {
      // redefine diff as difference from base scene, not current scene
      //diff = abs(_fc - fxp);
      //if (getGlobalSettings()->_randomMode || diff < getGlobalSettings()->_explorationTolerance)
      //  a = 1;
      //else {
        // Calculate how far we are from the boundary (diff guaranteed to be >= _explorationTolerance)
      //  double boundDiff = diff - getGlobalSettings()->_explorationTolerance;
        // Rescale a based on normal distribution with a std dev decided on by
        // tuning (or in this case, compeltely arbitrarily for now)
      //  a = 1 - (0.5 * erfc(-boundDiff / (sqrt(2) * sigma)) - 0.5 * erfc(-boundDiff / (sqrt(2) * -sigma)));
      //}

      // accept if a >= 1 or with probability a
      if (a >= 1 || udist(gen) <= a) {
        unsigned int sampleId = getGlobalSettings()->getSampleID();
        if (saveSamples && diff < getGlobalSettings()->_explorationTolerance) {
          // save sample in list if it's within tolerance
          results.push_back(mcmcSample(xp, sampleId));
        }

        // update x regardless of if it was saved or not
        x = xp;
        accepted++;
        fx = fxp;

        // diagnostics
        getGlobalSettings()->_fxs.push_back(fxp);
        getGlobalSettings()->_as.push_back(a);
        getGlobalSettings()->_editNames.push_back(name);
      }
    }
  }

  //if (saveSamples)
  //  getRecorder()->log(SYSTEM, "[Debug] " + name + " accepted " + String(((float)accepted / (float)maxIters) * 100).toStdString() + "% of proposals");

  // TEMP - only filter practical duplicates
  //getRecorder()->log(SYSTEM, "[Debug] " + name + " returned " + String(results.size()).toStdString() + " proposals");
  filterResults(results, 0.001);//getGlobalSettings()->_jndThreshold);
  //getRecorder()->log(SYSTEM, "[Debug] " + name + " after filter returned " + String(results.size()).toStdString() + " proposals");

  // Convert results to full vectors
  list<mcmcSample> fullResults;
  for (const auto& r : results) {
    // adjust s to match result
    for (int j = 0; j < r.first.size(); j++) {
      if (deviceLookup[j]._c._qty == D_JOINT) {
        // Joint adds the delta (xp[j]) to the start value to get the new value
        // for all devices affected by the joint param
        auto& devices = queryCache[j];
        for (auto& d : devices) {
          if (canEditDevice[d->getId()]) {
            double initVal = getDeviceValue(deviceLookup[j]._c, start, d->getId());
            setDeviceValue(DeviceInfo(deviceLookup[j]._c, d->getId()), r.first[j] + initVal, s);
          }
        }
      }
      else if (deviceLookup[j]._c._qty == D_UNIFORM) {
        // Uniform takes the same value and applies it to every light;
        auto& devices = queryCache[j];
        for (auto& d : devices) {
          if (canEditDevice[d->getId()]) {
            setDeviceValue(DeviceInfo(deviceLookup[j]._c, d->getId()), r.first[j], s);
          }
        }
      }
      else {
        // The next line acts as a physically based clamp function of sorts.
        // It updates the lighting scene and also returns the value of the parameter after the update.
        setDeviceValue(deviceLookup[j], r.first[j], s);
      }
    }

    fullResults.push_back(mcmcSample(snapshotToVector(s), r.second));
  }
  
  if (s != nullptr)
    delete s;
  if (s != sp && sp != nullptr)
    delete sp;

  return pair<list<mcmcSample>, int>(fullResults, accepted);
}

double AttributeSearchThread::numericDeriv(EditConstraint c, Snapshot* s, attrObjFunc f, string& id)
{
  // load the appropriate settings and get the proper device.
  double h = getGlobalSettings()->_searchDerivDelta;
  Device* d = s->getRigData()[id];
  double f1 = f(s);
  double f2;

  switch (c._param) {
  case INTENSITY:
  {
    if (isDeviceParamLocked(d->getId(), "intensity"))
      return 0;

    float o = d->getIntensity()->asPercent();
    if (o >= 1)
      h = -h;

    d->getIntensity()->setValAsPercent(o + h);
    f2 = f(s);
    d->getIntensity()->setValAsPercent(o);
    break;
  }
  case HUE:
  {
    if (isDeviceParamLocked(d->getId(), "color"))
      return 0;

    Eigen::Vector3d hsv = d->getColor()->getHSV();
    float huePct = hsv[0] / 360.0;

    // hue wraps around, derivative should be fine if hue is at max/min
    d->getColor()->setHSV((huePct + h) * 360.0, hsv[1], hsv[2]);
    f2 = f(s);
    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2]);

    break;
  }
  case SAT:
  {
    if (isDeviceParamLocked(d->getId(), "color"))
      return 0;

    Eigen::Vector3d hsv = d->getColor()->getHSV();
    if (hsv[1] >= 1)
      h = -h;

    d->getColor()->setHSV(hsv[0], hsv[1] + h, hsv[2]);
    f2 = f(s);
    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2]);
    break;
  }
  case VALUE:
  {
    if (isDeviceParamLocked(d->getId(), "color"))
      return 0;

    Eigen::Vector3d hsv = d->getColor()->getHSV();
    if (hsv[2] >= 1)
      h = -h;

    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2] + h);
    f2 = f(s);
    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2]);
    break;
  }
  case RED:
  {
    if (isDeviceParamLocked(d->getId(), "color"))
      return 0;

    double r = d->getColor()->getColorChannel("Red");
    if (r >= 1)
      h = -h;

    d->getColor()->setColorChannel("Red", r + h);
    f2 = f(s);
    d->getColor()->setColorChannel("Red", r);
    break;
  }
  case BLUE:
  {
    if (isDeviceParamLocked(d->getId(), "color"))
      return 0;

    double b = d->getColor()->getColorChannel("Blue");
    if (b >= 1)
      h = -h;

    d->getColor()->setColorChannel("Blue", b + h);
    f2 = f(s);
    d->getColor()->setColorChannel("Blue", b);
    break;
  }
  case GREEN:
  {
    if (isDeviceParamLocked(d->getId(), "color"))
      return 0;

    double g = d->getColor()->getColorChannel("Green");
    if (g >= 1)
      h = -h;

    d->getColor()->setColorChannel("Green", g + h);
    f2 = f(s);
    d->getColor()->setColorChannel("Green", g);
    break;
  }
  case POLAR:
  {
    if (isDeviceParamLocked(d->getId(), "polar"))
      return 0;

    LumiverseOrientation* val = (LumiverseOrientation*)d->getParam("polar");
    float p = val->asPercent();
    if (p >= 1)
      h = -h;

    val->setValAsPercent(p + h);
    f2 = f(s);
    val->setValAsPercent(p);

    break;
  }
  case AZIMUTH:
  {
    if (isDeviceParamLocked(d->getId(), "azimuth"))
      return 0;

    LumiverseOrientation* val = (LumiverseOrientation*)d->getParam("azimuth");
    float a = val->asPercent();
    if (a >= 1)
      h = -h;

    val->setValAsPercent(a + h);
    f2 = f(s);
    val->setValAsPercent(a);

    break;
  }
  default:
    break;
  }

  return (f2 - f1) / h;
}

double AttributeSearchThread::setDeviceValue(DeviceInfo& info, double val, Snapshot * s)
{
  Device* d = s->getRigData()[info._id];
  
  switch (info._c._param) {
  case INTENSITY:
  {
    d->getIntensity()->setValAsPercent(val);
    return d->getIntensity()->asPercent();
  }
  case HUE:
  {
    val *= 360;
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(val, hsv[1], hsv[2]);
    return d->getColor()->getHSV()[0] / 360.0;
  }
  case SAT:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(hsv[0], val, hsv[2]);
    return d->getColor()->getHSV()[1];
  }
  case VALUE:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(hsv[0], hsv[1], val);
    return d->getColor()->getHSV()[2];
  }
  case RED:
    d->getColor()->setColorChannel("Red", val);
    return d->getColor()->getColorChannel("Red");
  case BLUE:
    d->getColor()->setColorChannel("Blue", val);
    return d->getColor()->getColorChannel("Blue");
  case GREEN:
    d->getColor()->setColorChannel("Green", val);
    return d->getColor()->getColorChannel("Green");
  case POLAR:
  {
    if (d->paramExists("polar")) {
      LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("polar");
      o->setValAsPercent(val);
      return o->asPercent();
    }
    return 0;
  }
  case AZIMUTH:
  {
    if (d->paramExists("azimuth")) {
      LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("azimuth");
      o->setValAsPercent(val);

      return o->asPercent();
    }
    return 0;
  }
  case SOFT:
  {
    LumiverseFloat* s = d->getParam<LumiverseFloat>("penumbraAngle");
    s->setValAsPercent(val);
    return s->asPercent();
  }
  default:
    return 0;
  }

}

double AttributeSearchThread::getDeviceValue(EditConstraint c, Snapshot * s, string& id)
{
  Device* d = s->getRigData()[id];

  switch (c._param) {
  case INTENSITY:
    return d->getIntensity()->asPercent();
  case HUE:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    return hsv[0] / 360.0;
  }
  case SAT:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    return hsv[1];
  }
  case VALUE:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    return hsv[2];
  }
  case RED:
    return d->getColor()->getColorChannel("Red");
  case BLUE:
    return d->getColor()->getColorChannel("Blue");
  case GREEN:
    return d->getColor()->getColorChannel("Green");
  case POLAR:
  {
    if (d->paramExists("polar")) {
      LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("polar");
      return o->asPercent();
    }
    return 0;
  }
  case AZIMUTH:
  {
    if (d->paramExists("azimuth")) {
      LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("azimuth");
      return o->asPercent();
    }
    return 0;
  }
  case SOFT:
  {
    LumiverseFloat* s = d->getParam<LumiverseFloat>("penumbraAngle");
    return s->asPercent();
  }
  default:
    return 0;
  }
}

bool AttributeSearchThread::isParamLocked(EditConstraint c, string& id)
{
  switch (c._param) {
  case INTENSITY:
    return isDeviceParamLocked(id, "intensity") || _lockedParams.count("intensity") > 0;
  case HUE:
    return isDeviceParamLocked(id, "color") || _lockedParams.count("color") > 0;
  case SAT:
    return isDeviceParamLocked(id, "color") || _lockedParams.count("color") > 0;
  case VALUE:
    return isDeviceParamLocked(id, "color") || _lockedParams.count("color") > 0;
  case RED:
    return isDeviceParamLocked(id, "color") || _lockedParams.count("color") > 0;
  case GREEN:
    return isDeviceParamLocked(id, "color") || _lockedParams.count("color") > 0;
  case BLUE:
    return isDeviceParamLocked(id, "color") || _lockedParams.count("color") > 0;
  case POLAR:
    return isDeviceParamLocked(id, "polar") || _lockedParams.count("polar") > 0;
  case AZIMUTH:
    return isDeviceParamLocked(id, "azimuth") || _lockedParams.count("azimuth") > 0;
  case SOFT:
    return isDeviceParamLocked(id, "penumbraAngle") || _lockedParams.count("penumbraAngle") > 0;
  default:
    return false;
  }
}

int AttributeSearchThread::getVecLength(vector<EditConstraint>& edit, Snapshot * s)
{
  int size = 0;
  for (auto& c : edit) {
    if (c._qty == D_ALL) {
      size += getRig()->select(c._select).getDevices().size();
    }
    else if (c._qty == D_JOINT || c._qty == D_UNIFORM) {
      size += 1;
    }
  }

  return size;
}

list<SearchResult*> AttributeSearchThread::filterSearchResults(list<SearchResult*>& results) {
  double eps = getGlobalSettings()->_jndThreshold;
  do {
    filterWeightedResults(results, eps);
    eps += 0.01;
  } while (results.size() > getGlobalSettings()->_maxReturnedScenes);

  return results;
}

//list<SearchResult*> AttributeSearchThread::filterSearchResults(list<SearchResult*>& results)
//{
//  MeanShift shifter;
//
//  // eliminate near duplicates
//  filterResults(results, 1e-3);
//
//  // put scenes into list, and weights into vector
//  list<Eigen::VectorXd> pt;
//  vector<double> weights;
//  for (auto& s : results) {
//    pt.push_back(s->_scene);
//    weights.push_back(-s->_objFuncVal);
//  }
//
//  // get the centers
//  list<Eigen::VectorXd> centers = shifter.cluster(pt, getGlobalSettings()->_meanShiftBandwidth, weights);
//  filterResults(centers, 1e-3);
//
//  // place centers into a vector (for indexing)
//  vector<Eigen::VectorXd> cs;
//  for (auto& c : centers) {
//    cs.push_back(c);
//  }
//
//  // place search results into clusters
//  clusterResults(results, cs);
//
//  // resturn the closest results to the center of the clusters
//  return getClosestScenesToCenters(results, cs);
//}

void AttributeSearchThread::clusterResults(list<SearchResult*>& results, vector<Eigen::VectorXd>& centers)
{
  for (auto& r : results) {
    // calculate distance to centers, save closest
    double lowest = 1e10;
    for (int i = 0; i < centers.size(); i++) {
      double dist = (r->_scene - centers[i]).norm();
      if (dist < lowest) {
        r->_cluster = i;
        lowest = dist;
      }
    }
  }
}

void AttributeSearchThread::getNewColorScheme(Eigen::VectorXd & base, EditNumDevices type, normal_distribution<double>& dist, default_random_engine& rng)
{
  uniform_real_distribution<double> udist(0.0, 1.0);

  // colors are arranged as [hsv]
  if (type == D_COMPLEMENTARY_COLOR) {
    // pick random element to be key 
    int key = (udist(rng) < 0.5) ? 0 : 1;
    int notKey = (key == 0) ? 1 : 0;

    // Modify a color, adjust other
    base[key * 3] += dist(rng);
    base[notKey * 3] += fmodf(base[key * 3] + (180 + dist(rng)), 360);

    // adjust hsv params for some variety
    base[key * 3 + 1] += dist(rng) * 0.5;
    base[key * 3 + 2] += dist(rng) * 0.5;
    base[notKey * 3 + 1] += dist(rng) * 0.5;
    base[notKey * 3 + 2] += dist(rng) * 0.5;
  }
  else if (type == D_TRIADIC_COLOR) {
    vector<int> idx;
    idx.push_back(0);
    idx.push_back(1);
    idx.push_back(2);

    random_shuffle(idx.begin(), idx.end());

    base[idx[0] * 3] += dist(rng);
    base[idx[1] * 3] = fmodf(base[idx[0] * 3] + 120 + dist(rng), 360);
    base[idx[2] * 3] = fmodf(base[idx[0] * 3] - 120 + dist(rng), 360);

    // adjust hsv for some variety, if out of bounds gets clamped anyway
    for (int i : idx) {
      base[i * 3 + 1] += dist(rng) * 0.5;
      base[i * 3 + 2] += dist(rng) * 0.5;
    }
  }
  else if (type == D_SPLIT_COMPLEMENTARY_COLOR) {
    vector<int> idx;
    idx.push_back(0);
    idx.push_back(1);
    idx.push_back(2);

    random_shuffle(idx.begin(), idx.end());

    // pick complementary hue and adjust from there
    double keyHue = fmodf(base[idx[0] * 3] + dist(rng), 360);
    base[idx[0] * 3] = keyHue;

    double compBaseHue = fmodf(keyHue + 180, 360);
    base[idx[1] * 3] = fmodf(keyHue + 30, 360);
    base[idx[2] * 3] = fmodf(keyHue - 30, 360);

    // adjust hsv for some variety, if out of bounds gets clamped anyway
    for (int i : idx) {
      base[i * 3 + 1] += dist(rng) * 0.5;
      base[i * 3 + 2] += dist(rng) * 0.5;
    }
  }
  else if (type == D_ANALOGOUS_COLOR) {
    vector<int> idx;
    idx.push_back(0);
    idx.push_back(1);
    idx.push_back(2);

    random_shuffle(idx.begin(), idx.end());

    // pick base analogous hue and adjust from there
    double keyHue = fmodf(base[idx[0] * 3] + dist(rng), 360);
    base[idx[0] * 3] = keyHue;

    float adist = dist(rng) * 20;
    base[idx[1] * 3] = fmodf(keyHue + adist, 360);
    base[idx[2] * 3] = fmodf(keyHue - adist, 360);

    // adjust hsv for some variety, if out of bounds gets clamped anyway
    for (int i : idx) {
      base[i * 3 + 1] += dist(rng) * 0.5;
      base[i * 3 + 2] += dist(rng) * 0.5;
    }
  }
  else if (type == D_TETRADIC_COLOR) {
    // square tetradic colors over here
    vector<int> idx;
    idx.push_back(0);
    idx.push_back(1);
    idx.push_back(2);
    idx.push_back(3);

    random_shuffle(idx.begin(), idx.end());

    // pick base tetradic hue and adjust from there
    double keyHue = fmodf(base[idx[0] * 3] + dist(rng), 360);
    base[idx[0] * 3] = keyHue;

    base[idx[1] * 3] = fmodf(keyHue + 90 + dist(rng) * 0.5, 360);
    base[idx[2] * 3] = fmodf(keyHue + 180 + dist(rng) * 0.5, 360);
    base[idx[3] * 3] = fmodf(keyHue + 270 + dist(rng) * 0.5, 360);

    // adjust hsv for some variety, if out of bounds gets clamped anyway
    for (int i : idx) {
      base[i * 3 + 1] += dist(rng) * 0.5;
      base[i * 3 + 2] += dist(rng) * 0.5;
    }
  }
}
