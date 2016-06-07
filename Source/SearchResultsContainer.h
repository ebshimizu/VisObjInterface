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

  // Returns true when the container can't support any more elements
  bool isFull();

  // Removes results from the results section
  void cleanUp(int resultsToKeep);

  // Save results in the container to a file
  void saveResults(string filename);

  void loadResults(string filename);

  // Load a selected trace into the results section
  void loadTrace(int selected);

private:
  Array<AttributeSearchResult*> _results;
  Array<AttributeSearchResult*> _newResults;

  bool _isRoot;

  mutex _resultsLock;
  int _numResults;

  // internal sample id added to the result when its added to the container
  unsigned int _sampleId;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SearchResultsContainer)
};

#endif  // ATTRIBUTESEARCHCLUSTER_H_INCLUDED
