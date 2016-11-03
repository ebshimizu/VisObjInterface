/*
  ==============================================================================

    AttributeSearch.cpp
    Created: 6 Jan 2016 11:25:42am
    Author:  falindrith

  ==============================================================================
*/

#include "AttributeSearch.h"
#include "MeanShift.h"
#include "MainComponent.h"
#include "AttributeStyles.h"
#include "CMAES/src/cmaes_interface.h"
#include <list>
#include <algorithm>

// Search functions
// ==============================================================================
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

    if (d.second->paramExists("color")) {
      features[base + 3] = d.second->getParam<LumiverseColor>("color")->getColorChannel("Red");
      features[base + 4] = d.second->getParam<LumiverseColor>("color")->getColorChannel("Green");
      features[base + 5] = d.second->getParam<LumiverseColor>("color")->getColorChannel("Blue");
    }

    if (d.second->paramExists("penumbraAngle"))
      features[base + 6] = d.second->getParam<LumiverseFloat>("penumbraAngle")->asPercent();
    else
      features[base + 6] = 0;

    idx++;
  }

  return features;
}

Snapshot * vectorToSnapshot(Eigen::VectorXd v)
{
  Snapshot* s = new Snapshot(getRig());
	vectorToExistingSnapshot(v, *s);

  return s;
}

