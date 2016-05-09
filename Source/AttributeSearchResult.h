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
class SearchResultBlender : public Component, public Slider::Listener
{
public:
  SearchResultBlender(SearchResult* s);
  ~SearchResultBlender();

  void paint(Graphics& g);
  void resized();

  void sliderValueChanged(Slider* s) override;

private:
  Eigen::VectorXd _base;
  SearchResult* _target;
  Slider _blender;
};

//==============================================================================
/*
This class represents a scene returned from a search operation
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
  Image getImage() { return _render; }

  SearchResult* getSearchResult() { return _result; }
  Array<AttributeSearchResult*> getClusterElements() { return _clusterElems; }
 
  // Removes references to SearchResult objects in this GUI element. 
  // Note that this can cause memory leaks if used improperly
  void clearSearchResult();

  // Clicking does things.
  virtual void mouseDown(const MouseEvent& event);
  virtual void mouseEnter(const MouseEvent& event);
  virtual void mouseExit(const MouseEvent& event);

  void setClusterElements(Array<AttributeSearchResult*> elems);
  void addClusterElement(AttributeSearchResult* elem);

  // Indicates if the result is displaying its current cluster contents
  bool _isShowingCluster;

  int compareElements(AttributeSearchResult* first, AttributeSearchResult* second);

private:
  // Search result object from the attribute search
  SearchResult* _result;

  // rendered image 
  Image _render;

  // If this is empty, this object does not spawn a popup window element when clicked.
  Array<AttributeSearchResult*> _clusterElems;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeSearchResult)
};


#endif  // ATTRIBUTESEARCHRESULT_H_INCLUDED
