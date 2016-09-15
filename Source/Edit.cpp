/*
  ==============================================================================

    AttributeEdit.cpp
    Created: 12 May 2016 10:57:22am
    Author:  falindrith

  ==============================================================================
*/

#include "Edit.h"
#include "AttributeSearch.h"

ConsistencyConstraint::ConsistencyConstraint(DeviceSet affected, ConsistencyScope scope, set<EditParam> params) :
  _affected(affected), _scope(scope), _params(params)
{
}

ConsistencyConstraint::ConsistencyConstraint(string query, ConsistencyScope scope, set<EditParam> params) :
  _scope(scope), _params(params)
{
  _affected = getRig()->select(query);
}

ConsistencyConstraint::ConsistencyConstraint(const ConsistencyConstraint & other) :
  _affected(other._affected), _scope(other._scope), _params(other._params)
{
}

ConsistencyConstraint::ConsistencyConstraint()
{
}

ConsistencyConstraint::~ConsistencyConstraint()
{

}

bool ConsistencyConstraint::contains(Device * d)
{
  return _affected.contains(d->getId());
}

Edit::Edit(set<EditParam> lockedParams) : _lockedParams(lockedParams)
{
  _joint = false;
  _uniform = false;

  _gen = default_random_engine(std::random_device{}());
  _udist = uniform_real_distribution<double>(0.0, 1.0);
}

Edit::~Edit()
{
}

void Edit::initWithArea(string area, bool joint, bool uniform)
{
  string query = "$area=" + area;
	initArbitrary(query, joint, uniform);
}

void Edit::initWithSystem(string system, bool joint, bool uniform)
{
  string query = "$system=" + system;
	initArbitrary(query, joint, uniform);
}

void Edit::initArbitrary(string query, bool joint, bool uniform)
{
  _joint = joint;
  _uniform = uniform;
  _affectedDevices = getRig()->select(query);
	constructConsistencySets();
}

void Edit::setParams(set<EditParam> affectedParams)
{
  _affectedParams = affectedParams;
}

void Edit::performEdit(Snapshot * s, double stepSize)
{
  auto& devices = _affectedDevices.getDevices();
  auto& sd = s->getRigData();
  _gdist = normal_distribution<double>(0, stepSize);  // start with sdev 2

  if (_uniform) {
    // rather than randomly selecting a starting point, uniform will first
    // set all parameters to the minimum value that it finds
    double min = DBL_MAX;
    for (const auto& d : devices) {
      for (const auto& p : _affectedParams) {
        double val = getParam(sd[d->getId()], p);

        if (val < min)
          min = val;
      }
    }

    double val = min + _gdist(_gen);

    for (const auto& d : devices) {
      for (const auto& p : _affectedParams) {
        setParam(sd[d->getId()], p, val);
      }
    }
  }
  else if (_joint) {
    double delta = _gdist(_gen);

    for (const auto& d : devices) {
      for (const auto& p : _affectedParams) {
        applyParamEdit(sd[d->getId()], p, delta);
      }
    }
  }
  else {
    for (auto cs : _consistencySets) {
			Device* rep = sd[cs.second];
			set<Device*> devs = cs.first._affected.getDevices();

      for (auto& p : cs.first._params) {
				double delta = _gdist(_gen);
				applyParamEdit(rep, p, delta);

				// enforce consistency
				// admittedly this isnt the most efficient way, but it's logically what's happening
				for (auto& d : devs) {
					Device* snapshotDev = sd[d->getId()];
					if (snapshotDev == rep)
						continue;

          if (p == HUE || p == SAT || p == VALUE) {
            setParam(snapshotDev, HUE, getParam(rep, HUE));
            setParam(snapshotDev, SAT, getParam(rep, SAT));
            setParam(snapshotDev, VALUE, getParam(rep, VALUE));
          }
          else {
            setParam(snapshotDev, p, getParam(rep, p));
          }
				}
      }
    }
  }
}

