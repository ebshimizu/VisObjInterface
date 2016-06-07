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
  void setSearchResult(SearchResult* r) { _result = r; }
 
  // Removes references to SearchResult objects in this GUI element. 
  // Note that this can cause memory leaks if used improperly
  void clearSearchResult();

  // Clicking does things.
  virtual void mouseDown(const MouseEvent& event);
  virtual void mouseEnter(const MouseEvent& event);
  virtual void mouseExit(const MouseEvent& event);

  // Indicates if the result is displaying its current cluster contents
  bool _isHovered;

  int compareElements(AttributeSearchResult* first, AttributeSearchResult* second);

  Eigen::VectorXd getFeatures();

private:
  // Search result object from the attribute search
  SearchResult* _result;

  // rendered image 
  Image _render;

  // vector representation of a scaled down (100 x 100) thumbnail image
  Eigen::VectorXd _features;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeSearchResult)
};


#endif  // ATTRIBUTESEARCHRESULT_H_INCLUDED
