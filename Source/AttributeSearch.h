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

// controllable lighting parameters. Split here since don't want to waste time
// parsing strings for things like color.hue
enum EditParam {
  INTENSITY,
  HUE,
  SAT,
  VALUE,
  POLAR,
  AZIMUTH,
  SOFT,
  RED,
  GREEN,
  BLUE
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
  EditConstraint(string select, EditParam p, EditNumDevices q) : _select(select), _param(p), _qty(q) { }

  // this is actually a Lumiverse query string indicating which lights should be selected.
  string _select;
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
  Array<string> _editHistory;
  double _objFuncVal;

  // Paired with a vector of cluster centers, indicates which cluster the result belongs to.
  unsigned long _cluster;
};

// kmeans typedefs
typedef dlib::matrix<double, 0, 1> sampleType;
typedef dlib::linear_kernel<sampleType> kernelType;

// Entry point to the search algorithm
list<SearchResult*> attributeSearch(map<string, AttributeControllerBase*>& active, int editDepth = 1);

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
  map<string, vector<EditConstraint> > _edits;
  int _editDepth;
  list<SearchResult*> _results;
  Snapshot* _original;
  double _fc;
  bool _singleSame;

  // Populates the _edits map. Basically tells the search how to search the things.
  void generateEdits();

  // Generates the default set of edits for the select string
  void generateDefaultEdits(string select);

  // Runs a single level iteration of the search algorithm, starting at the given scenes.
  list<SearchResult*> runSingleLevelSearch(list<SearchResult*> startScenes, int level, attrObjFunc f);

  // Given a current configuration, use MCMC to perform an edit on the configuration
  list<Eigen::VectorXd> performEdit(vector<EditConstraint> edit, Snapshot* orig, attrObjFunc f, string name);

  // Do MCMC with the given parameters. Returns the list of samples and number of accepted samples.
  // Samples list will be empty if saveSamples is false.
  pair<list<Eigen::VectorXd>, int> doMCMC(vector<EditConstraint> edit, Snapshot* start, attrObjFunc f, int iters, double sigma, bool saveSamples, string name);

  // computes the numeric derivative for the particular lighting parameter and
  // specified attribute
  double numericDeriv(EditConstraint c, Snapshot* s, attrObjFunc f, string& id);

  // updates the value for a Lumiverse parameter and returns the actual value after the update
  double setDeviceValue(DeviceInfo& info, double val, Snapshot* s);

  // Retrieves the current value for a Lumiverse parameter
  double getDeviceValue(EditConstraint c, Snapshot* s, string& id);

  // Returns true if the specified device parameter is locked in the Rig
  bool isParamLocked(EditConstraint c, string& id);

  // Returns the number of features in the vector used for search
  int getVecLength(vector<EditConstraint>& edit, Snapshot* s);
};

#endif  // ATTRIBUTESEARCH_H_INCLUDED
