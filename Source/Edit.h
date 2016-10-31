/*
  ==============================================================================

    AttributeEdit.h
    Created: 12 May 2016 10:57:22am
    Author:  falindrith

  ==============================================================================
*/

#ifndef ATTRIBUTEEDIT_H_INCLUDED
#define ATTRIBUTEEDIT_H_INCLUDED

#include "globals.h"
#include "AttributeStyles.h"
#include <random>

using namespace Lumiverse;
using namespace Lumiverse::ShowControl;

// Objective function type to be passed to performEdit.
typedef function<double(Snapshot*, int, Style)> attrObjFunc;

enum ConsistencyScope
{
  LOCAL,  // Within edit. Parameters in affected devices must be the same within an edit.
  GLOBAL  // Within scene. Parameters in all relevent devices must be the same.
};

class ConsistencyConstraint
{
public:
  ConsistencyConstraint(DeviceSet affected, ConsistencyScope scope, set<EditParam> params);
  ConsistencyConstraint(string query, ConsistencyScope scope, set<EditParam> params);
  ConsistencyConstraint(const ConsistencyConstraint& other);
  ConsistencyConstraint();
  ~ConsistencyConstraint();

  bool contains(Device* d);

  DeviceSet _affected;
  set<EditParam> _params;
  ConsistencyScope _scope;
};

/*
An edit contains logic for manipulating one scene according to particular rules.
The base edit class does basic operations requiring no knowledge of how other
lights should be edited in relation to others.
*/
class Edit
{
public:
  Edit(set<EditParam> lockedParams);
  ~Edit();

  void initWithArea(string area, bool joint, bool uniform);
  void initWithSystem(string system, bool joint, bool uniform);
  void initArbitrary(string query, bool joint, bool uniform);

  void setParams(set<EditParam> affectedParams);

  // Takes a lighting configuration and modifies it according to the edit rules
  virtual void performEdit(Snapshot* s, double stepSize);

  // Gets the next edit given all the prior edits that have happened.
  virtual Edit* getNextEdit(Snapshot* current, attrObjFunc f, vector<Edit*>& editHistory, vector<Edit*>& availableEdits);

	// Gets the next edit based on specified weights
	virtual Edit* getNextEdit(vector<Edit*>& editHistory, map<double, Edit*>& weights, bool useHistory = false);

  // returns true if the edit can actually take effect
  virtual bool canDoEdit();

  // Returns the numeric derivative for the current edit
  // under default edit conditions, the elements in the returned vector correspond
  // to the elements in _affectedDevices
  // Pass in current function value to reduce calculation time if doing multiple derivative evals
  virtual Eigen::VectorXd numericDeriv(Snapshot* s, attrObjFunc f, double fx);

	// Returns an estimate of the variance of the edit around the given scene.
	// Approximates the importance of the edit
	virtual double variance(Snapshot* s, attrObjFunc f, double radius, int n);

  // Returns an expected value of the edit. This isn't necessarily the mathematical
  // expected value, but it's the same idea. Returns a number that expresses how
  // likely we are to get a better result
  virtual double expected(Snapshot* s, attrObjFunc f, double radius, int n);

  // Returns the number of samples that were better than the original value
  // along with the best scene found and the corresponding attribute value
  virtual double proportionGood(Snapshot* s, attrObjFunc f, double startVal, double radius, int n,
    Eigen::VectorXd& minScene, double& minVal);

  string _name;

  // Checks for edit equality.
  // If you are comparing subclasses, you will want to override this.
  virtual bool isEqual(Edit& other);

protected:
  void applyParamEdit(Device* d, EditParam p, double delta = NAN);
  void setParam(Device* d, EditParam p, double val);
  double getParam(Device* d, EditParam p);
  bool isParamLocked(Device* d, EditParam c);

	// Constructs the consistency sets (see below)
	void constructConsistencySets();

  set<EditParam> _affectedParams;

	// After the initialization step, the edit takes a look at the affected devices
	// and constructs these consistency sets. Consistency sets are sets of devices
	// that must have the same parameter values. When one parameter changes, the other devices
	// must match that parameter unless locked.
	// Note that areas do not necessarily have to be consistent.
	// The string is an id corresponding to the representative device in the set.
	// This is the device that sets the values for all other devices in the set
	vector<pair<ConsistencyConstraint, string> > _consistencySets;

  bool _joint;      // If true, adjusts all lights in the edit by the same delta
  bool _uniform;    // If true, sets all params in the edit to the same value
  
  DeviceSet _affectedDevices;

  default_random_engine _gen;
  uniform_real_distribution<double> _udist;
  normal_distribution<double> _gdist;

  set<EditParam> _lockedParams;
};



#endif  // ATTRIBUTEEDIT_H_INCLUDED
