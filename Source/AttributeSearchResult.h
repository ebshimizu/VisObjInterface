/*
  ==============================================================================

    AttributeSearchResult.h
    Created: 7 Jan 2016 4:59:12pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef ATTRIBUTESEARCHRESULT_H_INCLUDED
#define ATTRIBUTESEARCHRESULT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "globals.h"
#include "AttributeSearch.h"

//==============================================================================
/*
*/
class AttributeSearchResult : public Component, public SettableTooltipClient
{
public:
  AttributeSearchResult(SearchResult* result);
  ~AttributeSearchResult();

  void paint (Graphics&);
  void resized();

  // Sets the image for this component.
  void setImage(Image img);

private:
  // Search result object from the attribute search
  SearchResult* _result;

  // rendered image 
  Image _render;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeSearchResult)
};


#endif  // ATTRIBUTESEARCHRESULT_H_INCLUDED
