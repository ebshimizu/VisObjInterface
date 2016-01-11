/*
  ==============================================================================

    AttributeSearchResult.cpp
    Created: 7 Jan 2016 4:59:12pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeSearchResult.h"

//==============================================================================
AttributeSearchResult::AttributeSearchResult(SearchResult* result) : _result(result)
{
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
  String tt;
  for (const auto& kvp : result->_attrVals) {
    tt = tt + kvp.first + ": " + String(kvp.second) + "\n";
  }
  setTooltip(tt);
}

AttributeSearchResult::~AttributeSearchResult()
{
  if (_result != nullptr)
    delete _result;
}

void AttributeSearchResult::paint (Graphics& g)
{
  auto lbounds = getLocalBounds();
  lbounds.reduce(2, 2);
  g.drawImageWithin(_render, lbounds.getX(), lbounds.getY(), lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);
}

void AttributeSearchResult::resized()
{
}

void AttributeSearchResult::setImage(Image img)
{
  _render = img;
}

void AttributeSearchResult::mouseDown(const MouseEvent & event)
{
  if (event.mods.isRightButtonDown()) {
    PopupMenu m;
    m.addItem(1, "Move to Stage");
    m.addItem(2, "Move to Stage and Render");
    m.addItem(3, "Repeat Search with Selected");

    const int result = m.show();

    if (result == 1) {
      _result->_scene->loadRig(getRig());
      getApplicationCommandManager()->invokeDirectly(REFRESH_PARAMS, true);
      getApplicationCommandManager()->invokeDirectly(REFRESH_ATTR, true);
    }
    else if (result == 2) {
      _result->_scene->loadRig(getRig());
      getApplicationCommandManager()->invokeDirectly(REFRESH_PARAMS, false);
      getApplicationCommandManager()->invokeDirectly(REFRESH_ATTR, true);
      getApplicationCommandManager()->invokeDirectly(ARNOLD_RENDER, true);
    }
    else if (result == 3) {
      _result->_scene->loadRig(getRig());
      getApplicationCommandManager()->invokeDirectly(REFRESH_PARAMS, false);
      getApplicationCommandManager()->invokeDirectly(REFRESH_ATTR, true);
      getApplicationCommandManager()->invokeDirectly(SEARCH, true);
    }
  }
}
