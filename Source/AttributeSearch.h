/*
  ==============================================================================

    AttributeSearch.h
    Created: 6 Jan 2016 11:25:42am
    Author:  falindrith

  ==============================================================================
*/

#ifndef ATTRIBUTESEARCH_H_INCLUDED
#define ATTRIBUTESEARCH_H_INCLUDED

#include "globals.h"
#include "AttributeControllerBase.h"
#include <dlib/clustering.h>

enum EditType {
  // Normal edits - edits that manipulate specific lighting parameters
  ALL,
  ALL_INTENSITY,
  ALL_COLOR,
  ALL_HUE,
  ALL_POSITION,
  ALL_SOFTNESS,
  ALL_PRIMARY,
  ALL_SECONDARY,
  ALL_TONER,
  ALL_AMBIENT,
  ALL_PRIMARY_INTENSITY,
  ALL_PRIMARY_COLOR,
  ALL_PRIMARY_HUE,
  ALL_PRIMARY_POSITION,
  ALL_PRIMARY_SOFTNESS,
  ALL_SECONDARY_INTENSITY,
  ALL_SECONDARY_COLOR,
  ALL_SECONDARY_HUE,
  ALL_SECONDARY_POSITION,
  ALL_SECONDARY_SOFTNESS,
  ALL_TONER_INTENSITY,
  ALL_TONER_COLOR,
  ALL_TONER_HUE,
  ALL_TONER_POSITION,
  ALL_TONER_SOFTNESS,
  ALL_AMBIENT_INTENSITY,
  ALL_AMBIENT_COLOR,
  ALL_AMBIENT_HUE,
  ALL_AMBIENT_POSITION,
  ALL_AMBIENT_SOFTNESS,
  FG_ALL,
  FG_ALL_INTENSITY,
  FG_ALL_COLOR,
  FG_ALL_HUE,
  FG_ALL_POSITION,
  FG_ALL_SOFTNESS,
  FG_PRIMARY_ALL,
  FG_PRIMARY_INTENSITY,
  FG_PRIMARY_COLOR,
  FG_PRIMARY_HUE,
  FG_PRIMARY_POSITION,
  FG_PRIMARY_SOFTNESS,
  FG_SECONDARY_ALL,
  FG_SECONDARY_INTENSITY,
  FG_SECONDARY_COLOR,
  FG_SECONDARY_HUE,
  FG_SECONDARY_POSITION,
  FG_SECONDARY_SOFTNESS,
  FG_TONER_ALL,
  FG_TONER_INTENSITY,
  FG_TONER_COLOR,
  FG_TONER_HUE,
  FG_TONER_POSITION,
  FG_TONER_SOFTNESS,
  FG_AMBIENT_ALL,
  FG_AMBIENT_INTENSITY,
  FG_AMBIENT_COLOR,
  FG_AMBIENT_HUE,
  FG_AMBIENT_POSITION,
  FG_AMBIENT_SOFTNESS,
  BG_ALL,
  BG_ALL_INTENSITY,
  BG_ALL_COLOR,
  BG_ALL_HUE,
  BG_ALL_POSITION,
  BG_ALL_SOFTNESS,
  BG_PRIMARY_ALL,
  BG_PRIMARY_INTENSITY,
  BG_PRIMARY_COLOR,
  BG_PRIMARY_HUE,
  BG_PRIMARY_POSITION,
  BG_PRIMARY_SOFTNESS,
  BG_SECONDARY_ALL,
  BG_SECONDARY_INTENSITY,
  BG_SECONDARY_COLOR,
  BG_SECONDARY_HUE,
  BG_SECONDARY_POSITION,
  BG_SECONDARY_SOFTNESS,
  BG_TONER_ALL,
  BG_TONER_INTENSITY,
  BG_TONER_COLOR,
  BG_TONER_HUE,
  BG_TONER_POSITION,
  BG_TONER_SOFTNESS,
  BG_AMBIENT_ALL,
  BG_AMBIENT_INTENSITY,
  BG_AMBIENT_COLOR,
  BG_AMBIENT_HUE,
  BG_AMBIENT_POSITION,
  
  // Special edits - edits that require additional constraints to be matched
  // when they are performed.
  FG_PRIMARY_BG_PRIMARY_INTENSITY,    // Foreground and background intensity changes matched
  FG_PRIMARY_BG_PRIMARY_COLOR,        // Foreground and background color changes matched (color wheel rotation)
  FG_TONER_JOINT_COLOR,
  BG_TONER_JOINT_COLOR,

  // Special type for GUI
  CLUSTER_CENTER
};

// Don't want to use strings for this for speed reasons
// These categories may encompass multiple lights
// Primary lights are expected to be key lights in the scene, while
// secondary lights are expected to be fill lights in the scene. This
// may or may not actually end up being true when the search runs, however it
// should be a good guideline to follow when setting up scenes.
enum EditLightType {
  FG_PRIMARY,
  FG_SECONDARY,
  FG_TONER,
  FG_AMBIENT,
  BG_PRIMARY,
  BG_SECONDARY,
  BG_TONER,
  BG_AMBIENT
};

// controllable lighting parameters. Split here since don't want to waste time
// parsing strings for things like color.hue
enum EditParam {
  INTENSITY,
  HUE,
  SAT,
  VALUE,
  RED,
  GREEN,
  BLUE,
  POLAR,
  AZIMUTH,
  SOFT
};

