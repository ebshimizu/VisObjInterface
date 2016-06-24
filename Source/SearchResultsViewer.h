/*
  ==============================================================================

    SearchResultsViewer.h
    Created: 15 Dec 2015 5:07:58pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef SEARCHRESULTSVIEWER_H_INCLUDED
#define SEARCHRESULTSVIEWER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "globals.h"
#include "AttributeSearch.h"
#include "SearchResultContainer.h"
#include "HistoryPanel.h"
#include "SearchResultsContainer.h"
#include <mutex>

class SearchResultsRenderer : public ThreadWithProgressWindow
{
public:
  SearchResultsRenderer(Array<SearchResultContainer*> results);
  ~SearchResultsRenderer();

  void run() override;
  void threadComplete(bool userPressedCancel) override;

private:
  Array<SearchResultContainer*> _results;
};

//==============================================================================
/*
*/
class SearchResultsViewer : public Component
{
public:
  SearchResultsViewer();
  ~SearchResultsViewer();

  void paint (Graphics&);
  void resized();

  void cluster();
  void sort();

  HistoryPanel* getHistory() { return _history; }

  // adds a new search result to the display area. Thread safe.
  bool addNewResult(SearchResult* r);

  // tells the search result container to update itself
  void showNewResults();

  // Deletes all scenes currently displayed and contained by the viewer.
  void clearContainer();

  // Returns true when the viewer can't display any more scenes
  bool isFull();

  // Removes results from the results section
  void cleanUp(int resultsToKeep);

  // Save results to a file
  void saveResults(string filename);

  // Load results from a file
  void loadResults(string filename);

  // display the selected trace in the results window
  void loadTrace(int selected);

	// Saves the currently displayed clusters
	void saveClusters();

	// Loads the specified clusters
	void loadClusters(int i);

	// Returns the number of clusters saved by the container
	int numSavedClusters();

private:
  Viewport* _viewer;
  Viewport* _historyViewer;
  SearchResultsContainer* _results;
  HistoryPanel* _history;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SearchResultsViewer)
};


#endif  // SEARCHRESULTSVIEWER_H_INCLUDED
