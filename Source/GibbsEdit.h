/*
  ==============================================================================

    GibbsEdit.h
    Created: 31 Oct 2016 11:55:52am
    Author:  falindrith

  ==============================================================================
*/

#ifndef GIBBSEDIT_H_INCLUDED
#define GIBBSEDIT_H_INCLUDED

#include "Edit.h"

class GibbsEdit : public Edit
{
public:
  GibbsEdit(set<EditParam> lockedParams);
  GibbsEdit(set<EditParam> lockedParams, vector<pair<DeviceSet, EditParam> > sets);
  
  void setAffected(string field, EditParam param);

  virtual void performEdit(Snapshot* s, double stepSize);

protected:
  vector<pair<DeviceSet, EditParam> > _affectedSets;
};


#endif  // GIBBSEDIT_H_INCLUDED