Edit * Edit::getNextEdit(Snapshot* /* current */, attrObjFunc f, vector<Edit*>& /* editHistory */, vector<Edit*>& availableEdits)
{
  // select a random edit
  int idx = (int) (_udist(_gen) * availableEdits.size());
  return availableEdits[idx];
}

Edit * Edit::getNextEdit(vector<Edit*>& editHistory, map<double, Edit*>& weights, bool useHistory)
{
	// weighted selection
  if (!useHistory || editHistory.size() == 0) {
    return weights.lower_bound(_udist(_gen))->second;
  }
  else {
    // decrease likelihood of edits being repeated.
    // reconstruct original weights
    map<Edit*, double> w;
    double prev = 0;
    for (auto it = weights.begin(); it != weights.end(); it++) {
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

    return reweights.lower_bound(_udist(_gen))->second;
  }
}

bool Edit::canDoEdit()
{
  bool allLocked = true;

  // Can do the edit if at least one param is unlocked
  auto& devices = _affectedDevices.getDevices();
  for (auto d : devices) {
    for (auto& p : _affectedParams) {
      allLocked = allLocked & isParamLocked(d, p);
    }
  }

  return !allLocked;
}

Eigen::VectorXd Edit::numericDeriv(Snapshot * s, attrObjFunc f, double fx)
{
  Snapshot dx(*s);
  double h = getGlobalSettings()->_searchDerivDelta;

  // For normal edits: vector size should be number of affected devices * affected parameters
  // For uniform and joint edits, the vector will be size 1
  // for devices with locked parameters, the derivative will be 0
  Eigen::VectorXd deriv;

  auto& devices = _affectedDevices.getDevices();
  auto& tempRigDevices = dx.getRigData();

  if (_uniform) {
    deriv.resize(1);
    // do the uniform normalization step (clamp to min)
    double min = DBL_MAX;
    for (const auto& d : devices) {
      for (const auto& p : _affectedParams) {
        double val = getParam(tempRigDevices[d->getId()], p);

        if (val < min)
          min = val;
      }
    }

    // record the new function value
    fx = f(&dx);

    // set the values
    for (const auto& d : devices) {
      for (const auto& p : _affectedParams) {
        applyParamEdit(tempRigDevices[d->getId()], p, h);
      }
    }

    // get new function val
    double fxp = f(&dx);

    // calculate deriv
    deriv[0] = (fxp - fx) / h;
  }
  else if (_joint) {
    deriv.resize(1);

    // adjust all devices at once
    for (auto d : devices) {
      for (auto& p : _affectedParams) {
        // perturb by small amount
        applyParamEdit(tempRigDevices[d->getId()], p, h);
      }
    }

    // get new function val
    double fxp = f(&dx);

    // calculate deriv
    deriv[0] = (fxp - fx) / h;
  }
  else {
    deriv.resize(_affectedDevices.size() * _affectedParams.size());
    int i = 0;

    for (auto d : devices) {
      for (auto& p : _affectedParams) {
        // perturb by small amount
        applyParamEdit(tempRigDevices[d->getId()], p, h);

        // get new function val
        double fxp = f(&dx);

        // calculate deriv
        deriv[i] = (fxp - fx) / h;

        // reset
        applyParamEdit(tempRigDevices[d->getId()], p, -h);
        i++;
      }
    }
  }

  return deriv;
}

double Edit::variance(Snapshot * s, attrObjFunc f, double radius, int n)
{
	// take n samples using step size radius
	Eigen::VectorXd samples;
	samples.resize(n);
	Eigen::VectorXd start = snapshotToVector(s);

	// do the edit n times, record value
	for (int i = 0; i < n; i++) {
		performEdit(s, radius);
		samples[i] = f(s);
		vectorToExistingSnapshot(start, *s); // loading from vector probably faster than reallocating a new one
	}

	// compute sample variance
	double mean = samples.mean();
	Eigen::VectorXd shifted = (samples - mean * Eigen::VectorXd::Ones(n));
	shifted = shifted.cwiseProduct(shifted);
	return shifted.sum() / n;
}

double Edit::expected(Snapshot * s, attrObjFunc f, double radius, int n)
{
  double fstart = f(s);

	// take n samples using step size radius
	Eigen::VectorXd samples;
	samples.resize(n);
	Eigen::VectorXd start = snapshotToVector(s);

  int numPos = 0;
  double sum = 0;

	// do the edit n times, record difference from starting value
	for (int i = 0; i < n; i++) {
		performEdit(s, radius);
		samples[i] = fstart - f(s);
		
    if (samples[i] > 0) {
      numPos++;
      sum += samples[i];
    }
    
    vectorToExistingSnapshot(start, *s); // loading from vector probably faster than reallocating a new one
	}

	// compute sample variance
	double mean = samples.mean();
	Eigen::VectorXd shifted = (samples - mean * Eigen::VectorXd::Ones(n));
	shifted = shifted.cwiseProduct(shifted);
	double var = shifted.sum() / n;

  // compute ratio of positive changes and sum up total of those changes
  // multiply ratio by average change
  double ev = (sum * numPos) / n;

  double weight = getGlobalSettings()->_evWeight;

  return var * (1 - weight) + ev * weight;
}

double Edit::proportionGood(Snapshot * s, attrObjFunc f, double startVal, double radius, int n, Eigen::VectorXd & minScene, double & minVal)
{
  Eigen::VectorXd start = snapshotToVector(s);
  minVal = DBL_MAX;
  int good = 0;

  for (int i = 0; i < n; i++) {
    performEdit(s, radius);
    double fx = f(s);

    if (fx < minVal) {
      minVal = fx;
      minScene = snapshotToVector(s);
    }

    if (fx < startVal)
      good++;

    vectorToExistingSnapshot(start, *s);
  }

  return (double)good / n;
}


bool Edit::isEqual(Edit & other)
{
  // edits are equal if they affect the same devices and the same parameters
  // check params first
  set<EditParam> otherParams = set<EditParam>(other._affectedParams);

  if (otherParams.size() != _affectedParams.size())
    return false;

  for (auto p : _affectedParams) {
    otherParams.erase(p);
  }

  if (otherParams.size() != 0)
    return false;

  // check devices
  if (_affectedDevices.hasSameIds(other._affectedDevices)) {
    return true;
  }
  else {
    return false;
  }
}

void Edit::applyParamEdit(Device * d, EditParam p, double delta)
{
  if (isParamLocked(d, p))
    return;

  // if unassigned, generate a delta
  if (isnan(delta))
    delta = _gdist(_gen);

  switch (p) {
    case INTENSITY:
    {
      double val = d->getIntensity()->asPercent();
      d->getIntensity()->setValAsPercent((float) (delta + val));
      return;
    }
    case HUE:
    {
      Eigen::Vector3d hsv = d->getColor()->getHSV();
      d->getColor()->setHSV(hsv[0] + (delta * 360.0), hsv[1], hsv[2]);
      return;
    }
    case SAT:
    {
      Eigen::Vector3d hsv = d->getColor()->getHSV();
      d->getColor()->setHSV(hsv[0], hsv[1] + delta, hsv[2]);
      return;
    }
    case VALUE:
    {
      Eigen::Vector3d hsv = d->getColor()->getHSV();
      d->getColor()->setHSV(hsv[0], hsv[1], hsv[2] + delta);
      return;
    }
    case RED:
    {
      double val = d->getColor()->getColorChannel("Red");
      d->getColor()->setColorChannel("Red", val + delta);
      return;
    }
    case BLUE:
    {
      double val = d->getColor()->getColorChannel("Blue");
      d->getColor()->setColorChannel("Blue", val + delta);
      return;
    }
    case GREEN:
    {
      double val = d->getColor()->getColorChannel("Green");
      d->getColor()->setColorChannel("Green", val + delta);
      return;
    }
    case POLAR:
    {
      if (d->paramExists("polar")) {
        LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("polar");
        double val = o->asPercent();
        o->setValAsPercent((float) (val + delta));
      }
      return;
    }
    case AZIMUTH:
    {
      if (d->paramExists("azimuth")) {
        LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("azimuth");
        double val = o->asPercent();
        o->setValAsPercent((float) (val + delta));
      }
      return;
    }
    case SOFT:
    {
      if (d->paramExists("penumbraAngle")) {
        LumiverseFloat* s = d->getParam<LumiverseFloat>("penumbraAngle");
        double val = s->asPercent();
        s->setValAsPercent((float) (val + delta));
      }
      return;
    }
    default:
      return;
  }
}

void Edit::setParam(Device * d, EditParam p, double val)
{
  if (isParamLocked(d, p))
    return;

  switch (p) {
    case INTENSITY:
    {
      d->getIntensity()->setValAsPercent((float) val);
      return;
    }
    case HUE:
    {
      LumiverseColor* c = d->getColor();

      if (c == nullptr)
        return;

      Eigen::Vector3d hsv = c->getHSV();
      c->setHSV(val * 360, hsv[1], hsv[2]);
      return;
    }
    case SAT:
    {
      LumiverseColor* c = d->getColor();

      if (c == nullptr)
        return;

      Eigen::Vector3d hsv = c->getHSV();
      c->setHSV(hsv[0], val, hsv[2]);
      return;
    }
    case VALUE:
    {
      LumiverseColor* c = d->getColor();

      if (c == nullptr)
        return;

      Eigen::Vector3d hsv = c->getHSV();
      c->setHSV(hsv[0], hsv[1], val);
      return;
    }
    case RED:
    {
      LumiverseColor* c = d->getColor();

      if (c == nullptr)
        return;

      c->setColorChannel("Red", val);
      return;
    }
    case BLUE:
    {
      LumiverseColor* c = d->getColor();

      if (c == nullptr)
        return;

      c->setColorChannel("Blue", val);
      return;
    }
    case GREEN:
    {
      LumiverseColor* c = d->getColor();

      if (c == nullptr)
        return;

      c->setColorChannel("Green", val);
      return;
    }
    case POLAR:
    {
      if (d->paramExists("polar")) {
        LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("polar");
        o->setValAsPercent((float) val);
      }
      return;
    }
    case AZIMUTH:
    {
      if (d->paramExists("azimuth")) {
        LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("azimuth");
        o->setValAsPercent((float) val);
      }
      return;
    }
    case SOFT:
    {
      if (d->paramExists("penumbraAngle")) {
        LumiverseFloat* s = d->getParam<LumiverseFloat>("penumbraAngle");
        s->setValAsPercent((float) val);
      }
      return;
    }
    default:
      return;
  }
}

double Edit::getParam(Device * d, EditParam p)
{
  switch (p) {
  case INTENSITY:
  {
    return d->getIntensity()->asPercent();
  }
  case HUE:
  {
    LumiverseColor* c = d->getColor();

    if (c == nullptr)
      return 0;

    Eigen::Vector3d hsv = c->getHSV();
    return hsv[0] / 360.0;
  }
  case SAT:
  {
    LumiverseColor* c = d->getColor();

    if (c == nullptr)
      return 0;

    Eigen::Vector3d hsv = c->getHSV();
    return hsv[1];
  }
  case VALUE:
  {
    LumiverseColor* c = d->getColor();

    if (c == nullptr)
      return 0;

    Eigen::Vector3d hsv = c->getHSV();
    return hsv[2];
  }
  case RED:
  {
    LumiverseColor* c = d->getColor();

    if (c == nullptr)
      return 0;

    return c->getColorChannel("Red");
  }
  case BLUE:
  {
    LumiverseColor* c = d->getColor();

    if (c == nullptr)
      return 0;

    return c->getColorChannel("Blue");
  }
  case GREEN:
  {
    LumiverseColor* c = d->getColor();

    if (c == nullptr)
      return 0;

    return c->getColorChannel("Green");
  }
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
    if (d->paramExists("penumbraAngle")) {
      LumiverseFloat* s = d->getParam<LumiverseFloat>("penumbraAngle");
      return s->asPercent();
    }
    return 0;
  }
  default:
    return 0;
  }
}

