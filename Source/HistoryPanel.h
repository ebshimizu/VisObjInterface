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
#include "HistoryEntry.h"

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
  void addHistoryItem(HistoryEntry* h, int idx = -1);

  // If -1, removes history item from the stack (end of array) and returns it.
  HistoryEntry* removeHistoryItem(int idx = -1);

  // Obliterates the specified entry from the history.
  void deleteHistoryItem(int idx = -1);

  void addRedoItem(HistoryEntry* h, int idx = -1);
  HistoryEntry* removeRedoItem(int idx = -1);
  void deleteRedoItem(int idx = -1);
  void clearRedo();

	void clearAllHistory();

  void setWidth(int width);

private:
  Array<HistoryEntry*> _history;
  Array<HistoryEntry*> _redo;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HistoryPanel)
};


#endif  // HISTORYPANEL_H_INCLUDED
