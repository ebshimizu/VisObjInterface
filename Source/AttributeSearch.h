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
#include "Edit.h"
#include "SearchResultsViewer.h"

class SearchResultsViewer;

struct DeviceInfo {
  DeviceInfo() { }
  DeviceInfo(EditConstraint& c, string& id) : _c(c), _id(id) { }

  EditConstraint _c;
  string _id;
};

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

// updates the existing snapshot with the given vector
void vectorToExistingSnapshot(Eigen::VectorXd source, Snapshot& dest);

// Run with progress window to allow user to abort search early
class AttributeSearchThread : public Thread
{
public:
  AttributeSearchThread(String name, SearchResultsViewer* viewer);
  ~AttributeSearchThread();

  void setState(Snapshot* start, attrObjFunc& f, SearchMode m);

  void run() override;

  void setInternalID(int id) { _id = id; }

  int _phase;

private:
  // object to dump results into once search is complete.
  SearchResultsViewer* _viewer;
  int _maxDepth;
  Snapshot* _original;
  double _fc;
  bool _singleSame;
  vector<Edit*> _edits;
  attrObjFunc _f;
  double _T;
	SearchMode _mode;	// set before launching to prevent issues when switching mid-run
	bool _randomInit;	// For L-M descent, indicates if the starting position should be randomized

  // counter for how many times the search result got rejected from the collection.
  // fail too many times, search depth increases
  int _failures;

  // Internal thread ID. Note that this is not the same as the thread ID according to the OS,
  // however it uniquely identifies this particular object.
  int _id;

  // Starts up one search path. Run multiple times to get multiple results. Each run returns one result and puts
  // it in to the SearchResultsViewer object
  void runSearch();

	// Runs the MCMC edit sampling search
	void runMCMCEditSearch();

	// Runs the Levenberg-Marquardt Method. Gradient descent, does not use the edits generated
	// during setup. Intended as a reference.
	void runLMGDSearch();

  // Runs a search for each edit in order (non-parallel at the moment)
  void checkEdits();

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

	// Returns the partial numeric derivative wrt each adjustable parameter
	// Assumes we want the derivative for the current attribute objective function
	Eigen::VectorXd getDerivative(Snapshot& s);

	// Computes the jacobian matrix for the given configuration and current 
	Eigen::MatrixXd getJacobian(Snapshot& s);
};

class AttributeSearch : public Thread
{
public:
  AttributeSearch(SearchResultsViewer* viewer);
  ~AttributeSearch();

  // When changing the number of threads, reallocate resources appropriately
  void reinit();

  // updates the state of the search on running. Will assume ownership of snapshot.
  void setState(Snapshot* start, map<string, AttributeControllerBase*> active);

  // Runs the search dispatch thread
  // this thread really just manages the runtime of other threads that actually perform
  // the search. this thread could also synchronize them and provide new starting scenes
  // if desired.
  // runs indefinitely until stopped
  void run() override;

  // stops the current search operation
  void stop();

  // Populates the _edits map. Basically tells the search how to search the things.
  void generateEdits(bool explore);

  // Generates the default set of edits for the select string
  // editType - 1 = default, 2 = area, 3 = system
  // If using a non-arbitrary edit type, just pass the system or area name, not a query string
  void generateDefaultEdits(string select, int editType);

private:
  // component to dump results into
  SearchResultsViewer* _viewer;

  Array<AttributeSearchThread*> _threads;
  map<string, AttributeControllerBase*> _active;
  attrObjFunc _f;
  Snapshot* _start;
  set<string> _lockedParams;
};


#endif  // ATTRIBUTESEARCH_H_INCLUDED