bool Edit::isParamLocked(Device * d, EditParam c)
{
  switch (c) {
  case INTENSITY:
    return isDeviceParamLocked(d->getId(), "intensity") || _lockedParams.count(INTENSITY) > 0;
  case HUE:
    return isDeviceParamLocked(d->getId(), "color") || _lockedParams.count(HUE) > 0;
  case SAT:
    return isDeviceParamLocked(d->getId(), "color") || _lockedParams.count(SAT) > 0;
  case VALUE:
    return isDeviceParamLocked(d->getId(), "color") || isDeviceParamLocked(d->getId(), "intensity") || _lockedParams.count(VALUE) > 0;
  case RED:
    return isDeviceParamLocked(d->getId(), "color") || _lockedParams.count(RED) > 0;
  case GREEN:
    return isDeviceParamLocked(d->getId(), "color") || _lockedParams.count(GREEN) > 0;
  case BLUE:
    return isDeviceParamLocked(d->getId(), "color") || _lockedParams.count(BLUE) > 0;
  case POLAR:
    return isDeviceParamLocked(d->getId(), "polar") || _lockedParams.count(POLAR) > 0;
  case AZIMUTH:
    return isDeviceParamLocked(d->getId(), "azimuth") || _lockedParams.count(AZIMUTH) > 0;
  case SOFT:
    return isDeviceParamLocked(d->getId(), "penumbraAngle") || _lockedParams.count(SOFT) > 0;
  default:
    return false;
  }
}

