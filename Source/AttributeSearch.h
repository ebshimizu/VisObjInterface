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
#include "GibbsEdit.h"
#include "SearchResultsViewer.h"

class SearchResultsViewer;
class SearchResultContainer;

// Flag indicating thread status for the dispatch thread
enum ThreadState {
	IDLE,
	EXPLORE,
	EXPLOIT,
	EDIT_WEIGHTS
};

// Returns a vector representation of the rig state contained in the snapshot
// Order of parameters detailed in implementation
Eigen::VectorXd snapshotToVector(Snapshot* s);

// Returns a snapshot consisting of the state contained in the vector
Snapshot* vectorToSnapshot(Eigen::VectorXd v);

// Serializes a vector to a CSV row
String vectorToString(Eigen::VectorXd v);

// updates the existing snapshot with the given vector
void vectorToExistingSnapshot(Eigen::VectorXd source, Snapshot& dest);

class EditStats {
public:
  EditStats() {};

  float variance();
  float meanVals();
  float meanDiffs();
  float meanAccept();

  // given any arbitrary step, this would be the expected value of the positive steps:
  // avg(_diffs > 0) * count(_diffs > 0) / _diffs.size()
  float expectedDiff();

  vector<float> _diffs;  // stores the difference from the starting point obtained by the edit.
  vector<float> _vals;
  vector<float> _as;     // stores the acceptance chance for each sample, clamped at 1
  int _success;
  int _failure;
  double _G;
  double _p;
};

class AttributeSearchThread : public Thread
{
public:
  AttributeSearchThread(String name, SearchResultsViewer* viewer, map<string, int>& sharedData);
  ~AttributeSearchThread();

  void setState(Snapshot* start, attrObjFunc& f, attrObjFunc& fsq, SearchMode m, Image freeze);

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

	void setParent(int p) { _parent = p; }

	// Special function for computing statistics about the hybrid search mode.
	void runHybridDebug();

  // resets the thread state to use the given scene ast the starting config.
  // also recomputes edit weights as needed, etc.
  void recenter(Snapshot* s = nullptr);

  // select the next edit, maybe based on edit history, maybe not
  Edit* getNextEdit(vector<Edit*>& editHistory, bool useHistory = false);

  int _phase;
  String _statusMessage;

private:
  // object to dump results into once search is complete.
  // most of these variables capture the current search settings so user interference
  // doesn't affect how the current search is running
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
	ThreadState _status;
  int _resampleTime;
  int _samplesTaken;
  int _resampleThreads;   // number threads that get moved during search (particle filtering)
  map<Edit*, EditStats> _editStats;      // collection of numbers for computing edit weights during runtime
  EditSelectMode _editMode;
  Style _currentStyle;
  Array<shared_ptr<Snapshot> > _frontier;
  int _k;
  Array<shared_ptr<SearchResultContainer> > _currentResults;
  DistanceMetric _distMetric;
  DistanceMetric _dispMetric;

  // repulsion terms
  double _coneK;
  double _costScale;
  double _coneRadius;
  int _previousResultsSize;
  int _numPairs;

  // Note: this variable should probably change to an array of allowed style types at some point
  bool _useStyles;

  default_random_engine _gen;
  uniform_real_distribution<double> _udist;
  uniform_int_distribution<int> _styleDist;

  // Non-zero pixels indicate the affected pixel should not change
  Image _freezeMask;

  // True if the mask has at least one non-zero pixel
  bool _useMask;

  // from globals on search start, how much the pixels can differ in the highlighted regions
  double _maskTolerance;

  // this snapshot will always be the very first configuration the thread is set to.
  Snapshot* _fallback;

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

  // temporarily magic numbers
  double _n = 1;
  double _g = 0.5;

  // Starts up one search path. Run multiple times to get multiple results. Each run returns one result and puts
  // it in to the SearchResultsViewer object
  void runSearch();

  // Runs a search without using an inner iteration loop
  void runSearchNoInnerLoop();

  // Runs a search using a randomly selected starting point
  void runRandomStartSearch();

  // Maintains a frontier of elements and uses a random start to mix things up
  void runKRandomStartSearch();

  // Run frontier based search with no random init
  void runKSearch();

  // Run the CMA-ES search method
  void runCMAES();

  // repulsion term added to normal objective function
  void runRepulsionKMCMC();

  // internal helper functino for CMAES
  Eigen::VectorXd CMAESHelper(const Eigen::VectorXd &startingPoint, int lambda, int maxIters, vector<Eigen::VectorXd> *candidates);

	// Returns the partial numeric derivative wrt each adjustable parameter
	// Assumes we want the derivative for the current attribute objective function
	Eigen::VectorXd getDerivative(Snapshot& s);

	// Computes the jacobian matrix for the given configuration and current 
	Eigen::MatrixXd getJacobian(Snapshot& s);
  
  // randomizes the starting position using the available edits
  void randomizeStart();

  // Performs gradient descent from the starting scene
  Eigen::VectorXd performLMGD(Snapshot* scene, double& finalObjVal);

  // initializes the local weights to uniform values
  void setLocalWeightsUniform();

  // Updates the local weights with info from previous run
  void updateEditWeights(Edit* lastUsed, double gain);

  // Initialize the edit weights
  void initEditWeights();

  // Repulsion penalty term based on how similar current config is to other existing results
  double repulsion(Snapshot* s);

  // updates the relevant variables for the repulsion term
  // also since we know the results set is constantly increasing, if there's nothing new,
  // we can skip this step
  void updateRepulsionVars();
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
  Image _freezeMask;
  bool _useMask;
};


#endif  // ATTRIBUTESEARCH_H_INCLUDED