// Since we're dealing with a variable number of lights, the system needs
// to know how many devices to change on one edit.
enum EditNumDevices {
  D_ALL,    // Search though all devices at once
  D_UNIFORM, // Search through one device at a time (sub-edit)
  D_JOINT   // All lights get the same change applied to them
};

// these constraints define an edit (or rather, which parameters an edit can deal with)
// Some more uncommon edits may have additional constraints (maintain position of
// fill for example) and will be treated separately
struct EditConstraint {
  EditConstraint() { }
  EditConstraint(EditLightType t, EditParam p, EditNumDevices q) : _light(t), _param(p), _qty(q) { }

  EditLightType _light;
  EditParam _param;
  EditNumDevices _qty;
};

struct DeviceInfo {
  DeviceInfo() { }
  DeviceInfo(EditConstraint& c, string& id) : _c(c), _id(id) { }

  EditConstraint _c;
  string _id;
};

// Objective function type to be passed to performEdit.
typedef function<double(Snapshot*)> attrObjFunc;

// Results that eventually get returned to the UI layer
// contains edit history for debug, attribute value, and scene
class SearchResult {
public:
  SearchResult();
  SearchResult(const SearchResult& other);
  ~SearchResult();
  
  Eigen::VectorXd _scene;
  Array<EditType> _editHistory;
  double _objFuncVal;

  // Paired with a vector of cluster centers, indicates which cluster the result belongs to.
  unsigned long _cluster;
};

// kmeans typedefs
typedef dlib::matrix<double, 0, 1> sampleType;
typedef dlib::linear_kernel<sampleType> kernelType;

// Entry point to the search algorithm
list<SearchResult*> attributeSearch(map<string, AttributeControllerBase*>& active, int editDepth = 1);

string editTypeToString(EditType t);

// Clusters results using k-means. K is chosen iteratively by computing the
// mean distance from every scene to the cluster center until it is below
// a specified threshold (global setting)
// Returns the cluster centers as vectors
vector<Eigen::VectorXd> clusterResults(list<SearchResult*> results, int c = -1);

// Filters out scenes that are too similar to each other.
// - Sorts scenes into clusters
// - then from closest to farthest from the center, removes scenes that are
//   within a threshold from the point being considered
list<SearchResult*> filterResults(list<SearchResult*>& results, vector<Eigen::VectorXd>& centers);

// Remove vectors from the set that are within a specified threshold of other elements
// in the set
void filterResults(list<Eigen::VectorXd>& results, double t);

// Remove scenes from the set that are within a specified threshold of other elements
void filterResults(list<SearchResult*>& results, double t);

// For each center, return the closest search result to that center
list<SearchResult*> getClosestScenesToCenters(list<SearchResult*>& results, vector<Eigen::VectorXd>& centers);

// Returns a vector representation of the rig state contained in the snapshot
// Order of parameters detailed in implementation
Eigen::VectorXd snapshotToVector(Snapshot* s);

// Returns a snapshot consisting of the state contained in the vector
Snapshot* vectorToSnapshot(Eigen::VectorXd v);

// Serializes a vector to a CSV row
String vectorToString(Eigen::VectorXd v);

// Run with progress window to allow user to abort search early
class AttributeSearchThread : public ThreadWithProgressWindow
{
public:
  AttributeSearchThread(map<string, AttributeControllerBase*> active, int editDepth = 1);
  ~AttributeSearchThread();

  void run() override;
  void threadComplete(bool userPressedCancel) override;

  list<SearchResult*> getResults() { return _results; }

private:
  map<string, AttributeControllerBase*> _active;
  int _editDepth;
  list<SearchResult*> _results;
  Snapshot* _original;
  double _fc;
  bool _singleSame;

  // Runs a single level iteration of the search algorithm, starting at the given scenes.
  list<SearchResult*> runSingleLevelSearch(list<SearchResult*> startScenes, int level, attrObjFunc f);

  // Given a current configuration, use MCMC to perform an edit on the configuration
  list<Eigen::VectorXd> performEdit(EditType t, Snapshot* orig, attrObjFunc f);

  // Do MCMC with the given parameters. Returns the list of samples and number of accepted samples.
  // Samples list will be empty if saveSamples is false.
  pair<list<Eigen::VectorXd>, int> doMCMC(EditType t, Snapshot* start, attrObjFunc f, int iters, double sigma, bool saveSamples);

  // computes the numeric derivative for the particular lighting parameter and
  // specified attribute
  double numericDeriv(EditConstraint c, Snapshot* s, attrObjFunc f, string& id);

  // updates the value for a Lumiverse parameter and returns the actual value after the update
  double setDeviceValue(DeviceInfo& info, double val, Snapshot* s);

  // Retrieves the current value for a Lumiverse parameter
  double getDeviceValue(EditConstraint c, Snapshot* s, string& id);

  // Returns true if the specified device parameter is locked in the Rig
  bool isParamLocked(EditConstraint c, EditType t, Snapshot* s, string& id);

  // Returns the number of features in the vector used for search
  int getVecLength(EditType t, Snapshot* s);
};

#endif  // ATTRIBUTESEARCH_H_INCLUDED
