/*
  ==============================================================================

    HistoryPanel.h
    Created: 5 Feb 2016 4:47:40pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef HISTORYPANEL_H_INCLUDED
#define HISTORYPANEL_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "SearchResultContainer.h"

//==============================================================================
/*
*/
class HistoryPanel : public Component
{
public:
  HistoryPanel();
  ~HistoryPanel();

  void paint (Graphics&);
  void resized();

  // Add an item at the end or in the specified location
  void addHistoryItem(SearchResultContainer* h, int idx = -1);

  // If -1, removes history item from the stack (end of array) and returns it.
  SearchResultContainer* removeHistoryItem(int idx = -1);

  // Deletes the specified entry from the history.
  void deleteHistoryItem(int idx = -1);

	void clearAllHistory();

  void setWidth(int width);

  // Returns the size of the history stack
  int getHistorySize();

private:
  Array<SearchResultContainer*> _history;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HistoryPanel)
};


#endif  // HISTORYPANEL_H_INCLUDED