void Edit::constructConsistencySets()
{
  // using the constraints in the global settings, construct the consistency sets
  // for this edit
  auto& constraints = getGlobalSettings()->_constraints;

  // here's where we want to combine consistency sets and get a 
  // nice compact list of things we actually need to change for the edit 
  // to execute efficiently. 

  // this structure tracks which sets of devices need their parameters to be consistent.
  // If when adding to the relevant sets, a device belongs to two sets in the same
  // parameter, both sets must be merged.
  map<EditParam, Array<DeviceSet> > relevantSets;

  // all devices start in separate consistency sets for the affected parameters
  auto& devices = _affectedDevices.getDevices();
  for (auto& p : _affectedParams) {
    for (auto& d : devices) {
      DeviceSet newset;
      relevantSets[p].add(newset.add(d));
    }
  }

  for (auto& c : constraints) {
    auto& cdevices = c.second._affected.getDevices();
    
    // merging is done on a parameter level
    for (auto& p : c.second._params) {
      // if the edit isn't looking at the parameter, skip
      if (_affectedParams.count(p) == 0)
        continue;

      // construct the relevant set for this constraint
      DeviceSet r;

      // add all devices for global
      if (c.second._scope == GLOBAL) {
        r = r.add(c.second._affected);
      }
      // add devices in the edit for local
      if (c.second._scope == LOCAL) {
        for (auto& d : cdevices) {
          if (_affectedDevices.contains(d->getId())) {
            r = r.add(d);
          }
        }
      }

      // if the set is empty, just skip this next part
      if (r.size() == 0)
        continue;

      // check which sets the param set needs to be merged with
      auto& rdevices = r.getDevices();
      vector<DeviceSet> mergeList;
      for (auto& d : rdevices) {
        for (int i = 0; i < relevantSets[p].size(); ) {
          if (relevantSets[p][i].contains(d->getId())) {
            // remove from relevantSets and add to merge list
            mergeList.push_back(relevantSets[p].remove(i));
          }
          else {
            i++;
          }
        }
      }

      // merge the things that should be merged
      for (int i = 0; i < mergeList.size(); i++) {
        r = r.add(mergeList[i]);
      }

      relevantSets[p].add(r);
    }
  }

  // now that we have the constraints all nicely organized, time to create lower-level constraints
  // out of them and assign them a representative device
  for (auto& s : relevantSets) {
    for (auto& ds : s.second) {
      _consistencySets.push_back(pair<ConsistencyConstraint, string>(ConsistencyConstraint(ds, GLOBAL, { s.first }), ds.getIds()[0]));
    }
  }
}
