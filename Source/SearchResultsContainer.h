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

//==============================================================================
class SearchResultsContainer : public Component
{
public:
  SearchResultsContainer();
  ~SearchResultsContainer();

  void paint(Graphics& g);
  void resized();

  void updateSize();
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

private:
  // All results contains every result in the container. It should only be deleted at the top
  // level of the container hierarchy, which looks like:
  // -Search Result Container
  // --TopLevelCluster
  // ---SearchResult
  // ----SearchResultContainer
  // -----...
  // --SearchResultContainer
  // Any function that modifies the arrangement of the clusters needs to make sure to remove
  // the elements properly before deleting, so that they don't get deleted too early
  Array<shared_ptr<SearchResultContainer> > _allResults;
  SearchResultList _unclusteredResults;
  Array<shared_ptr<SearchResultContainer> > _newResults;
  Array<shared_ptr<TopLevelCluster> > _clusters;

  // Viewport for unclustered results
  Viewport* _unclusteredViewer;

  // Creates an attribute search result container for the given result struct
  SearchResultContainer* createContainerFor(SearchResult* r);

  // Clustering functions should all return an array of attribute search results contianers
  // that contain the elements belonging to that cluster center

  // cluster elements using k-means. K is specified arbitrarily.
  // Returns the centers.
  Array<shared_ptr<TopLevelCluster> > kmeansClustering(Array<shared_ptr<SearchResultContainer> >& elems, int k);

  // Mean Shift clustering
  Array<shared_ptr<SearchResultContainer> > meanShiftClustering(Array<shared_ptr<SearchResultContainer> >& elems, double bandwidth);

  // Spectral clustering
  Array<shared_ptr<SearchResultContainer> > spectralClustering(Array<shared_ptr<SearchResultContainer >>& elems);

  // clustering quality metric
  double daviesBouldin();

  mutex _resultsLock;

  int _elemsPerRow;

  int _columnSize;

  // internal sample id added to the result when its added to the container
  unsigned int _sampleId;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SearchResultsContainer)
};

#endif  // ATTRIBUTESEARCHCLUSTER_H_INCLUDED
