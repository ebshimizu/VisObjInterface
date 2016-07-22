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
#include <list>
#include <algorithm>

// Search functions
// ==============================================================================
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

  // put first element in to results vector, ignore the rest
  list<SearchResult*> filteredResults;
  for (auto c : res) {
    bool first = true;
    for (auto kvp : c) {
      if (first) {
        filteredResults.push_back(kvp.second);
        first = false;
      }
      else {
        // skip
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

		d.second->getParam<LumiverseColor>("color")->setColorChannel("Red", source[base + 3]);
		d.second->getParam<LumiverseColor>("color")->setColorChannel("Green", source[base + 4]);
		d.second->getParam<LumiverseColor>("color")->setColorChannel("Blue", source[base + 5]);

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

//=============================================================================

AttributeSearchThread::AttributeSearchThread(String name, SearchResultsViewer* viewer, map<string, int>& sharedData) :
	Thread(name), _viewer(viewer), _edits(vector<Edit*>()), _sharedData(sharedData)
{
  _original = nullptr;
}

AttributeSearchThread::~AttributeSearchThread()
{
  // don't delete anything, other objects need the results allocated here.
  // except for internally used variables only
  delete _original;

  //for (auto e : _edits)
  //  delete e;
}

void AttributeSearchThread::setState(Snapshot * start, attrObjFunc & f, SearchMode m)
{
  if (_original != nullptr)
    delete _original;

  _original = new Snapshot(*start);
  _edits.clear();
  _edits = getGlobalSettings()->_edits;
  _T = getGlobalSettings()->_T;
  _maxDepth = (getGlobalSettings()->_standardMCMC) ? 1 : getGlobalSettings()->_editDepth;
  _failures = 0;

  _f = f;
	_mode = m;
	_randomInit = false;
	_status = IDLE;
}

void AttributeSearchThread::run()
{
  //checkEdits();
  //return;

  // here we basically want to run the search indefinitely until cancelled by user
  // so we just call the actual search function over and over
  while (1) {    
    runSearch();

    if (threadShouldExit())
      return;
  }
}

void AttributeSearchThread::computeEditWeights()
{
	// Here we take the starting scene and evaluate the variance of all available edits
	// when performed on the starting scene. The idea is to measure how much each edit
	// "matters" and use that to bias local searches towards meaningfuly different changes

	// NOTE: Right now the weights are computed globally, as in for one particular scene
	// in the future we may want to maintain separate weights for each scene

	Eigen::VectorXd weights;
	weights.resize(_edits.size());

	for (int i = 0; i < _edits.size(); i++) {
		double weight = sqrt(_edits[i]->variance(_original, _f, getGlobalSettings()->_editStepSize * 3, 50));

    // minimum weight, prevent conflicts and also enable all edits to possibly be chosen
    weights[i] = (weight < 1e-3) ? 1e-3 : weight;
	}

	// normalize to [0,1] and update weights
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

void AttributeSearchThread::setStartConfig(Snapshot * start)
{
	if (_original != nullptr)
		delete _original;

	_original = new Snapshot(*start);
	_acceptedSamples = 0;
}

void AttributeSearchThread::runHybridDebug()
{
	setSessionName();
	ofstream hybridLog;
	hybridLog.open(getGlobalSettings()->_logRootDir + "/traces/" + getGlobalSettings()->_sessionName + "-debug.txt", ios::app);

	// here we want to test whether or not local samples around a local optimum
	// fall back to that local optimum value.

	// first, compute edit weights
	computeEditWeights();

	// get descent scene, forcibly add scene to results container
	runLMGDSearch(true);

	// ok now we need to find that scene. It should be the most recently added
	// to the terminal set
	map<int, shared_ptr<SearchResultContainer> >& lmterminals = _viewer->getTerminalScenes();
	shared_ptr<SearchResultContainer> target = lmterminals.rbegin()->second;

	// update the starting config. ok to overwrite here, we'll randomize start scene after first run
	Snapshot* newStart = vectorToSnapshot(target->getSearchResult()->_scene);

	// Check the descent n times
	// here n is 10 because magic numbers are great
	for (int i = 0; i < 10; i++) {
		if (threadShouldExit())
			return;

		setStartConfig(newStart); // creates duplicate

		setParent(target->getSearchResult()->_sampleNo);

		// Find a random scene around the target
		runMCMCEditSearch(true);

		// Retrieve scene. There's a special function for this
		shared_ptr<SearchResultContainer> last = _viewer->getLastSample();
		
		// Run LMGD from the last sample's position
		Snapshot* lastStart = vectorToSnapshot(target->getSearchResult()->_scene);
		setStartConfig(lastStart);
		delete lastStart;

		useRandomInit(false);
		runLMGDSearch(true);

		// get the result
		shared_ptr<SearchResultContainer> LMGDRedo = lmterminals.rbegin()->second;

		// compare search from random sample to target
		double dist = (target->getSearchResult()->_scene - LMGDRedo->getSearchResult()->_scene).norm();

		// output
		hybridLog << "Local Sample " << last->getSearchResult()->_sampleNo << " f(x) = " << last->getSearchResult()->_objFuncVal << "\n";
		hybridLog << "Distance: " << dist << " (" << target->getSearchResult()->_sampleNo << "," << last->getSearchResult()->_sampleNo << ")\n";

		// repeat
	}

	hybridLog.close();
}

void AttributeSearchThread::runSearch()
{
	if (_mode == MCMC_EDIT)
		runMCMCEditSearch();
	else if (_mode == LM_GRAD_DESCENT)
		runLMGDSearch();
	else if (_mode == HYBRID_EXPLORE) {
		// This search mode has multiple phases, and each thread can be on a different phase
		// On thread creation, we should wait for further instructions
		if (_status == IDLE)
			wait(-1);

		if (_status == EXPLORE) {
			runLMGDSearch();
		}
		else if (_status == EXPLOIT) {
			if (_acceptedSamples > 25) {
				// stop
				_status = IDLE;
			}
			else {
				runMCMCEditSearch();
			}
		}
		else if (_status == EDIT_WEIGHTS) {
			computeEditWeights();
		}
	}
	else if (_mode == HYBRID_DEBUG) {
		runHybridDebug();
	}
}

void AttributeSearchThread::runMCMCEditSearch(bool force)
{
	// assign start scene, initialize result
	Snapshot* start = new Snapshot(*_original);
	SearchResult* r = new SearchResult();
	double fx = _f(start);
	double orig = fx;

	// RNG
	default_random_engine gen(std::random_device{}());
	uniform_real_distribution<double> udist(0.0, 1.0);

	// do the MCMC search
	int depth = 0;
	Edit* e = nullptr;

	// magic number alert
	int iters = getGlobalSettings()->_standardMCMC ? getGlobalSettings()->_standardMCMCIters : getGlobalSettings()->_maxMCMCIters;

	// depth increases when scenes are rejected from the viewer
	while (depth < _maxDepth) {
		if (threadShouldExit()) {
			delete r;
			delete start;
			return;
		}

		//  pick a next plausible edit
    if (r->_editHistory.size() == 0)
      e = _edits[0]->getNextEdit(r->_editHistory, getGlobalSettings()->_globalEditWeights, getGlobalSettings()->_reduceRepeatEdits);
    else
      e = e->getNextEdit(r->_editHistory, getGlobalSettings()->_globalEditWeights, getGlobalSettings()->_reduceRepeatEdits);

		r->_editHistory.push_back(e);

		// iterate a bit, do a little bit of searching
		// at this point it's probably better to just do gradient descent, but for testing
		// we'll simulate this by doing some mcmc iterations
		Eigen::VectorXd minScene;

		// do the adjustment until acceptance
		for (int i = 0; i < iters; i++) {
			if (threadShouldExit()) {
				delete r;
				delete start;
				return;
			}

			//  adjust the starting scene
			Snapshot* sp = new Snapshot(*start);
			e->performEdit(sp, getGlobalSettings()->_editStepSize);

			// check for acceptance
			double fxp = _f(sp);
			double a = 0;
			
			if (_mode == MCMC_EDIT) {
				a = min(exp((1 / _T) * (fx - fxp)), 1.0);
			}
			else if (_mode == HYBRID_EXPLORE || _mode == HYBRID_DEBUG) {
				// here is the main difference between this search and the normal edit search:
				// we allow a much larger range of motion by loosening the temperature parameter
				// for this section
				a = min(exp((1 / (_T * 0.25)) * (fx - fxp)), 1.0);
			}

			// accept if a >= 1 or with probability a
			if (a >= 1 || udist(gen) < a) {
				// update x
				delete start;
				start = sp;
				fx = fxp;

				// update result
				r->_objFuncVal = fx;

				// diagnostics
				if (!getGlobalSettings()->_standardMCMC) {
					DebugData data;
					auto& samples = getGlobalSettings()->_samples;
					data._f = r->_objFuncVal;
					data._a = a;
					data._sampleId = (unsigned int) samples[_id].size() + 1;
					data._editName = e->_name;
					data._accepted = true;
					data._scene = snapshotToVector(sp);
					samples[_id].push_back(data);
				}

				// break after acceptance
				//break;
			}
			else {
				delete sp;
			}
			//sleep(10);
		}
		depth++;
	}

	r->_scene = snapshotToVector(start);
	delete start;

	// diagnostics
	DebugData data;
	auto& samples = getGlobalSettings()->_samples;
	data._f = r->_objFuncVal;
	data._a = 1;
	data._sampleId = (unsigned int) samples[_id].size() + 1;
	data._editName = "TERMINAL";
	data._accepted = true;
	data._scene = r->_scene;
	
	r->_extraData["Thread"] = String(_id);

	if (_mode == HYBRID_EXPLORE || _mode == HYBRID_DEBUG) {
		r->_extraData["Parent"] = String(_parent);
	}

	// add if we did better
	// hybrid method doesn't actually care just add it anyway
	if (r->_objFuncVal < orig || _mode == HYBRID_EXPLORE || _mode == HYBRID_DEBUG) {
		// send scene to the results area. may chose to not use the scene
		if (!_viewer->addNewResult(r, force)) {
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
		delete r;
	}

	samples[_id].push_back(data);
}

void AttributeSearchThread::runLMGDSearch(bool force)
{
	// Levenberg-Marquardt
	// It's a bit questionable whether or not this formulation of the optimization problem
	// actually fits the non-linear least squares method, however
	// we're just gonna do it and see what happens.

	Snapshot xs(*_original);
	Eigen::VectorXd x = snapshotToVector(&xs);

	if (_randomInit) {
		// RNG
		default_random_engine gen(std::random_device{}());
		uniform_real_distribution<double> udist(0.0, 1.0);

		for (int i = 0; i < x.size(); i++) {
			x[i] = udist(gen);
		}
		vectorToExistingSnapshot(x, xs);
		x = snapshotToVector(&xs);
	}

	int maxIters = getGlobalSettings()->_maxGradIters;
	double nu = 2;
	double eps = 1e-3;
	double tau = 1e-3;
	double fx = _f(&xs);
	double forig = fx;

	Eigen::MatrixXd J = getJacobian(xs);
	Eigen::MatrixXd H = J.transpose() * J;		// may need to replace with actual calculation of Hessian
	Eigen::MatrixXd g = J.transpose() * _f(&xs);

	double mu = tau * H.diagonal().maxCoeff();

	bool found = false;
	int k = 0;

	while (!found && k < maxIters) {
		if (threadShouldExit()) {
			return;
		}

		k = k + 1;
		Eigen::MatrixXd inv = (H + mu * Eigen::MatrixXd::Identity(H.rows(), H.cols())).inverse();
		Eigen::MatrixXd hlm = -inv * g;

		if (hlm.norm() <= eps * (x.norm() + eps)) {
			found = true;
		}
		else {
			Eigen::VectorXd xnew = x + hlm;
			
			// fix constraints, bounded at 1 and 0
			for (int i = 0; i < x.size(); i++) {
				xnew[i] = clamp((float) xnew[i], 0, 1);
			}

			vectorToExistingSnapshot(xnew, xs);
			double fnew = _f(&xs);
			Eigen::MatrixXd rho = (0.5 * hlm.transpose() * (mu * hlm - g)) * (1 / (fx * fx / 2 - fnew * fnew / 2));

			// should be a scalar
			double rhoval = rho(0);

			if (rhoval > 0) {
				// acceptable step
				x = xnew;
				J = getJacobian(xs);
				g = J.transpose() * fnew;
				found = (g.norm() <= eps);
				mu = mu * max(1.0 / 3.0, 1 - pow(2 * rhoval - 1, 3));
				nu = 2;
				fx = fnew;

				// put the result in the visible set for debugging
				//SearchResult* r = new SearchResult();
				//r->_objFuncVal = fnew;
				//r->_scene = x;
				//_viewer->addNewResult(r);

				// debug data
				DebugData data;
				auto& samples = getGlobalSettings()->_samples;
				data._f = fnew;
				data._a = 1;
				data._sampleId = (unsigned int) samples[_id].size() + 1;
				data._editName = "L-M DESCENT STEP";
				data._accepted = true;
				data._scene = x;
				samples[_id].push_back(data);
			}
			else {
				mu = mu * nu;
				nu = 2 * nu;

				// reset
				vectorToExistingSnapshot(x, xs);
			}
		}
	}

	// found, stick in visible set if actually better

	// debug data
	DebugData data;
	auto& samples = getGlobalSettings()->_samples;
	data._f = fx;
	data._a = 1;
	data._sampleId = (unsigned int) samples[_id].size() + 1;
	data._editName = "L-M TERMINAL";
	data._scene = x;
	samples[_id].push_back(data);

	if (fx < forig) {
		SearchResult* r = new SearchResult();
		r->_objFuncVal = _f(&xs);
		r->_scene = x;
		r->_extraData["LM Terminal"] = "True";
		r->_extraData["Thread"] = String(_id);
		// r->_extraData["Parent"]

		data._accepted = _viewer->addNewResult(r, force);
	}
	else {
		data._accepted = false;
	}

	_status = IDLE;

	if (_mode == LM_GRAD_DESCENT || _mode == HYBRID_DEBUG) {
		_randomInit = true;
	}
}

void AttributeSearchThread::checkEdits()
{
  // RNG
  default_random_engine gen(std::random_device{}());
  uniform_real_distribution<double> udist(0.0, 1.0);

  for (auto e : _edits) {
    // assign start scene, initialize result
    Snapshot* start = new Snapshot(*_original);
    SearchResult* r = new SearchResult();
    double fx = _f(start);
    double orig = fx;

    // magic number alert
    int iters = getGlobalSettings()->_standardMCMC ? getGlobalSettings()->_standardMCMCIters : getGlobalSettings()->_maxMCMCIters;

    // No depth counter here, just one edit
    if (threadShouldExit()) {
      delete r;
      delete start;
      return;
    }

    r->_editHistory.push_back(e);

    // iterate a bit, do a little bit of searching
    // at this point it's probably better to just do gradient descent, but for testing
    // we'll simulate this by doing some mcmc iterations
    Eigen::VectorXd minScene;

    // do the adjustment until acceptance
    for (int i = 0; i < iters; i++) {
      if (threadShouldExit()) {
        delete r;
        delete start;
        return;
      }

      //  adjust the starting scene
      Snapshot* sp = new Snapshot(*start);
      e->performEdit(sp, getGlobalSettings()->_editStepSize);

      // check for acceptance
      double fxp = _f(sp);
      double a = min(exp((1 / _T) * (fx - fxp)), 1.0);

      // accept if a >= 1 or with probability a
      if (a >= 1 || udist(gen) < a) {
        // update x
        delete start;
        start = sp;
        fx = fxp;

        // update result
        r->_objFuncVal = fx;

        // diagnostics
        if (!getGlobalSettings()->_standardMCMC) {
          DebugData data;
          auto& samples = getGlobalSettings()->_samples;
          data._f = r->_objFuncVal;
          data._a = a;
          data._sampleId = (unsigned int) samples[_id].size() + 1;
          data._editName = e->_name;
          data._accepted = true;
          data._scene = snapshotToVector(sp);
          samples[_id].push_back(data);
        }
      }
      else {
        delete sp;
      }
    }

    r->_scene = snapshotToVector(start);
    delete start;

    // diagnostics
    DebugData data;
    auto& samples = getGlobalSettings()->_samples;
    data._f = r->_objFuncVal;
    data._a = 1;
    data._sampleId = (unsigned int) samples[_id].size() + 1;
    data._editName = "TERMINAL";
    data._accepted = true;
    data._scene = r->_scene;

    // add if we did better
    if (r->_objFuncVal < orig) {
      // send scene to the results area. may chose to not use the scene
      if (!_viewer->addNewResult(r)) {
        // r has been deleted by _viewer here
        _failures++;
        data._accepted = false;

        if (_failures > getGlobalSettings()->_searchFailureLimit) {
          _failures = 0;
          _maxDepth++;
        }
      }
    }
    else {
      data._accepted = false;
      delete r;
    }

    samples[_id].push_back(data);
  }
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

Eigen::VectorXd AttributeSearchThread::getDerivative(Snapshot & s)
{
	Eigen::VectorXd x = snapshotToVector(&s);
	Eigen::VectorXd dx;
	dx.resizeLike(x);
	double fx = _f(&s);

	// adjust each parameter in order
	double h = 1e-3;

	for (int i = 0; i < dx.size(); i++) {
		// we're bounded by some physical restrictions here, however, they get
		// partially addressed by the vector to snapshot function
		if (x[i] >= 1) {
			x[i] -= h;
			vectorToExistingSnapshot(x, s);
			double fxp = _f(&s);
			dx[i] = (fx - fxp) / h;
			x[i] += h;
		}
		else {
			x[i] += h;
			vectorToExistingSnapshot(x, s);
			double fxp = _f(&s);
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

  _active = active;
  _start = start;

  // objective function for combined set of active attributes.
  if (getGlobalSettings()->_searchMode == MCMC_EDIT) {
    _f = [this](Snapshot* s) {
      double sum = 0;
      for (const auto& kvp : _active) {
        if (kvp.second->getStatus() == A_LESS)
          sum += kvp.second->evaluateScene(s);
        else if (kvp.second->getStatus() == A_MORE) {
          sum -= kvp.second->evaluateScene(s);
        }
        else if (kvp.second->getStatus() == A_EQUAL)
          sum += pow(kvp.second->evaluateScene(s) - kvp.second->evaluateScene(_start), 2);
      }

      return sum;
    };
  }
  else {
    _f = [this](Snapshot* s) {
      double sum = 0;
      for (const auto& kvp : _active) {
        if (kvp.second->getStatus() == A_LESS)
          sum += kvp.second->evaluateScene(s);
        else if (kvp.second->getStatus() == A_MORE) {
          // larger values = smaller function, LM expects things to be non-linear least squares,
          // which are all positive functions
          sum += (100 - kvp.second->evaluateScene(s));
        }
        else if (kvp.second->getStatus() == A_EQUAL)
          sum += pow(kvp.second->evaluateScene(s) - kvp.second->evaluateScene(_start), 2);
      }

      return sum;
    };
  }

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
    t->setState(_start, _f, getGlobalSettings()->_searchMode);

    // set up diagnostics container
    samples[i] = vector<DebugData>();
    t->setInternalID(i);
    i++;
  }

  DebugData data;
  data._f = _f(start);
  data._a = 1;
  data._sampleId = (unsigned int) samples[-1].size() + 1;
  data._editName = "START";
  data._accepted = true;
  data._scene = snapshotToVector(start);
  samples[-1].push_back(data);

  setSessionName();
  getGlobalSettings()->_sessionSearchSettings = "";

  // record what the search params were
  for (const auto& attr : _active) {
    string attrStr = attr.first + " -> ";
    if (attr.second->getStatus() == A_LESS)
      attrStr = attrStr + "LESS";
    if (attr.second->getStatus() == A_MORE)
      attrStr = attrStr + "MORE";
    if (attr.second->getStatus() == A_EQUAL)
      attrStr = attrStr + "SAME";
    attrStr += "\n";

    getGlobalSettings()->_sessionSearchSettings += attrStr;
  }
}

void AttributeSearch::run()
{
  // run all threads
  // make sure to set the state properly before running/resuming
	if (_mode == MCMC_EDIT) {
		// compute edit weights first before starting
		_threads[0]->computeEditWeights();
	}

	if (_mode == HYBRID_DEBUG) {
		// Force single-threaded mode for debug. Much slow. Very debug.
		_threads[0]->startThread(1);
	}
	else {
		for (auto& t : _threads) {
			t->startThread(1);
		}
	}

  // idle until told to exit
  // or add other logic to control search path/execution
  while (1) {
    wait(100);

    if (threadShouldExit())
      return;

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

		// Hybrid dispatching
		// basically here we just want to make sure we precompute everything we need
		// before jumping in to the whole algorithm
		// may want to move this outside the while loop in that case.
		if (_mode == HYBRID_EXPLORE) {
			// find an idle thread
			for (auto& t : _threads) {
				if (t->getState() == IDLE) {
					// compute the weights
					if (_sharedData["Edit Weight Status"] == 0) {
						t->setState(EDIT_WEIGHTS);
						_sharedData["Edit Weight Status"] = 1;
					}
					// run LM on starting scene
					else if (_sharedData["Initial Scene Run"] == 0) {
						// initial scene already set
						t->setState(EXPLORE);
						_sharedData["Initial Scene Run"] = 1;
					}
					// run LM on random scene if weights not finished
					else if (_sharedData["Initial Scene Run"] == 1 && _sharedData["Edit Weight Status"] == 1) {
						t->useRandomInit(true);
						t->setState(EXPLORE);
					}
					else {
						// after all the initialization stuff, do things based off of ratios of completed scenes
						// Generally, after each explore step the thread does an exploit step.
						// however, we also want to explore the local area around the starting scene too.
						auto& terminal = _viewer->getTerminalScenes();
						auto& counts = _viewer->getLocalSampleCounts();

						// TODO: Parameterize these magic numbers
						if (counts.count(0) == 0) {
							t->setStartConfig(_start);
							t->setParent(0);
							t->setState(EXPLOIT);
							counts[0] = 0;
							t->notify();
							break;
						}

						bool terminalFound = false;
						// find a terminal scene that doesnt have any local scenes
						for (auto& kvp : terminal) {
							if (counts.count(kvp.first) == 0) {
								t->setState(EXPLOIT);
								t->setParent(kvp.first);
								t->setStartConfig(vectorToSnapshot(kvp.second->getSearchResult()->_scene));
								terminalFound = true;
								counts[kvp.first] = 0;
								break;
							}
						}

						if (!terminalFound) {
							// otherwise run a default explore -> exploit loop
							t->useRandomInit(true);
							t->setState(EXPLORE);
						}
					}
					t->notify();
				}
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

  if (!getGlobalSettings()->_standardMCMC) {
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

    // edits for individual devices?
  }
  else {
    Edit* e = new Edit(_lockedParams);
    e->initArbitrary("*", false, false);
    set<EditParam> params;
    params.insert(EditParam::INTENSITY);
    params.insert(EditParam::RED);
    params.insert(EditParam::GREEN);
    params.insert(EditParam::BLUE);
    e->setParams(params);
    e->_name = "*";
    getGlobalSettings()->_edits.push_back(e);
  }

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

  // all parameters
  Edit* e = new Edit(_lockedParams);
  e->setParams({ INTENSITY, RED, BLUE, GREEN });
  e->_name = select + "_all";
  initfunc(e, false, false);
  if (e->canDoEdit())
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

  // hue only
  e = new Edit(_lockedParams);
  e->setParams({ HUE });
  e->_name = select + "_hue";
  initfunc(e, false, false);
  if (e->canDoEdit())
    edits.push_back(e);
  else
    delete e;

  // sat only
  e = new Edit(_lockedParams);
  e->setParams({ SAT });
  e->_name = select + "_sat";
  initfunc(e, false, false);
  if (e->canDoEdit())
    edits.push_back(e);
  else
    delete e;
}