/*
  ==============================================================================

    HistoryEntry.h
    Created: 5 Feb 2016 3:46:25pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef HISTORYENTRY_H_INCLUDED
#define HISTORYENTRY_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "LumiverseCore.h"
#include "LumiverseShowControl/LumiverseShowControl.h"
#include "globals.h"

using namespace Lumiverse;
using namespace Lumiverse::ShowControl;

//==============================================================================
/*
// Generally we'll add a history element every time someone renders the scene.
*/
class HistoryEntry    : public Component, public SettableTooltipClient
{
public:
  HistoryEntry(Snapshot* s, String note, Image img);
  ~HistoryEntry();

  void paint (Graphics&);
  void resized();

  void loadEntry();
  
  Snapshot* _sceneState;
  String _historyNote;
  Image _thumb;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HistoryEntry)
};


#endif  // HISTORYENTRY_H_INCLUDED
