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
#include <random>
#include <dlib/clustering.h>

struct DeviceInfo {
  DeviceInfo() { }
  DeviceInfo(EditConstraint& c, string& id) : _c(c), _id(id) { }

  EditConstraint _c;
  string _id;
};

// Objective function type to be passed to performEdit.
typedef function<double(Snapshot*)> attrObjFunc;

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

// Remove vectors from the set that are within a specified threshold of other elements
// in the set
void filterResults(list<mcmcSample>& results, double t);

// Remove scenes from the set that are within a specified threshold of other elements
void filterResults(list<SearchResult*>& results, double t);

// remove points from the set that are within a specified threshold 
void filterResults(list<Eigen::VectorXd>& points, double t);

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
  set<string> _lockedParams;

  // Runs a search with the current scenes in results as the starting scenes.
  void runStandardSearch();

  // runs an exploratory search using results as the starting scenes
  void runExploreSearch();

  // Populates the _edits map. Basically tells the search how to search the things.
  void generateEdits(bool explore);

  // Generates the default set of edits for the select string
  void generateDefaultEdits(string select);

  // Generates the color scheme edits for the selected area and system. If area is blank,
  // applies to all areas in the scene.
  void generateColorEdits(string area);

  // Runs a single level iteration of the search algorithm, starting at the given scenes.
  list<SearchResult*> runSingleLevelSearch(list<SearchResult*> startScenes, int level, attrObjFunc f);

  // Runs a single level iteration of the search algorithm, starting at the given scenes.
  list<SearchResult*> runSingleLevelExploreSearch(list<SearchResult*> startScenes, int level);

  // Given a current configuration, use MCMC to perform an edit on the configuration
  list<mcmcSample> performEdit(vector<EditConstraint> edit, Snapshot* orig, attrObjFunc f, string name, bool acceptStd = true);

  // Do MCMC with the given parameters. Returns the list of samples and number of accepted samples.
  // Samples list will be empty if saveSamples is false.
  pair<list<mcmcSample>, int> doMCMC(vector<EditConstraint> edit, Snapshot* start,
    attrObjFunc f, int iters, double sigma, bool saveSamples, string name, bool acceptStd);

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

  // Filters the _results set down to a reasonable size;
  list<SearchResult*> filterSearchResults(list<SearchResult*>& results);

  // Assign each result to the closest center (L2 norm)
  void clusterResults(list<SearchResult*>& results, vector<Eigen::VectorXd>& centers);

  // takes the base color scheme and returns a new one following color theory rules specified by type
  // This function will assume a certain number of dimensions to be present when editing the color.
  // it will also probably die if something it doesn't expect to happen happens
  void getNewColorScheme(Eigen::VectorXd& base, EditNumDevices type, normal_distribution<double>& dist, default_random_engine& rng);
};

#endif  // ATTRIBUTESEARCH_H_INCLUDED
