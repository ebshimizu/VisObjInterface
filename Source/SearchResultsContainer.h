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
#include "AttributeSearchResult.h"
#include "AttributeSorting.h"

class AttributeSearchResult;
class AttributeSorter;

//==============================================================================
class SearchResultsContainer : public Component
{
public:
  SearchResultsContainer();
  ~SearchResultsContainer();

  void paint(Graphics& g);
  void resized();

  void setWidth(int width);
  void sort();  // Uses the value in the globals to sort
  void sort(AttributeSorter* s);

  // Return the results for some other use
  Array<AttributeSearchResult*> getResults();

  // Add a new search result to the display area. Is thread safe.
  bool addNewResult(SearchResult* r);

  // integrate new results and display
  void showNewResults();

  // Deletes all scenes currently contained in the viewer.
  void clear();

  // Removes all scenes currently contained in the viewer.
  // THIS DOES NOT DELETE THE RESULTS
  void remove();

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

  // Insert pre-constructed attribute search result into this container
  // Does no similarity checks, is also thread safe
  void appendNewResult(AttributeSearchResult* r);

  // Number of elements to display on each row, defaults to the value stored in
  // the global settings
  void setElemsPerRow(int epr);

  // Returns the number of results contained in this container and all child containers
  int numResults();

private:
  Array<AttributeSearchResult*> _results;
  Array<AttributeSearchResult*> _newResults;

  // Creates an attribute search result container for the given result struct
  AttributeSearchResult* createContainerFor(SearchResult* r);

  // Clustering functions should all return an array of attribute search results contianers
  // that contain the elements belonging to that cluster center

  // cluster elements using k-means. K is specified arbitrarily.
  // Returns the centers.
  Array<AttributeSearchResult*> kmeansClustering(Array<AttributeSearchResult*>& elems, int k);

  // Mean Shift clustering
  Array<AttributeSearchResult*> meanShiftClustering(Array<AttributeSearchResult*>& elems, double bandwidth);

  // Spectral clustering
  Array<AttributeSearchResult*> spectralClustering(Array<AttributeSearchResult*>& elems);

  mutex _resultsLock;

  int _elemsPerRow;

  // internal sample id added to the result when its added to the container
  unsigned int _sampleId;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SearchResultsContainer)
};

#endif  // ATTRIBUTESEARCHCLUSTER_H_INCLUDED
