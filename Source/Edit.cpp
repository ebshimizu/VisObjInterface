/*
  ==============================================================================

    AttributeEdit.cpp
    Created: 12 May 2016 10:57:22am
    Author:  falindrith

  ==============================================================================
*/

#include "Edit.h"

Edit::Edit(set<string> lockedParams) : _lockedParams(lockedParams)
{
  _joint = false;
  _uniform = false;

  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
  _gen = default_random_engine(seed1);
  _udist = uniform_real_distribution<double>(0.0, 1.0);
}

Edit::~Edit()
{
}

void Edit::initWithArea(string area, bool joint, bool uniform)
{
  _affectedAreas.insert(area);
  _affectedSystems.clear();
  string query = "$area=" + area;
  _joint = joint;
  _uniform = uniform;

  _affectedDevices = getRig()->select(query);
}

void Edit::initWithSystem(string system, bool joint, bool uniform)
{
  _affectedSystems.insert(system);
  _affectedAreas.clear();
  string query = "$system=" + system;
  _joint = joint;
  _uniform = uniform;

  _affectedDevices = getRig()->select(query);
}

void Edit::initArbitrary(string query, bool joint, bool uniform)
{
  _joint = joint;
  _uniform = uniform;
  _affectedDevices = getRig()->select(query);
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
    for (const auto& d : devices) {
      for (const auto& p : _affectedParams) {
        applyParamEdit(sd[d->getId()], p);
      }
    }
  }
}

Edit * Edit::getNextEdit(Snapshot* current, attrObjFunc f, vector<Edit*>& editHistory, vector<Edit*>& availableEdits)
{
  // TODO: DO BETTER THAN RANDOM
  // for now just pick a random edit until the entire search flow is working right
  int idx = (int) (_udist(_gen) * availableEdits.size());
  return availableEdits[idx];

  // weighted draw from the available edits towards edits with a stronger gradient magnitude
  vector<float> weights;
  float total = 0;
  double fx = f(current);

  for (auto& e : availableEdits) {
    auto grad = e->numericDeriv(current, f, fx);

    // minimum magnitude, so at least something gets selected if they're all 0.
    double mag = grad.norm();
    if (mag < 0.01)
      mag = 0.01;

    weights.push_back(mag);
    total += mag;
  }

  // normalize and sort
  map<float, Edit*> normWeights;
  float sum = 0;
  for (int i = 0; i < availableEdits.size(); i++) {
    sum += weights[i] / total;
    normWeights[sum] = availableEdits[i];
  }

  // pick an edit
  return normWeights.lower_bound(_udist(_gen))->second;
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
      d->getIntensity()->setValAsPercent(delta + val);
      return;
    }
    case HUE:
    {
      Eigen::Vector3d hsv = d->getColor()->getHSV();
      d->getColor()->setHSV(hsv[0] + (delta * 360), hsv[1], hsv[2]);
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
        o->setValAsPercent(val + delta);
      }
      return;
    }
    case AZIMUTH:
    {
      if (d->paramExists("azimuth")) {
        LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("azimuth");
        double val = o->asPercent();
        o->setValAsPercent(val + delta);
      }
      return;
    }
    case SOFT:
    {
      if (d->paramExists("penumbraAngle")) {
        LumiverseFloat* s = d->getParam<LumiverseFloat>("penumbraAngle");
        double val = s->asPercent();
        s->setValAsPercent(val + delta);
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
      d->getIntensity()->setValAsPercent(val);
      return;
    }
    case HUE:
    {
      Eigen::Vector3d hsv = d->getColor()->getHSV();
      d->getColor()->setHSV(val * 360, hsv[1], hsv[2]);
      return;
    }
    case SAT:
    {
      Eigen::Vector3d hsv = d->getColor()->getHSV();
      d->getColor()->setHSV(hsv[0], val, hsv[2]);
      return;
    }
    case VALUE:
    {
      Eigen::Vector3d hsv = d->getColor()->getHSV();
      d->getColor()->setHSV(hsv[0], hsv[1], val);
      return;
    }
    case RED:
    {
      d->getColor()->setColorChannel("Red", val);
      return;
    }
    case BLUE:
    {
      d->getColor()->setColorChannel("Blue", val);
      return;
    }
    case GREEN:
    {
      d->getColor()->setColorChannel("Green", val);
      return;
    }
    case POLAR:
    {
      if (d->paramExists("polar")) {
        LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("polar");
        o->setValAsPercent(val);
      }
      return;
    }
    case AZIMUTH:
    {
      if (d->paramExists("azimuth")) {
        LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("azimuth");
        o->setValAsPercent(val);
      }
      return;
    }
    case SOFT:
    {
      if (d->paramExists("penumbraAngle")) {
        LumiverseFloat* s = d->getParam<LumiverseFloat>("penumbraAngle");
        s->setValAsPercent(val);
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
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    return hsv[0];
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
  {
    return d->getColor()->getColorChannel("Red");
  }
  case BLUE:
  {
    return d->getColor()->getColorChannel("Blue");
  }
  case GREEN:
  {
    return d->getColor()->getColorChannel("Green");
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
    return isDeviceParamLocked(d->getId(), "intensity") || _lockedParams.count("intensity") > 0;
  case HUE:
    return isDeviceParamLocked(d->getId(), "color") || _lockedParams.count("color") > 0;
  case SAT:
    return isDeviceParamLocked(d->getId(), "color") || _lockedParams.count("color") > 0;
  case VALUE:
    return isDeviceParamLocked(d->getId(), "color") || _lockedParams.count("color") > 0;
  case RED:
    return isDeviceParamLocked(d->getId(), "color") || _lockedParams.count("color") > 0;
  case GREEN:
    return isDeviceParamLocked(d->getId(), "color") || _lockedParams.count("color") > 0;
  case BLUE:
    return isDeviceParamLocked(d->getId(), "color") || _lockedParams.count("color") > 0;
  case POLAR:
    return isDeviceParamLocked(d->getId(), "polar") || _lockedParams.count("polar") > 0;
  case AZIMUTH:
    return isDeviceParamLocked(d->getId(), "azimuth") || _lockedParams.count("azimuth") > 0;
  case SOFT:
    return isDeviceParamLocked(d->getId(), "penumbraAngle") || _lockedParams.count("penumbraAngle") > 0;
  default:
    return false;
  }
}
