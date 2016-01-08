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
