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
    double val = _udist(_gen);
    
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

Edit * Edit::getNextEdit(vector<Edit*>& editHistory, vector<Edit*>& availableEdits)
{
  // TODO: DO BETTER THAN RANDOM
  // for now just pick a random edit until the entire search flow is working right
  int idx = (int) (_udist(_gen) * availableEdits.size());
  return availableEdits[idx];
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

void Edit::applyParamEdit(Device * d, EditParam p, double delta)
{
  if (isParamLocked(d, p))
    return;

  // if unassigned, generate a delta
  if (delta < 0)
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

    return;
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
