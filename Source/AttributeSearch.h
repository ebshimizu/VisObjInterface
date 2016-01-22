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
  ALL_HSV,
  ALL_RGB,
  ALL_SAT,
  ALL_HUE,
  ALL_INTENS,
  ALL_POS,
  KEY_HUE,
  FILL_HUE,
  RIM_HUE,
  KEY_INTENS,
  FILL_INTENS,
  RIM_INTENS,
  KEY_POS,
  FILL_POS,
  RIM_POS,
  KEY_SAT,
  FILL_SAT,
  RIM_SAT,
  KEY_HSV,
  FILL_HSV,
  RIM_HSV,
  KEY_FILL_INTENS,
  KEY_RIM_INTENS,
  FILL_RIM_INTENS,
  KEY_FILL_HUE,
  KEY_FILL_SAT,
  KEY_FILL_HSV,
  KEY_FILL_POS,
  KEY_RIM_HSV,
  KEY_RIM_POS,
  KEY_RGB,
  FILL_RGB,
  RIM_RGB,
  // Special edits - edits that require additional constraints to be matched
  // when they are performed.
  KEY_POS_FILL_POS_MATCH,             // Move the key but maintain fill relative position
  KEY_INTENS_RIM_CONTRAST_MATCH,      // Adjust key intensity but keep key-rim contrast constant
  KEY_INTENS_FILL_CONTRAST_MATCH,     // Adjust key intensity but keep key-fill contrast constant
  KEY_HUE_FILL_HUE_MATCH,             // Adjust key color, apply same relative change to fill color (rotate around color wheel basically)
  KEY_HUE_FILL_RIM_HUE_MATCH,         // Adjust key color, apply same relative change to rim/fill color
  // Special type for GUI
  CLUSTER_CENTER
};

// Don't want to use strings for this for speed reasons
enum EditLightType {
  L_KEY,
  L_FILL,
  L_RIM
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
  AZIMUTH
};

// these constraints define an edit (or rather, which parameters an edit can deal with)
// Some more uncommon edits may have additional constraints (maintain position of
// fill for example) and will be treated separately
struct EditConstraint {
  EditConstraint(EditLightType t, EditParam p) : _light(t), _param(p) { }

  EditLightType _light;
  EditParam _param;
};

// Results that eventually get returned to the UI layer
// contains edit history for debug, attribute value, and scene
class SearchResult {
public:
  SearchResult();
  SearchResult(Snapshot* s, Array<EditType> eh, map<string, double> av);
  SearchResult(const SearchResult& other);
  ~SearchResult();
  
  Snapshot* _scene;
  Array<EditType> _editHistory;
  map<string, double> _attrVals;

  // Paired with a vector of cluster centers, indicates which cluster the result belongs to.
  unsigned long _cluster;
};

// Objective function type to be passed to performEdit.
typedef function<double(Snapshot*)> attrObjFunc;

// kmeans typedefs
typedef dlib::matrix<double, 0, 1> sampleType;
typedef dlib::radial_basis_kernel<sampleType> kernelType;

// Entry point to the search algorithm
vector<SearchResult*> attributeSearch(map<string, AttributeControllerBase*> active, int editDepth = 1);

string editTypeToString(EditType t);

// Clusters results using k-means. K is chosen iteratively by computing the
// mean distance from every scene to the cluster center until it is below
// a specified threshold (global setting)
// Returns the cluster centers as vectors
vector<Eigen::VectorXd> clusterResults(vector<SearchResult*> results);

// Filters out scenes that are too similar to each other.
// - Sorts scenes into clusters
// - then from closest to farthest from the center, removes scenes that are
//   within a threshold from the point being considered
vector<SearchResult*> filterResults(vector<SearchResult*> results, vector<Eigen::VectorXd> centers);

// Returns a vector representation of the rig state contained in the snapshot
// Order of parameters detailed in implementation
Eigen::VectorXd snapshotToVector(Snapshot* s);

// Returns a snapshot consisting of the state contained in the vector
Snapshot* vectorToSnapshot(Eigen::VectorXd v);

// Given an EditLightType, get the corresponding light in the rig
Device* getSpecifiedDevice(EditLightType l, Snapshot* s);

// Run with progress window to allow user to abort search early
class AttributeSearchThread : public ThreadWithProgressWindow
{
public:
  AttributeSearchThread(map<string, AttributeControllerBase*> active, int editDepth = 1);
  ~AttributeSearchThread();

  void run() override;
  void threadComplete(bool userPressedCancel) override;

  vector<SearchResult*> getResults() { return _results; }

private:
  map<string, AttributeControllerBase*> _active;
  int _editDepth;
  vector<SearchResult*> _results;
  Snapshot* _original;

  // Runs a single level iteration of the search algorithm, starting at the given scenes.
  vector<SearchResult*> runSingleLevelSearch(vector<SearchResult*> startScenes, int level);

  // Given a current configuration, perform an edit on the configuration
  vector<Snapshot*> performEdit(EditType t, Snapshot* orig, attrObjFunc f);

  // computes the numeric derivative for the particular lighting parameter and
  // specified attribute
  double numericDeriv(EditConstraint c, EditType t, Snapshot* s, attrObjFunc f);

  // updates the value for a Lumiverse parameter
  void setDeviceValue(EditConstraint c, EditType t, double val, Snapshot* s);

  // Retrieves the current value for a Lumiverse parameter
  double getDeviceValue(EditConstraint c, Snapshot* s);
};

#endif  // ATTRIBUTESEARCH_H_INCLUDED
