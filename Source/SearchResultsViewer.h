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
#include "AttributeSearchResult.h"
#include "HistoryPanel.h"
#include "AttributeSearchCluster.h"

class SearchResultsRenderer : public ThreadWithProgressWindow
{
public:
  SearchResultsRenderer(Array<AttributeSearchResult*> results);
  ~SearchResultsRenderer();

  void run() override;
  void threadComplete(bool userPressedCancel) override;

private:
  Array<AttributeSearchResult*> _results;
};

//===============================================================================

class SearchResultsContainer : public Component
{
public:
  SearchResultsContainer();
  ~SearchResultsContainer();

  void paint(Graphics& g);
  void resized();

  // Display a new set of results in the container
  void display(list<SearchResult*>& results);

  // Recluster results
  void recluster();

  // Update the width of the component
  void setHeight(int height);

  void markDisplayedCluster(AttributeSearchResult* c);

  // Return the results for some other use
  Array<AttributeSearchResult*> getResults();

private:
  int _width;
  int _height;

  bool _isRoot;

  // results components
  Array<AttributeSearchResult*> _results;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SearchResultsContainer)
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

  void display(list<SearchResult*>& results);
  void redisplay();
  void sortDisplayedCluster();

  void setBotComponent(Component* c, Component* source);

  Array<AttributeSearchResult*> getResults();

  HistoryPanel* getHistory() { return _history; }

private:
  Viewport* _viewer;
  Viewport* _detailViewer;
  Viewport* _historyViewer;
  SearchResultsContainer* _container;
  AttributeSearchCluster* _displayedCluster;
  HistoryPanel* _history;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SearchResultsViewer)
};


#endif  // SEARCHRESULTSVIEWER_H_INCLUDED
