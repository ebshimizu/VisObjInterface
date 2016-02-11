/*
  ==============================================================================

    HistoryEntry.cpp
    Created: 5 Feb 2016 3:46:25pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "HistoryEntry.h"
#include "MainComponent.h"

//==============================================================================
HistoryEntry::HistoryEntry(Snapshot * s, String note, Image img) :
  _sceneState(s), _historyNote(note), _thumb(img)
{
  setTooltip(note);
}

HistoryEntry::~HistoryEntry()
{
  delete _sceneState;
}

void HistoryEntry::paint (Graphics& g)
{
  g.fillAll(Colour(0xff333333));

  auto lbounds = getLocalBounds();
  g.drawImageWithin(_thumb, lbounds.getX(), lbounds.getY(), lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);
}

void HistoryEntry::resized()
{
}

void HistoryEntry::loadEntry()
{
  _sceneState->loadRig(getRig());

  // Re-render
  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

  if (mc != nullptr) {
    mc->refreshParams();
    mc->refreshAttr();
    mc->arnoldRender();
  }
}
