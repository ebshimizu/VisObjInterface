/*
  ==============================================================================

    GibbsEdit.cpp
    Created: 31 Oct 2016 11:55:52am
    Author:  falindrith

  ==============================================================================
*/

#include "GibbsEdit.h"

GibbsEdit::GibbsEdit(set<EditParam> lockedParams) : Edit(lockedParams)
{
}

GibbsEdit::GibbsEdit(set<EditParam> lockedParams, vector<pair<DeviceSet, EditParam> > sets) : Edit(lockedParams),
  _affectedSets(sets)
{
  _affectedDevices = DeviceSet();
  for (auto d : _affectedSets) {
    _affectedDevices.add(d.first);
  }
}

void GibbsEdit::setAffected(string field, EditParam param)
{
  // tells this Edit to manipulate all systems
  _affectedSets.clear();
  _affectedDevices = DeviceSet();
  _affectedParams.clear();
  _affectedParams.insert(param);

  Rig* rig = getRig();
  auto systems =  rig->getMetadataValues(field);
  for (auto system : systems) {
    DeviceSet s = rig->select("$" + field + "=" + system);
    _affectedSets.push_back(pair<DeviceSet, EditParam>(s, param));
    _affectedDevices = _affectedDevices.add(s);
  }
}

void GibbsEdit::performEdit(Snapshot * s, double /*stepSize*/)
{
  // here instead of using a delta we set all device sets and their parameters
  // according to a gibbs sampling process. 

  // TODO: For now hardcoding to directional settings. Will want to break in to specific edits
  // when integrating
  vector<float> results;
  results.resize(_affectedSets.size());

  vector<int> constraint;
  for (auto sn : _affectedSets) {
    constraint.push_back(0);
  }

  //GibbsSamplingGaussianMixture(results, constraint, results.size(), 1, 0.9, 0.1);

  auto data = s->getRigData();

  // assign results to systems
  int i = 0;
  for (pair<DeviceSet, EditParam> x : _affectedSets) {
    for (auto d : x.first.getDevices()) {
      setParam(data[d->getId()], x.second, results[i]);
    }

    i++;
  }
}