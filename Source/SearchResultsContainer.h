/*
  ==============================================================================

    AttributeSearchCluster.h
    Created: 16 Feb 2016 3:20:25pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef ATTRIBUTESEARCHCLUSTER_H_INCLUDED
#define ATTRIBUTESEARCHCLUSTER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "SearchResultContainer.h"
#include "AttributeSorting.h"
#include "TopLevelCluster.h"
#include "SearchResultList.h"

class SearchResultContainer;
class AttributeSorter;
class SearchResultList;
class TopLevelCluster;

struct SearchMetadata {
	ClusterDisplayMode _mode;
	int _primaryClusters;
	int _secondaryClusters;
	ClusterMethod _primaryMethod;
	ClusterMethod _secondaryMethod;
	DistanceMetric _primaryMetric;
	DistanceMetric _secondaryMetric;
	FocusArea _primaryArea;
	FocusArea _secondaryArea;
	double _primaryThreshold;
	double _secondaryThreshold;
};

//==============================================================================
class SearchResultsContainer : public Component
{
public:
  SearchResultsContainer();
  ~SearchResultsContainer();

  void paint(Graphics& g);
  void resized();

  void updateSize(int height, int width);
  void sort();  // Uses the value in the globals to sort
  void sort(AttributeSorter* s);

  // Return the results for some other use
  Array<shared_ptr<SearchResultContainer> > getResults();

  // Add a new search result to the display area. Is thread safe.
  bool addNewResult(SearchResult* r);

  // integrate new results and display
  void showNewResults();

  // Deletes all scenes currently contained in the viewer.
  void clear();

  // Returns true when the container can't support any more elements
  bool isFull();

  // Removes results from the results section
  // also resets any clustering that was done previously
  void cleanUp(int resultsToKeep);

  // Sorts current elements into clusters
  // at the moment, this is tehnically thread safe but it basically just locks out
  // everyone until its done.
  void cluster();

  // Save results in the container to a file
  void saveResults(string filename);

  void loadResults(string filename);

  // Load a selected trace into the results section
  void loadTrace(int selected);

  // Number of elements to display on each row, defaults to the value stored in
  // the global settings
  void setElemsPerRow(int epr);

  // Returns the number of results contained in this container and all child containers
  int numResults();

  void calculateClusterStats();

	// these functions save stats about the clusters to a file,
	// based on which display mode was used
	void saveClusterStats(function<double(SearchResultContainer*, SearchResultContainer*)> f, SearchMetadata md);
	void saveClusterStats(function<double(SearchResultContainer*, SearchResultContainer*)> f,
		function<double(SearchResultContainer*, SearchResultContainer*)> f2, SearchMetadata md,
		Array<shared_ptr<TopLevelCluster> >& centers1, Array<shared_ptr<TopLevelCluster> >& centers2);

  // Saves the current clusters to a separate list
  void saveClustering();

	// create a search metadata object using current global settings
	SearchMetadata createSearchMetadata();

	// Loads the specified clusters from the specified index
	void loadClustering(int idx);

	// Returns the number of saved clusters
	int numSavedClusters();
	
	// Resets the clusters. Used when the display mode changes.
	void clearClusters();

private:
  // All results contains every result in the container. It should only be deleted at the top
  // level of the container hierarchy, which looks like:
  // -Search Result Container
  // --TopLevelCluster
  // ---SearchResult
  // ----SearchResultContainer
  // -----...
  // --SearchResultContainery
  Array<shared_ptr<SearchResultContainer> > _allResults;
  SearchResultList* _unclusteredResults;
  Array<shared_ptr<SearchResultContainer> > _newResults;
  Array<shared_ptr<TopLevelCluster> > _clusters;

  // Viewport for unclustered results
  Viewport* _unclusteredViewer;

  // Creates an attribute search result container for the given result struct
  SearchResultContainer* createContainerFor(SearchResult* r);

  // clustering quality metric
  double daviesBouldin();

  mutex _resultsLock;

  int _elemsPerRow;
  int _columnSize;

  // internal sample id added to the result when its added to the container
  unsigned int _sampleId;

  // list containing saved clusterings
  // Saving a clustering configuration should be as easy as saving the top level
  // clusters. 
  Array<Array<shared_ptr<TopLevelCluster> > > _savedResults;
	Array<SearchMetadata> _savedMetadata;

	void writeMetadata(std::ofstream &statsFile, SearchMetadata &md);
	map<int, map<int, double> > getPairwiseDist(function<double(SearchResultContainer*, SearchResultContainer*)> f, double& maxDist, int& x, int& y);

	// calculates the per-pixel standard deviation
	void ppsd(String prefix);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SearchResultsContainer)
};

#endif  // ATTRIBUTESEARCHCLUSTER_H_INCLUDED
