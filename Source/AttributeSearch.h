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

// Flag indicating thread status for the dispatch thread
enum ThreadState {
	IDLE,
	EXPLORE,
	EXPLOIT,
	EDIT_WEIGHTS
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

class AttributeSearchThread : public Thread
{
public:
  AttributeSearchThread(String name, SearchResultsViewer* viewer, map<string, int>& sharedData);
  ~AttributeSearchThread();

  void setState(Snapshot* start, attrObjFunc& f, attrObjFunc& fsq, SearchMode m);

  void run() override;

  void setInternalID(int id) { _id = id; }

	// Returns the current state of the thread
	ThreadState getState() { return _status; }

	// Sets the thread state
	void setState(ThreadState s) { _status = s; }

	// Uses the thread's set of edits and current objective function to compute the
	// relative effectiveness of each edit
	void computeEditWeights(bool showStatusMessages = true, bool global = true);

	// Sets the starting lighting configuration
	void setStartConfig(Snapshot* start);

	// Sets the thread to use random initialization for LM searches
	void useRandomInit(bool use) { _randomInit = use; }

	void setParent(int p) { _parent = p; }

	// Special function for computing statistics about the hybrid search mode.
	void runHybridDebug();

  // resets the thread state to use the given scene ast the starting config.
  // also recomputes edit weights as needed, etc.
  void recenter(Snapshot* s = nullptr);

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
  attrObjFunc _fsq;
  double _T;
	SearchMode _mode;	// set before launching to prevent issues when switching mid-run
	bool _randomInit;	// For L-M descent, indicates if the starting position should be randomized
	ThreadState _status;

  // computes the edit weights for this thread's starting scene
  map<double, Edit*> _localEditWeights;

	// Counter for determining when to stop the exploit phase
	int _acceptedSamples;

	// Parent terminal scene id, -1 for root (current scene)
	int _parent;

	// Thread shared data, managed mostly by the dispatcher thread
	map<string, int>& _sharedData;

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
	void runMCMCEditSearch(bool force = false);

	// Runs the Levenberg-Marquardt Method. Gradient descent, does not use the edits generated
	// during setup. Intended as a reference.
	void runLMGDSearch(bool force = false);

  // Runs the MCMC edit sampling + LM refinement search.
  // there's a lot of code duplication here but that's mostly to easily isolate changes
  // between different search approaches.
  void runMCMCLMGDSearch();

  // Search function for testing out new search methods.
  // other stable searches left there for comparison
  void runExperimentalSearch();

  // Runs a search for each edit in order (non-parallel at the moment)
  void checkEdits();

  // Filters the _results set down to a reasonable size;
  list<SearchResult*> filterSearchResults(list<SearchResult*>& results);

  // Assign each result to the closest center (L2 norm)
  void clusterResults(list<SearchResult*>& results, vector<Eigen::VectorXd>& centers);

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
  bool isDuplicateEdit(Edit* e);

  // component to dump results into
  SearchResultsViewer* _viewer;
	SearchMode _mode;
	map<string, int> _sharedData;

  Array<AttributeSearchThread*> _threads;
  map<string, pair<AttributeControllerBase*, AttributeConstraint> > _active;
  attrObjFunc _f;
  attrObjFunc _fsq; // least squares version (guaranteed to always be positive basically
  Snapshot* _start;
  set<EditParam> _lockedParams;
};


#endif  // ATTRIBUTESEARCH_H_INCLUDED