void vectorToExistingSnapshot(Eigen::VectorXd source, Snapshot& dest)
{
	auto devices = dest.getRigData();
	int numFeats = 7;

	int idx = 0;

	for (const auto& d : devices) {
		int base = idx * numFeats;
		d.second->getParam<LumiverseFloat>("intensity")->setValAsPercent((float) source[base]);

		if (d.second->paramExists("polar"))
			d.second->getParam<LumiverseOrientation>("polar")->setValAsPercent((float) source[base + 1]);
		if (d.second->paramExists("azimuth"))
			d.second->getParam<LumiverseOrientation>("azimuth")->setValAsPercent((float) source[base + 2]);

    if (d.second->paramExists("color")) {
      d.second->getParam<LumiverseColor>("color")->setColorChannel("Red", source[base + 3]);
      d.second->getParam<LumiverseColor>("color")->setColorChannel("Green", source[base + 4]);
      d.second->getParam<LumiverseColor>("color")->setColorChannel("Blue", source[base + 5]);
    }

		if (d.second->paramExists("penumbraAngle"))
			d.second->getParam<LumiverseFloat>("penumbraAngle")->setValAsPercent((float) source[base + 6]);

		idx++;
	}
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

float EditStats::variance() {
  double mean = meanVals();

  Eigen::VectorXf vals;
  vals.resize(_vals.size());
  for (int i = 0; i < _vals.size(); i++) {
    vals[i] = _vals[i];
  }

  Eigen::VectorXf shifted = (vals - mean * Eigen::VectorXf::Ones(_vals.size()));
  shifted = shifted.cwiseProduct(shifted);
  return shifted.sum() / _vals.size();
}

float EditStats::meanVals()
{
  float sum = 0;
  for (float v : _vals) {
    sum += v;
  }

  return sum / _vals.size();
}

float EditStats::meanDiffs()
{
  float sum = 0;
  for (float v : _diffs) {
    sum += v;
  }

  return sum / _diffs.size();
}

float EditStats::meanAccept()
{
  float sum = 0;
  for (float a : _as) {
    sum += a;
  }

  return sum / _as.size();
}

float EditStats::expectedDiff()
{
  if (_diffs.size() == 0)
    return 0;

  // we only want the negative differences (minimization), take the average, then mult by
  // proportion of diffs that were actually negative
  float sum = 0;
  int count = 0;
  for (float d : _diffs) {
    if (d < 0) {
      sum += d;
      count++;
    }
  }

  return sum / _diffs.size();
}

//=============================================================================

AttributeSearchThread::AttributeSearchThread(String name, SearchResultsViewer* viewer, map<string, int>& sharedData) :
	Thread(name), _viewer(viewer), _edits(vector<Edit*>()), _sharedData(sharedData)
{
  _original = nullptr;
  _fallback = nullptr;
  _samplesTaken = 0;

  _gen = default_random_engine(std::random_device{}());
  _udist = uniform_real_distribution<double>(0.0, 1.0);
  _styleDist = uniform_int_distribution<int>(0, 3);
}

AttributeSearchThread::~AttributeSearchThread()
{
  // don't delete anything, other objects need the results allocated here.
  // except for internally used variables only
  delete _original;
  delete _fallback;

  //for (auto e : _edits)
  //  delete e;
}

void AttributeSearchThread::setState(Snapshot * start, attrObjFunc & f, attrObjFunc& fsq, SearchMode m, Image freeze)
{
  if (_original != nullptr)
    delete _original;
  if (_fallback != nullptr)
    delete _fallback;

  _original = new Snapshot(*start);
  _edits.clear();
  _edits = getGlobalSettings()->_edits;
  _T = getGlobalSettings()->_T;
  _maxDepth = getGlobalSettings()->_startChainLength;
  _failures = 0;
  _resampleTime = getGlobalSettings()->_resampleTime;
  _resampleThreads = getGlobalSettings()->_resampleThreads;
  _currentStyle = NO_STYLE;

  _f = f;
  _fsq = fsq;
	_mode = m;
  _editMode = getGlobalSettings()->_editSelectMode;
	_status = IDLE;
  _fallback = new Snapshot(*start);
  _samplesTaken = 0;
  _freezeMask = freeze;
  _useMask = false;
  _maskTolerance = getGlobalSettings()->_maskTolerance;
  _useStyles = getGlobalSettings()->_useSearchStyles;
  _k = getGlobalSettings()->_searchFrontierSize;
  _frontier.clear();
  _previousResultsSize = 0;
  _distMetric = getGlobalSettings()->_searchDistMetric;
  _dispMetric = getGlobalSettings()->_searchDispMetric;

  _coneK = getGlobalSettings()->_repulsionConeK;
  _costScale = getGlobalSettings()->_repulsionCostK;
  _numPairs = getGlobalSettings()->_numPairs;
  _coneRadius = 0;

  // quick check to see if the mask is actually filled in
  for (int y = 0; y < _freezeMask.getHeight(); y++) {
    for (int x = 0; x < _freezeMask.getWidth(); x++) {
      _useMask |= (_freezeMask.getPixelAt(x, y).getBrightness() > 0);
    }
  }

  _statusMessage = "Initialized for new search. Mode: " + String(_mode);
}

void AttributeSearchThread::setState(Snapshot * start, vector<pair<GibbsScheduleConstraint, GibbsSchedule*>> sampler)
{
  if (_original != nullptr)
    delete _original;
  if (_fallback != nullptr)
    delete _fallback;

  _original = new Snapshot(*start);
  _edits.clear();
  _edits = getGlobalSettings()->_edits;
  _T = getGlobalSettings()->_T;
  _maxDepth = getGlobalSettings()->_startChainLength;
  _failures = 0;
  _resampleTime = getGlobalSettings()->_resampleTime;
  _resampleThreads = getGlobalSettings()->_resampleThreads;
  _currentStyle = NO_STYLE;

	_mode = getGlobalSettings()->_searchMode;
  _editMode = getGlobalSettings()->_editSelectMode;
	_status = IDLE;
  _fallback = new Snapshot(*start);
  _samplesTaken = 0;
  _useMask = false;
  _maskTolerance = getGlobalSettings()->_maskTolerance;
  _useStyles = getGlobalSettings()->_useSearchStyles;
  _k = getGlobalSettings()->_searchFrontierSize;
  _frontier.clear();
  _previousResultsSize = 0;
  _distMetric = getGlobalSettings()->_searchDistMetric;
  _dispMetric = getGlobalSettings()->_searchDispMetric;
  _activeSchedule = sampler;

  _coneK = getGlobalSettings()->_repulsionConeK;
  _costScale = getGlobalSettings()->_repulsionCostK;
  _numPairs = getGlobalSettings()->_numPairs;
  _coneRadius = 0;

  // quick check to see if the mask is actually filled in
  for (int y = 0; y < _freezeMask.getHeight(); y++) {
    for (int x = 0; x < _freezeMask.getWidth(); x++) {
      _useMask |= (_freezeMask.getPixelAt(x, y).getBrightness() > 0);
    }
  }

  _statusMessage = "Initialized for new search. Mode: " + String(_mode);
}

void AttributeSearchThread::run()
{
  initEditWeights();

  if (_mode == RANDOM_START || _mode == KRANDOM_START || _mode == REPULSION_KMCMC) {
    if (_id != 0) {
      randomizeStart();
    }
  }

  if (_mode == KRANDOM_START || _mode == KMCMC || _mode == REPULSION_KMCMC) {
    _frontier.add(shared_ptr<Snapshot>(new Snapshot(*_original)));
  }

  // here we basically want to run the search indefinitely until cancelled by user
  // so we just call the actual search function over and over
  while (1) {    
    runSearch();

    if (threadShouldExit())
      return;
  }
}

void AttributeSearchThread::computeEditWeights(bool showStatusMessages, bool global)
{
	// Here we take the starting scene and evaluate the variance of all available edits
	// when performed on the starting scene. The idea is to measure how much each edit
	// "matters" and use that to bias local searches towards meaningfuly different changes
  _localEditWeights.clear();
	Eigen::VectorXd weights;
	weights.resize(_edits.size());

  _statusMessage = "Computing " + String(_edits.size()) + " edit weights";

	for (int i = 0; i < _edits.size(); i++) {
    if (threadShouldExit())
      return;
    if (getGlobalSettings()->_uniformEditWeights) {
      // uniform edit weight
      weights[i] = 1;
      _statusMessage = "Precomputing Edit Weights... (" + String(i + 1) + "/" + String(_edits.size()) + ")";
    }
    else {
      if (showStatusMessages) {
        MessageManagerLock mmlock(this);
        if (mmlock.lockWasGained()) {
          getStatusBar()->setStatusMessage(_statusMessage);
        }
      }

      double weight = sqrt(_edits[i]->expected(_original, _f, getGlobalSettings()->_editStepSize * 3, 50));

      // minimum weight, prevent conflicts and also enable all edits to possibly be chosen
      weights[i] = (weight < 1e-1) ? 1e-1 : weight;
    }
	}

  _statusMessage = "Precomputing Edit Weights... (" + String(_edits.size()) + "/" + String(_edits.size()) + ")";
  if (showStatusMessages) {
    MessageManagerLock mmlock(this);
    if (mmlock.lockWasGained()) {
      getStatusBar()->setStatusMessage(_statusMessage);
    }
  }

	// normalize to [0,1] and update weights
  if (global) {
    map<double, Edit*>& editWeights = getGlobalSettings()->_globalEditWeights;

    double sum = weights.sum();
    double total = 0;

    for (int i = 0; i < _edits.size(); i++) {
      total += weights[i] / sum;
      editWeights[total] = _edits[i];
    }

    // reset to idle state after completion
    // also update shared values to indicate completion
    _sharedData["Edit Weight Status"] = 2;
    _status = IDLE;
  }
  else {
    double sum = weights.sum();
    double total = 0;

    for (int i = 0; i < _edits.size(); i++) {
      total += weights[i] / sum;
      _localEditWeights[total] = _edits[i];
    }
  }

  _statusMessage = "Precompute complete";
}

void AttributeSearchThread::setStartConfig(Snapshot * start)
{
	if (_original != nullptr)
		delete _original;

	_original = new Snapshot(*start);
	_acceptedSamples = 0;
}

void AttributeSearchThread::recenter(Snapshot * s)
{
  _statusMessage = "Recentering thread";

  // for now, thread id 0 can never be recentered, but it can use a larger edit depth
  if (_id >= _resampleThreads) {
    _maxDepth++;
    return;
  }

  // if we weren't explicitly told where to go, recenter according to search mode
  if (s == nullptr) {
    if (_mode == RANDOM_START) {
      s = new Snapshot(*_fallback);
    }
    else if (_mode == KRANDOM_START || _mode == KMCMC || _mode == REPULSION_KMCMC) {
      // Select a k-sized fontier based off of a clustering of the current result set.
      _frontier.clear();
      Array<shared_ptr<SearchResultContainer> > fc = _viewer->getKCenters(_k, _distMetric);

      // log the new frontier
      for (auto r : fc) {
        if (r == nullptr)
          continue;

        _frontier.add(shared_ptr<Snapshot>(vectorToSnapshot(r->getSearchResult()->_scene)));

        // log it
        DebugData data;
        auto& samples = getGlobalSettings()->_samples;
        data._f = r->getSearchResult()->_objFuncVal;
        data._sampleId = r->getSearchResult()->_sampleNo;
        data._editName = "FRONTIER ELEMENT";
        data._accepted = true;
        data._scene = r->getSearchResult()->_scene;
        data._timeStamp = chrono::high_resolution_clock::now();
        samples[_id].push_back(data);
      }

      if (_frontier.size() == 0)
      {
        // fallback if the frontier is empty for some reason
        _frontier.add(shared_ptr<Snapshot>(new Snapshot(*_fallback)));
      }

      uniform_int_distribution<int> rng(0, _frontier.size() - 1);
      setStartConfig(_frontier[rng(_gen)].get());

      _samplesTaken = 0;
      getRecorder()->log(SYSTEM, "Recentered thread " + String(_id).toStdString() + " to new scene");

      return;
    }
    else {
      // if the snapshot is null we should ask the viewer for the 
      // best not already exploited scene in the results
      auto container = _viewer->getBestUnexploitedResult();

      if (container == nullptr) {
        // if there's nothing left to exploit, reset back to the beginning for a bit
        setStartConfig(_fallback);
        getRecorder()->log(SYSTEM, "Recentered thread " + String(_id).toStdString() + " to original config");
        updateEditWeights(nullptr, 0);
        return;
      }
      else {
        s = vectorToSnapshot(container->getSearchResult()->_scene);

        // log which config we moved to
        DebugData data;
        auto& samples = getGlobalSettings()->_samples;
        data._f = container->getSearchResult()->_objFuncVal;
        data._a = 1;
        data._sampleId = container->getSearchResult()->_sampleNo;
        data._editName = "MOVE TARGET";
        data._accepted = true;
        data._scene = container->getSearchResult()->_scene;
        data._timeStamp = chrono::high_resolution_clock::now();
        samples[_id].push_back(data);

        _maxDepth = getGlobalSettings()->_startChainLength;
      }
    }
  }
  
  setStartConfig(s);

  if (_mode == RANDOM_START) {
    randomizeStart();
  }

  if (_editMode == DEFAULT_CHOICE) {
    setLocalWeightsUniform();
  }
  else if (_editMode == SIMPLE_BANDIT || _editMode == ADVERSARIAL_BANDIT || _editMode == DIRECTED_SAMPLING) {
    updateEditWeights(nullptr, 0);
  }
  else if (_editMode == UNIFORM_RANDOM) {
    initEditWeights();
  }

  _samplesTaken = 0;
  getRecorder()->log(SYSTEM, "Recentered thread " + String(_id).toStdString() + " to new scene");
  delete s;
}

Edit * AttributeSearchThread::getNextEdit(vector<Edit*>& editHistory, bool useHistory)
{
  if (_editMode == DIRECTED_SAMPLING) {
    if (editHistory.size() == 0) {
      // if we haven't done an edit, lets just start with the gibbs sampler
      for (Edit* e : _edits) {
        if (dynamic_cast<GibbsEdit*>(e)) {
          return e;
        }
      }
    }
    else {
      Edit* selected = _localEditWeights.lower_bound(_udist(_gen))->second;
      while (dynamic_cast<GibbsEdit*>(selected) != nullptr) {
        selected = _localEditWeights.lower_bound(_udist(_gen))->second;
      }
      return selected;
    }
  }

  // weighted selection
  if (!useHistory || editHistory.size() == 0 || _editMode == UNIFORM_RANDOM) {
    return _localEditWeights.lower_bound(_udist(_gen))->second;
  }

  // decrease likelihood of edits being repeated.
  // reconstruct original weights
  map<Edit*, double> w;
  double prev = 0;
  for (auto it = _localEditWeights.begin(); it != _localEditWeights.end(); it++) {
    w[it->second] = it->first - prev;
    prev = it->first;
  }

  // decrease probability of selection each time edit is encountered in history
  for (auto& e : editHistory) {
    w[e] = w[e] / 2;
    w[e] = (w[e] < 1e-3) ? 1e-3 : w[e];
  }

  // recompute weights
  double sum = 0;
  for (auto& kvp : w) {
    sum += kvp.second;
  }

  map<double, Edit*> reweights;
  double total = 0;
  for (auto& kvp : w) {
    total += kvp.second / sum;
    reweights[total] = kvp.first;
  }

  // if this is the directed sampler, we never want to run the gibbs sampler again in a chain
  return reweights.lower_bound(_udist(_gen))->second;
}

void AttributeSearchThread::runSearch()
{
  if (_mode == REDUCE_REDUNDANCY) {
    runSearchNoInnerLoop();
  }
  else if (_mode == RANDOM_START) {
    runRandomStartSearch();
  }
  else if (_mode == KRANDOM_START) {
    runKRandomStartSearch();
  }
  else if (_mode == KMCMC) {
    runKSearch();
  }
  else if (_mode == CMAES) {
    runCMAES();
  }
  else if (_mode == REPULSION_KMCMC) {
    runRepulsionKMCMC();
  }
  else if (_mode == GIBBS_SAMPLING) {
    runGibbsSampling();
  }

  if (_useStyles) {
    // pick new random style
    _currentStyle = (Style)_styleDist(_gen);
  }
}

void AttributeSearchThread::runSearchNoInnerLoop()
{
  _statusMessage = "Running Recenter-Move MCMC with fast start";

  double fx = _f(_original, _id, _currentStyle);

  // assign start scene, initialize result
  Snapshot* start = new Snapshot(*_original);
  shared_ptr<SearchResult> r = shared_ptr<SearchResult>(new SearchResult());
  double orig = fx;

  // RNG
  default_random_engine gen(std::random_device{}());
  uniform_real_distribution<double> udist(0.0, 1.0);

  // do the MCMC search
  int depth = 0;
  Edit* e = nullptr;

  // magic number alert
  int iters = getGlobalSettings()->_maxMCMCIters;

  // depth increases when scenes are rejected from the viewer
  while (depth < _maxDepth) {
    if (threadShouldExit()) {
      delete start;
      return;
    }

    //  pick a next plausible edit
    if (getGlobalSettings()->_editSelectMode == DEFAULT_CHOICE) {
      e = getNextEdit(r->_editHistory, getGlobalSettings()->_reduceRepeatEdits);
    }
    else {
      e = getNextEdit(r->_editHistory);
    }

    //  adjust the starting scene
    Snapshot* sp = new Snapshot(*start);
    e->performEdit(sp, getGlobalSettings()->_editStepSize);

    // check for acceptance
    double fxp = _f(sp, _id, _currentStyle);
    double diff = fxp - fx;
    double a = min(exp((1 / _T) * (fx - fxp)), 1.0);

    // store statistics
    _editStats[e]._vals.push_back((float)fxp);
    _editStats[e]._diffs.push_back((float)(fx - fxp));
    _editStats[e]._as.push_back((float)a);

    // accept if a >= 1 or with probability a
    if (a >= 1 || udist(gen) < a) {
      // update x
      delete start;
      start = sp;
      fx = fxp;

      // update result
      r->_objFuncVal = fx;

      // diagnostics
      DebugData data;
      auto& samples = getGlobalSettings()->_samples;
      data._f = r->_objFuncVal;
      data._a = a;
      data._sampleId = (unsigned int)samples[_id].size() + 1;
      data._editName = e->_name;
      data._accepted = true;
      data._scene = snapshotToVector(sp);
      data._timeStamp = chrono::high_resolution_clock::now();
      samples[_id].push_back(data);

      _editStats[e]._success += 1;
      r->_editHistory.push_back(e);
    }
    else {
      _editStats[e]._failure += 1;
      delete sp;
    }

    updateEditWeights(e, diff);
    depth++;
  }

  r->_scene = snapshotToVector(start);

  // diagnostics
  DebugData data;
  auto& samples = getGlobalSettings()->_samples;
  data._f = r->_objFuncVal;
  data._a = 1;
  data._sampleId = (unsigned int)samples[_id].size() + 1;
  data._editName = "TERMINAL";
  data._accepted = true;
  data._scene = r->_scene;
  data._timeStamp = chrono::high_resolution_clock::now();

  r->_extraData["Thread"] = String(_id);
  r->_extraData["Sample"] = String(data._sampleId);
  r->_extraData["Style"] = String(_currentStyle);
  r->_creationTime = chrono::high_resolution_clock::now();

  // add if we did better
  // also if the mask is active, add only if the masked area doesn't change that much.
  double maskDiff = 0;
  if (_useMask) {
    // need target and current images
    Image a = renderImage(_original, 100, 100);
    Image b = renderImage(start, 100, 100);
    maskDiff = avgLabMaskedImgDiff(a, b, _freezeMask);
  }
  delete start;

  //Lumiverse::Logger::log(INFO, "Result with f(x) " + String(r->_objFuncVal).toStdString() + " and maskDiff " + String(maskDiff).toStdString() + " returned.");

  if (r->_objFuncVal < orig && maskDiff < _maskTolerance) {
    // send scene to the results area. may chose to not use the scene
    if (!_viewer->addNewResult(r, _id, _dispMetric)) {
      // r has been deleted by _viewer here
      _failures++;
      data._accepted = false;

      if (_failures > getGlobalSettings()->_searchFailureLimit) {
        _failures = 0;
        _maxDepth++;
      }
    }
    else {
      _acceptedSamples += 1;
    }
  }
  else {
    data._accepted = false;
  }

  samples[_id].push_back(data);
  _samplesTaken++;

  if (_samplesTaken > _resampleTime) {
    // recentering resets the weights
    recenter();
  }
}

void AttributeSearchThread::runRandomStartSearch()
{
  _statusMessage = "Running Random center search";

  runSearchNoInnerLoop();
}

void AttributeSearchThread::runKRandomStartSearch()
{
  _statusMessage = "Starting KRandom Search loop";

  // randomize start location based on what's in the frontier
  uniform_int_distribution<int> rng(0, _frontier.size() - 1);
  setStartConfig(_frontier[rng(_gen)].get());

  runSearchNoInnerLoop();
}

void AttributeSearchThread::runKSearch()
{
  _statusMessage = "Starting K-means search loop";

  // randomize start location based on what's in the frontier
  uniform_int_distribution<int> rng(0, _frontier.size() - 1);
  setStartConfig(_frontier[rng(_gen)].get());

  runSearchNoInnerLoop();
}

void AttributeSearchThread::runCMAES()
{
  _statusMessage = "Running CMAES";

  const int baseLambda = 100;
  Eigen::VectorXd startX = snapshotToVector(_original);
  vector<Eigen::VectorXd> results;

  Eigen::VectorXd x = CMAESHelper(startX, baseLambda, 20, nullptr);
  x = CMAESHelper(x, baseLambda, 20, nullptr);
  x = CMAESHelper(x, baseLambda, 20, nullptr);
  x = CMAESHelper(x, baseLambda, 20, nullptr);
  x = CMAESHelper(x, baseLambda, 50, nullptr);
  x = CMAESHelper(x, baseLambda, 50, nullptr);
  x = CMAESHelper(x, baseLambda, 50, &results);
  x = CMAESHelper(x, baseLambda, 200, &results);

  // insert results
  for (auto s : results) {
    // create result container
    shared_ptr<SearchResult> r = shared_ptr<SearchResult>(new SearchResult());
    r->_scene = s;
    
    Snapshot* sn = vectorToSnapshot(s);
    r->_objFuncVal = _f(sn, _id, NO_STYLE);
    delete sn;

    // diagnostics
    DebugData data;
    auto& samples = getGlobalSettings()->_samples;
    data._f = r->_objFuncVal;
    data._sampleId = (unsigned int)samples[_id].size() + 1;
    data._editName = "CMA-ES";
    data._accepted = true;
    data._scene = r->_scene;
    data._timeStamp = chrono::high_resolution_clock::now();

    r->_extraData["Thread"] = String(_id);
    r->_extraData["Sample"] = String(data._sampleId);
    r->_creationTime = chrono::high_resolution_clock::now();

    // force add all results for CMA-ES
    if (!_viewer->addNewResult(r, _id, _dispMetric, true)) {
      data._accepted = false;
    }
  }
}

void AttributeSearchThread::runRepulsionKMCMC()
{
  _statusMessage = "Running K-Frontier MCMC with repulsion term";

  // randomize start location based on what's in the frontier
  uniform_int_distribution<int> rng(0, _frontier.size() - 1);
  setStartConfig(_frontier[rng(_gen)].get());

  double fx = _f(_original, _id, _currentStyle);

  // assign start scene, initialize result
  Snapshot* start = new Snapshot(*_original);
  shared_ptr<SearchResult> r = shared_ptr<SearchResult>(new SearchResult());
  double orig = fx;

  // RNG
  default_random_engine gen(std::random_device{}());
  uniform_real_distribution<double> udist(0.0, 1.0);

  // do the MCMC search
  int depth = 0;
  Edit* e = nullptr;

  // magic number alert
  int iters = getGlobalSettings()->_maxMCMCIters;

  // depth increases when scenes are rejected from the viewer
  while (depth < _maxDepth) {
    if (threadShouldExit()) {
      delete start;
      return;
    }

    //  pick a next plausible edit
    if (getGlobalSettings()->_editSelectMode == DEFAULT_CHOICE) {
      e = getNextEdit(r->_editHistory, getGlobalSettings()->_reduceRepeatEdits);
    }
    else {
      e = getNextEdit(r->_editHistory);
    }

    //  adjust the starting scene
    Snapshot* sp = new Snapshot(*start);
    e->performEdit(sp, getGlobalSettings()->_editStepSize);

    // check for acceptance
    double fxp = _f(sp, _id, _currentStyle) + repulsion(sp);
    double diff = fxp - fx;
    double a = min(exp((1 / _T) * (fx - fxp)), 1.0);

    // store statistics
    _editStats[e]._vals.push_back((float)fxp);
    _editStats[e]._diffs.push_back((float)(fx - fxp));
    _editStats[e]._as.push_back((float)a);

    // accept if a >= 1 or with probability a
    if (a >= 1 || udist(gen) < a) {
      // update x
      delete start;
      start = sp;
      fx = fxp;

      // update result
      r->_objFuncVal = fx;

      // diagnostics
      DebugData data;
      auto& samples = getGlobalSettings()->_samples;
      data._f = r->_objFuncVal;
      data._a = a;
      data._sampleId = (unsigned int)samples[_id].size() + 1;
      data._editName = e->_name;
      data._accepted = true;
      data._scene = snapshotToVector(sp);
      data._timeStamp = chrono::high_resolution_clock::now();
      samples[_id].push_back(data);

      _editStats[e]._success += 1;
      r->_editHistory.push_back(e);
    }
    else {
      _editStats[e]._failure += 1;
      delete sp;
    }

    updateEditWeights(e, diff);
    depth++;
  }

  r->_scene = snapshotToVector(start);

  // diagnostics
  DebugData data;
  auto& samples = getGlobalSettings()->_samples;
  data._f = r->_objFuncVal;
  data._a = 1;
  data._sampleId = (unsigned int)samples[_id].size() + 1;
  data._editName = "TERMINAL";
  data._accepted = true;
  data._scene = r->_scene;
  data._timeStamp = chrono::high_resolution_clock::now();

  r->_extraData["Thread"] = String(_id);
  r->_extraData["Sample"] = String(data._sampleId);
  r->_extraData["Style"] = String(_currentStyle);
  r->_creationTime = chrono::high_resolution_clock::now();

  // add if we did better
  // also if the mask is active, add only if the masked area doesn't change that much.
  double maskDiff = 0;
  if (_useMask) {
    // need target and current images
    Image a = renderImage(_original, 100, 100);
    Image b = renderImage(start, 100, 100);
    maskDiff = avgLabMaskedImgDiff(a, b, _freezeMask);
  }
  delete start;

  //Lumiverse::Logger::log(INFO, "Result with f(x) " + String(r->_objFuncVal).toStdString() + " and maskDiff " + String(maskDiff).toStdString() + " returned.");

  if (r->_objFuncVal < orig && maskDiff < _maskTolerance) {
    // send scene to the results area. may chose to not use the scene
    if (!_viewer->addNewResult(r, _id, _dispMetric, false, _currentResults)) {
      // r has been deleted by _viewer here
      _failures++;
      data._accepted = false;

      if (_failures > getGlobalSettings()->_searchFailureLimit) {
        _failures = 0;
        _maxDepth++;
      }
    }
    else {
      _acceptedSamples += 1;
    }

    // update stats for repulsion term 
    updateRepulsionVars();
  }
  else {
    data._accepted = false;
  }

  samples[_id].push_back(data);
  _samplesTaken++;

  if (_samplesTaken > _resampleTime) {
    // recentering resets the weights
    recenter();
  }
}

void AttributeSearchThread::runGibbsSampling()
{
  // fundamentally different than MCMC sampling, here we already know what colors we want
  // and maybe what intensites we're interested in. We therefore generate samples based
  // on these priors and present them to the user. We may also choose to present
  // other options to the user outside of their explicit constraints.
  // timing diagnostics
  auto start = chrono::high_resolution_clock::now();

  Snapshot* sample = new Snapshot(*_original);
  auto deviceData = sample->getRigData();

  // we have a list of sampling schedules and the affected devices.
  // Unless otherwise specified, we assume all devices in the sets are adjusted
  // as a system
  for (auto schedule : _activeSchedule) {
    if (threadShouldExit()) {
      delete sample;
      return;
    }

    DeviceSet devices = schedule.first._targets;

    if (schedule.first._followConventions) {
      // determine number of affected systems
      set<string> systems = devices.getAllMetadataForKey("system");
      vector<float> sampleData;

      if (schedule.first._param == GINTENSITY) {
        sampleData.resize(systems.size());

        // for now we assume all things are free
        // TODO: CHECK LOCKS
        vector<GibbsConstraint> constraints;
        for (int i = 0; i < sampleData.size(); i++)
          constraints.push_back(FREE);

        schedule.second->sampleIntensity(sampleData, constraints);

        // link system with val
        map<string, float> systemToVal;
        int i = 0;
        for (auto s : systems) {
          systemToVal[s] = sampleData[i];
          i++;
        }

        // update devices
        for (auto id : devices.getIds()) {
          if (!isDeviceParamLocked(id, "intensity")) {
            deviceData[id]->getIntensity()->setValAsPercent(systemToVal[deviceData[id]->getMetadata("system")]);
          }
        }
      }
      else if (schedule.first._param == GCOLOR) {
        sampleData.resize(systems.size() * 3);

        // for now we assume all things are free
        // TODO: CHECK LOCKS
        vector<GibbsConstraint> constraints;
        for (int i = 0; i < systems.size(); i++)
          constraints.push_back(FREE);

        schedule.second->sampleColor(sampleData, constraints);

        // link system with val
        map<string, int> systemToVal;
        int i = 0;
        for (auto s : systems) {
          systemToVal[s] = i;
          i++;
        }

        // update devices
        for (auto id : devices.getIds()) {
          if (!isDeviceParamLocked(id, "color")) {
            int index = systemToVal[deviceData[id]->getMetadata("system")] * 3;
            float hue = sampleData[index];
            float sat = sampleData[index + 1];
            float val = sampleData[index + 2];
            deviceData[id]->setColorHSV("color", hue * 360.0f, sat, val);
          }
        }
      }
    }
    else {
      // TODO: FILL THIS IN. TESTING BASIC FUNCTIONALITY WITH CONVENTIONAL VERSION
    }
  }

  // now that we have the sample, apply the usual display criteria to it
  shared_ptr<SearchResult> r = shared_ptr<SearchResult>(new SearchResult());
  r->_scene = snapshotToVector(sample);

  // diagnostics
  DebugData data;
  auto& samples = getGlobalSettings()->_samples;
  data._sampleId = (unsigned int)samples[_id].size() + 1;
  data._editName = "GIBBS SAMPLE";
  data._accepted = true;
  data._scene = r->_scene;
  data._timeStamp = chrono::high_resolution_clock::now();

  r->_extraData["Thread"] = String(_id);
  r->_extraData["Sample"] = String(data._sampleId);
  r->_creationTime = chrono::high_resolution_clock::now();

  // at this point all the data has been saved in the search result
  delete sample;

  // send scene to the results area. may chose to not use the scene
  if (!_viewer->addNewResult(r, _id, _dispMetric, false)) {
    // r has been deleted by _viewer here
    _failures++;
    data._accepted = false;
  }

  samples[_id].push_back(data);
  _samplesTaken++;

  // timing diagnostics
  getGlobalSettings()->_timings[_id]._sampleTime += chrono::duration<float>(chrono::high_resolution_clock::now() - start).count();
  getGlobalSettings()->_timings[_id]._numSamples += 1;

  // repeat infinitely
}

Eigen::VectorXd AttributeSearchThread::CMAESHelper(const Eigen::VectorXd & startingPoint, int lambda, int maxIters, vector<Eigen::VectorXd>* candidates)
{
  const float baseStdDev = 0.01f;
  cmaes_t opt;

  const int dimension = startingPoint.size();
  vector<double> xStart, stdDev;
  for (int i = 0; i < startingPoint.size(); i++) {
    stdDev.push_back(baseStdDev);
    xStart.push_back(startingPoint(i));
  }

  double *arFunVals = cmaes_init(&opt, dimension, xStart.data(), stdDev.data(), 0, lambda, nullptr);

  int iter = 0;

  while (!cmaes_TestForTermination(&opt) && iter <= maxIters) {
    double* const* pop = cmaes_SamplePopulation(&opt);

    int bestPopIndex = 0;
    double bestPopValue = std::numeric_limits<double>::max();

    for (int i = 0; i < lambda; i++) {
      Eigen::VectorXd x;
      x.resizeLike(startingPoint);

      for (int j = 0; j < dimension; j++) {
        x[j] = pop[i][j];
      }

      Snapshot* s = vectorToSnapshot(x);
      arFunVals[i] = _f(s, _id, NO_STYLE);
      delete s;

      if (arFunVals[i] < bestPopValue) {
        bestPopIndex = i;
        bestPopValue = arFunVals[i];
      }
    }

    if (candidates != nullptr) {
      Eigen::VectorXd x;
      x.resizeLike(startingPoint);

      for (int j = 0; j < dimension; j++) {
        x[j] = pop[bestPopIndex][j];
      }
      candidates->push_back(x);
    }

    iter++;
    cmaes_UpdateDistribution(&opt, arFunVals);
  }

  double* xFinal = cmaes_GetNew(&opt, "xbestever");
  cmaes_exit(&opt);

  Eigen::VectorXd result;
  result.resizeLike(startingPoint);
  for (int j = 0; j < dimension; j++) {
    result[j] = xFinal[j];
  }
  free(xFinal);

  return result;
}

Eigen::VectorXd AttributeSearchThread::getDerivative(Snapshot & s)
{
	Eigen::VectorXd x = snapshotToVector(&s);
	Eigen::VectorXd dx;
	dx.resizeLike(x);
	double fx = _fsq(&s, _id, _currentStyle);

	// adjust each parameter in order
	double h = 1e-3;

	for (int i = 0; i < dx.size(); i++) {
    // we're bounded by some physical restrictions here, however, they get
    // partially addressed by the vector to snapshot function
    if (x[i] >= 1) {
      x[i] -= h;
      vectorToExistingSnapshot(x, s);
      double fxp = _fsq(&s, _id, _currentStyle);
      dx[i] = (fx - fxp) / h;
      x[i] += h;
    }
    else {
      x[i] += h;
      vectorToExistingSnapshot(x, s);
      double fxp = _fsq(&s, _id, _currentStyle);
      dx[i] = (fxp - fx) / h;
      x[i] -= h;
    }
	}

	return dx;
}

Eigen::MatrixXd AttributeSearchThread::getJacobian(Snapshot & s)
{
	// The jacobian is the matrix consisting of first order derivatives for
	// each function being considered. At the moment, we just have one.

	Eigen::VectorXd dx = getDerivative(s);

	Eigen::MatrixXd J;

	// right now J is basically a row vector, if more functions are added in the
	// future, this will change
	J.resize(1, dx.size());
	J.row(0) = dx;

	return J;
}

void AttributeSearchThread::randomizeStart()
{
  default_random_engine gen(std::random_device{}());
  uniform_real_distribution<float> udist(50, 500);
  uniform_real_distribution<float> edist(0, getGlobalSettings()->_edits.size());

  // only use the edits in globals to maintain consistency
  for (int i = 0; i < (int)udist(gen); i++) {
    int id = (int)edist(gen);
    getGlobalSettings()->_edits[id]->performEdit(_original, 0.5); // go nuts
  }
}

Eigen::VectorXd AttributeSearchThread::performLMGD(Snapshot* scene, double& finalObjVal)
{
  // implementation based off of Non-Linear Methods for Least Squares Problems, 2nd ed.
  // by K Madsen et al.
  Snapshot xs(*scene);
  Eigen::VectorXd x = snapshotToVector(&xs);

  int maxIters = getGlobalSettings()->_maxGradIters;
  double nu = 2;
  double eps = 1e-3;
  double eps2 = 1e-6;
  double tau = 1e-3;
  double fx = _fsq(&xs, _id, _currentStyle);
  double forig = fx;

  Eigen::MatrixXd J = getJacobian(xs);
  Eigen::MatrixXd H = J.transpose() * J;		// may need to replace with actual calculation of Hessian
  Eigen::MatrixXd g = J.transpose() * _fsq(&xs, _id, _currentStyle);

  double mu = tau * H.diagonal().maxCoeff();

  bool found = false;
  int k = 0;

  while (!found && k < maxIters) {
    if (threadShouldExit()) {
      x.setZero();
      return x;
    }

    k = k + 1;
    Eigen::MatrixXd inv = (H + mu * Eigen::MatrixXd::Identity(H.rows(), H.cols())).inverse();
    Eigen::MatrixXd hlm = -inv * g;

    if (hlm.norm() <= eps2 * (x.norm() + eps2)) {
      found = true;
    }
    else {
      Eigen::VectorXd xnew = x + hlm;

      // fix constraints, bounded at 1 and 0
      for (int i = 0; i < x.size(); i++) {
        xnew[i] = Lumiverse::clamp((float)xnew[i], 0, 1);
      }

      vectorToExistingSnapshot(xnew, xs);
      double fnew = _fsq(&xs, _id, _currentStyle);
      Eigen::MatrixXd denom = (0.5 * hlm.transpose() * (mu * hlm - g));
      double rhoval = (fx * fx / 2 - fnew * fnew / 2) / denom(0);

      if (rhoval > 0) {
        // acceptable step
        x = xnew;
        J = getJacobian(xs);
        g = J.transpose() * fnew;
        found = (g.norm() <= eps);
        mu = mu * max(1.0 / 3.0, 1 - pow(2 * rhoval - 1, 3));
        nu = 2;
        fx = fnew;

        // debug data
        DebugData data;
        auto& samples = getGlobalSettings()->_samples;
        data._f = fnew;
        data._a = 1;
        data._sampleId = (unsigned int)samples[_id].size() + 1;
        data._editName = "L-M DESCENT STEP";
        data._accepted = true;
        data._scene = x;
        data._timeStamp = chrono::high_resolution_clock::now();
        samples[_id].push_back(data);
      }
      else {
        mu = mu * 2; //nu;
        // nu = 2 * nu;

        // reset
        vectorToExistingSnapshot(x, xs);
      }
    }
  }

  finalObjVal = fx;
  return x;
}

void AttributeSearchThread::setLocalWeightsUniform()
{
  _localEditWeights.clear();

  float sum = _edits.size();

  for (int i = 0; i < _edits.size(); i++) {
    _localEditWeights[(i + 1) / sum] = _edits[i];
    _editStats[_edits[i]] = EditStats();
  }

  _statusMessage = "Edit weights set to uniform";
}

void AttributeSearchThread::updateEditWeights(Edit* lastUsed, double gain)
{
  if (_editMode == DEFAULT_CHOICE) {
    // compute expected positive diff for each edit that's been looked at
    map<Edit*, float> expectedDiff;
    for (auto stat : _editStats) {
      if (stat.second._diffs.size() > 0) {
        expectedDiff[stat.first] = max(-stat.second.expectedDiff(), 0.01f);
      }
      else {
        expectedDiff[stat.first] = 0.1f;
      }
    }

    // question is how to balance things with unknown weight vs known weights
    // for now, minimum weight is 1
    float totalWeight = 0;
    for (auto e : _edits) {
      totalWeight += expectedDiff[e];
    }

    map<double, Edit*> weights;
    float sum = 0;
    for (auto e : _edits) {
      sum += expectedDiff[e];
      weights[sum / totalWeight] = e;
    }

    _localEditWeights = weights;
  }
  else if (_editMode == SIMPLE_BANDIT || _editMode == DIRECTED_SAMPLING) {
    // initial weights are the ratio of success to failure
    map<double, Edit*> weights;
    float sum = 0;
    double totalWeight = 0;

    for (auto data : _editStats) {
      totalWeight += (double)data.second._success / (data.second._failure + data.second._success);
    }

    // then the weights are normalized
    for (auto e : _edits) {
      sum += (double)_editStats[e]._success / (_editStats[e]._success + _editStats[e]._failure);
      weights[sum / totalWeight] = e;
    }

    _localEditWeights = weights;
  }
  else if (_editMode == UNIFORM_RANDOM) {
    // nothing, its uniform all the time i hope the compiler catches this
  }
  else if (_editMode == ADVERSARIAL_BANDIT) {
    // update G based on previous edit, if it exists
    if (lastUsed != nullptr) {
      if (gain <= 0) {
        // note: negative gain good, since this is a minimization
        gain = -gain;

        _editStats[lastUsed]._G += gain / _editStats[lastUsed]._p;
      }
    }

    // probability dist from hedge
    Eigen::VectorXd pt;
    pt.resize(_edits.size());

    for (int i = 0; i < _edits.size(); i++) {
      pt[i] = _n * _editStats[_edits[i]]._G;
    }

    pt /= pt.sum();

    map<double, Edit*> weights;
    double sum = 0;
    for (int i = 0; i < _edits.size(); i++) {
      double weight = pt[i] * (1 - _g) * pt[i] + (_g / _edits.size());
      weights[sum] = _edits[i];
      _editStats[_edits[i]]._p = weight;
      sum += weight;
    }

    _localEditWeights = weights;
  }
}

void AttributeSearchThread::initEditWeights()
{
  _localEditWeights.clear();
  _editStats.clear();

  // update the edit weights
  if (_editMode == DEFAULT_CHOICE || _editMode == UNIFORM_RANDOM) {
    double weight = 1.0 / _edits.size();
    for (int i = 0; i < _edits.size(); i++) {
      _localEditWeights[weight * (i + 1)] = _edits[i];
    }
  }
  else if (_editMode == SIMPLE_BANDIT || _editMode == DIRECTED_SAMPLING) {
    for (auto e : _edits) {
      // some default weights for this thing
      _editStats[e]._success = 5;
      _editStats[e]._failure = 10;
    }

    updateEditWeights(nullptr, 0);
  }
  else if (_editMode == ADVERSARIAL_BANDIT) {
    for (auto e : _edits) {
      _editStats[e]._G = 0;
    }

    updateEditWeights(nullptr, 0);
  }
}

double AttributeSearchThread::repulsion(Snapshot * s)
{
  shared_ptr<SearchResult> t = shared_ptr<SearchResult>(new SearchResult());
  t->_scene = snapshotToVector(s);
  SearchResultContainer* temp = new SearchResultContainer(t, false);
  temp->setImage(renderImage(s, 100, 100));

  double sum = repulsionTerm(temp, _currentResults, _costScale, _coneRadius, _distMetric);

  delete temp;
  return sum;
}

void AttributeSearchThread::updateRepulsionVars()
{
  // If the size is the same, the results are the same and we should be updated.
  if (_previousResultsSize == _currentResults.size())
    return;

  _previousResultsSize = _currentResults.size();

  if (_previousResultsSize == 1) {
    _coneRadius = 0;
    return;
  }

  uniform_int_distribution<int> rng(0, _currentResults.size() - 1);
  double avgDist = 0;

  // pull n random pairs
  for (int i = 0; i < _numPairs; i++) {
    int idx1 = rng(_gen);
    int idx2 = rng(_gen);

    while (idx2 == idx1) {
      idx2 = rng(_gen);
    }

    avgDist += _currentResults[idx1]->dist(_currentResults[idx2].get(), _distMetric, false, false);
  }

  _coneRadius = avgDist / _numPairs;
  return;
}

AttributeSearch::AttributeSearch(SearchResultsViewer * viewer) : _viewer(viewer), Thread("Attribute Search Dispatcher")
{
  reinit();
  _start = nullptr;
}

AttributeSearch::~AttributeSearch()
{
  for (int i = 0; i < _threads.size(); i++) {
    _threads[i]->stopThread(50);
    delete _threads[i];
  }

  _threads.clear();

  delete _start;
}

void AttributeSearch::reinit()
{
  // assumes attribute search is stopped
  assert(isThreadRunning() == false);

  for (auto& t : _threads)
    delete t;

  _threads.clear();

  for (int i = 0; i < getGlobalSettings()->_searchThreads; i++) {
    _threads.add(new AttributeSearchThread("Attribute Searcher Worker " + String(i), _viewer, _sharedData));
  }
}

void AttributeSearch::setState(Snapshot* start, map<string, AttributeControllerBase*> active)
{
  if (_start != nullptr)
    delete _start;

  if (_threads.size() != getGlobalSettings()->_searchThreads)
    reinit();

  // need to freeze status of attributes but still have access to their functions
  _active.clear();
  for (auto& attr : active) {
    _active[attr.first] = pair<AttributeControllerBase*, AttributeConstraint>(attr.second, attr.second->getStatus());
  }

  _start = start;

  // MAGIC NUMBER ALERT - canonical size
  _freezeMask = getGlobalSettings()->_freeze.rescaled(100, 100);

  // quick check to see if the mask is actually filled in
  _useMask = false;
  for (int y = 0; y < _freezeMask.getHeight(); y++) {
    for (int x = 0; x < _freezeMask.getWidth(); x++) {
      _useMask |= (_freezeMask.getPixelAt(x, y).getBrightness() > 0);
    }
  }

  // objective functions for combined set of active attributes.
  _f = [this](Snapshot* s, int callingThreadId, Style st) {
    auto start = chrono::high_resolution_clock::now();
    if (_active.size() == 0)
      return 0.0;

    // for multiple attributes, we generate the image here first
    Image img = _active.begin()->second.first->generateImage(s);
    getGlobalSettings()->_timings[callingThreadId]._sampleRenderTime += chrono::duration<float>(chrono::high_resolution_clock::now() - start).count();

    // compute this image on demand
    Image startImg;
    double sum = 0;
    for (const auto& kvp : _active) {
      if (kvp.second.second == A_LESS) {
        auto evalStart = chrono::high_resolution_clock::now();
        sum += kvp.second.first->evaluateScene(s, img);
        getGlobalSettings()->_timings[callingThreadId]._sampleEvalTime += chrono::duration<float>(chrono::high_resolution_clock::now() - evalStart).count();
      }
      else if (kvp.second.second == A_MORE) {
        auto evalStart = chrono::high_resolution_clock::now();
        sum -= kvp.second.first->evaluateScene(s, img);
        getGlobalSettings()->_timings[callingThreadId]._sampleEvalTime += chrono::duration<float>(chrono::high_resolution_clock::now() - evalStart).count();
      }
      else if (kvp.second.second == A_EQUAL) {
        if (startImg.getWidth() == 0) {
          auto renderStart = chrono::high_resolution_clock::now();
          Image startImg = _active.begin()->second.first->generateImage(_start);
          getGlobalSettings()->_timings[callingThreadId]._sampleRenderTime += chrono::duration<float>(chrono::high_resolution_clock::now() - renderStart).count();
        }
        auto evalStart = chrono::high_resolution_clock::now();
        sum += pow(kvp.second.first->evaluateScene(s, img) - kvp.second.first->evaluateScene(_start, startImg), 2);
        getGlobalSettings()->_timings[callingThreadId]._sampleRenderTime += chrono::duration<float>(chrono::high_resolution_clock::now() - evalStart).count();
      }
    }

    // freeze region penalty
    if (_useMask) {
      if (startImg.getWidth() == 0) {
        auto renderStart = chrono::high_resolution_clock::now();
        Image startImg = _active.begin()->second.first->generateImage(_start);
        getGlobalSettings()->_timings[callingThreadId]._sampleRenderTime += chrono::duration<float>(chrono::high_resolution_clock::now() - renderStart).count();
      }
      sum += avgLabMaskedImgDiff(img, startImg, _freezeMask);
    }

    // style term
    sum += getStyleTerm(st, s, img);

    getGlobalSettings()->_timings[callingThreadId]._sampleTime += chrono::duration<float>(chrono::high_resolution_clock::now() - start).count();
    getGlobalSettings()->_timings[callingThreadId]._numSamples += 1;
    return sum;
  };

  // this function is not instrumented at the moment due it it only being used in LMGD, which is not
  // actively being looked at.
  _fsq = [this](Snapshot* s, int callingThreadId, Style st) {
    if (_active.size() == 0)
      return 0.0;

    Image img = _active.begin()->second.first->generateImage(s);

    // compute this on demand
    Image startImg;
    double sum = 0;
    for (const auto& kvp : _active) {
      if (kvp.second.second == A_LESS)
        sum += kvp.second.first->evaluateScene(s, img);
      else if (kvp.second.second == A_MORE) {
        // larger values = smaller function, LM expects things to be non-linear least squares,
        // which are all positive functions
        sum += (100 - kvp.second.first->evaluateScene(s, img));
      }
      else if (kvp.second.second == A_EQUAL) {
        if (startImg.getWidth() == 0) {
          Image startImg = _active.begin()->second.first->generateImage(_start);
        }
        sum += pow(kvp.second.first->evaluateScene(s, img) - kvp.second.first->evaluateScene(_start, startImg), 2);
      }
    }

    // freeze region penalty
    if (_useMask) {
      if (startImg.getWidth() == 0) {
        Image startImg = _active.begin()->second.first->generateImage(_start);
      }
      sum += avgLabMaskedImgDiff(img, startImg, _freezeMask);
    }

    sum += getStyleTerm(st, s, img);

    return sum;
  };

  generateEdits(false);

  auto& samples = getGlobalSettings()->_samples;
  samples.clear();
	_mode = getGlobalSettings()->_searchMode;
	_sharedData.clear();
	_sharedData["Edit Weight Status"] = 0;
	_sharedData["Initial Scene Run"] = 0;

  // init all threads
  int i = 0;
  for (auto& t : _threads) {
    t->setState(_start, _f, _fsq, getGlobalSettings()->_searchMode, _freezeMask);

    // set up diagnostics container
    samples[i] = vector<DebugData>();
    getGlobalSettings()->_timings[i] = { 0,0,0,0,0,0,0,0 };
    t->setInternalID(i);
    i++;
  }

  DebugData data;
  data._f = _f(start, -1, NO_STYLE);
  data._a = 1;
  data._sampleId = (unsigned int) samples[-1].size() + 1;
  data._editName = "START";
  data._accepted = true;
  data._scene = snapshotToVector(start);
  data._timeStamp = chrono::high_resolution_clock::now();
  samples[-1].push_back(data);

  setSessionName();
  getGlobalSettings()->_sessionSearchSettings = "";

  // record what the search params were
  for (const auto& attr : _active) {
    string attrStr = attr.first + " -> ";
    if (attr.second.second== A_LESS)
      attrStr = attrStr + "LESS";
    if (attr.second.second == A_MORE)
      attrStr = attrStr + "MORE";
    if (attr.second.second == A_EQUAL)
      attrStr = attrStr + "SAME";
    attrStr += "\n";

    getGlobalSettings()->_sessionSearchSettings += attrStr;
  }
}

void AttributeSearch::setState(Snapshot * start, vector<pair<GibbsScheduleConstraint, GibbsSchedule*>> sampler)
{
  if (_start != nullptr)
    delete _start;

  if (_threads.size() != getGlobalSettings()->_searchThreads)
    reinit();

  _activeSchedule = sampler;
  _start = start;

  auto& samples = getGlobalSettings()->_samples;
  samples.clear();
  _mode = getGlobalSettings()->_searchMode;
  _sharedData.clear();
  _sharedData["Edit Weight Status"] = 0;
  _sharedData["Initial Scene Run"] = 0;

  // init all threads
  int i = 0;
  for (auto& t : _threads) {
    t->setState(_start, _activeSchedule);

    // set up diagnostics container
    samples[i] = vector<DebugData>();
    getGlobalSettings()->_timings[i] = { 0,0,0,0,0,0,0,0 };
    t->setInternalID(i);
    i++;
  }

  DebugData data;
  data._sampleId = (unsigned int)samples[-1].size() + 1;
  data._editName = "START";
  data._accepted = true;
  data._scene = snapshotToVector(start);
  data._timeStamp = chrono::high_resolution_clock::now();
  samples[-1].push_back(data);

  setSessionName();
  getGlobalSettings()->_sessionSearchSettings = "";
}

void AttributeSearch::run()
{
  {
    MessageManagerLock mmlock(this);
    if (mmlock.lockWasGained()) {
      getStatusBar()->setStatusMessage("Started Attribute Search.");
    }
  }
  getRecorder()->log(SYSTEM, "Started Attribute Search.");
  getGlobalSettings()->_searchStartTime = chrono::high_resolution_clock::now();
  getGlobalSettings()->_searchAbsStartTime = chrono::system_clock::now();

  // run all threads
  // make sure to set the state properly before running/resuming
  for (auto& t : _threads) {
    t->startThread(1);
	}

  // idle until told to exit
  // or add other logic to control search path/execution
  while (1) {
    wait(100);

    if (threadShouldExit()) {
      getGlobalSettings()->_searchEndTime = chrono::high_resolution_clock::now();
      return;
    }

    if (_viewer->isFull()) {
      // clear new results queue, forcefully
      {
        MessageManagerLock mmlock(this);
        if (mmlock.lockWasGained()) {
          // reach into the main component and call the function
          MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

          if (mc != nullptr) {
            mc->showNewResults();
          }
        }
      }

      for (auto& t : _threads)
        t->stopThread(5000);
      signalThreadShouldExit();
    }
    else {
      MessageManagerLock mmlock(this);
      if (mmlock.lockWasGained()) {
        getApplicationCommandManager()->invokeDirectly(command::GET_NEW_RESULTS, false);
      }
    }
  }
}

void AttributeSearch::stop()
{
  for (auto& t : _threads) {
    if (t->isThreadRunning())
      t->stopThread(5000);
  }

  stopThread(5000);
}

void AttributeSearch::generateEdits(bool /* explore */)
{
  // here we dynamically create all of the edits used by the search algorithm for all
  // levels of the search. This is the function to change if we want to change
  // how the search goes through the lighting space.
  auto& edits = getGlobalSettings()->_edits;
	getGlobalSettings()->_globalEditWeights.clear();
  edits.clear();
  _lockedParams.clear();

  // in order for a parameter to be autolocked, it must be present in the autolock list
  // for all active attributes
  map<EditParam, int> lockedParams;
  for (auto& a : _active) {
    for (auto& p : a.second.first->_autoLockParams) {
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
  vector<string> deviceIds = getRig()->getAllDevices().getIds();

  // Create all devices edit types
  generateDefaultEdits("*", 1);
  //generateColorEdits("*");

  // Create edits for each system
  for (const auto& s : systems) {
    generateDefaultEdits(s, 3);
  }

  // Create edits for each area
  for (const auto& a : areas) {
    generateDefaultEdits(a, 2);
    // color edits are not final or even really implemented yet...
    // generateColorEdits(a);
  }

  // if we're doing the experimental gibbs sampler, add the gibbs edit
  if (getGlobalSettings()->_editSelectMode == DIRECTED_SAMPLING) {
    GibbsEdit* g = new GibbsEdit(_lockedParams);
    g->setAffected("system", INTENSITY);
    if (g->canDoEdit()) {
      g->_name = "Directed Gibbs Sample";
      edits.push_back(g);
    }
  }

  // pivot edits (all lights except specified system)?
  // not sure if necessary

  // Special edit types
  // left blank for now.
  // may be used for user specified edits? May do cross-system/area edits?
}

void AttributeSearch::generateDefaultEdits(string select, int editType)
{
  // count the number of devices, some edits may not be useful for selections
  // of just single devices
  string query;
  if (editType == 1)
    query = select;
  else if (editType == 2)
    query = "$area=" + select;
  else if (editType == 3)
    query = "$system=" + select;

  auto devices = getRig()->select(query).getDevices();

  auto& edits = getGlobalSettings()->_edits;

  // set init function
  auto initfunc = [=](Edit* e, bool joint, bool uniform) {
    if (editType == 1)
      e->initArbitrary(select, joint, uniform);
    else if (editType == 2)
      e->initWithArea(select, joint, uniform);
    else if (editType == 3)
      e->initWithSystem(select, joint, uniform);
  };

  Edit* e;
  // all parameters
  if (getGlobalSettings()->_editSelectMode != DIRECTED_SAMPLING) {
    e = new Edit(_lockedParams);
    e->setParams({ INTENSITY, HUE, SAT, VALUE });
    e->_name = select + "_all";
    initfunc(e, false, false);
    if (e->canDoEdit() && !isDuplicateEdit(e))
      edits.push_back(e);
    else
      delete e;

    // intensity/brightness only
    e = new Edit(_lockedParams);
    e->setParams({ INTENSITY });
    e->_name = select + "_intensity";
    initfunc(e, false, false);
    //if (e->canDoEdit())
    //  edits.push_back(e);
    //else
    delete e;
  }

  // hue only
  e = new Edit(_lockedParams);
  e->setParams({ HUE });
  e->_name = select + "_hue";
  initfunc(e, false, false);
  if (e->canDoEdit() && !isDuplicateEdit(e))
    edits.push_back(e);
  else
    delete e;

  // sat only
  e = new Edit(_lockedParams);
  e->setParams({ SAT });
  e->_name = select + "_sat";
  initfunc(e, false, false);
  if (e->canDoEdit() && !isDuplicateEdit(e))
    edits.push_back(e);
  else
    delete e;
}

bool AttributeSearch::isDuplicateEdit(Edit * e)
{
  for (auto ed : getGlobalSettings()->_edits) {
    if (ed->isEqual(*e))
      return true;
  }

  return false;
}
