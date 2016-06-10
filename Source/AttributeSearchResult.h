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
#include "SearchResultsContainer.h"

class SearchResultsContainer;

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

  // if true, the clusterContents field has stuff in it
  bool isClusterCenter();

  // Gets the container for cluster elements 
  SearchResultsContainer* getClusterContainer();

  // Adds the specified element to this cluster
  void addToCluster(AttributeSearchResult* elem);

  // convert RGB to Lab, D65 ref illuminant
  Eigen::Vector3d rgbToLab(double r, double g, double b);

  // Calculate the distance between this result and another result
  // Current metric: average pixel difference in Lab
  double dist(Eigen::VectorXd& y);

private:
  // Search result object from the attribute search
  SearchResult* _result;

  // rendered image 
  Image _render;

  // vector representation of a scaled down (100 x 100) thumbnail image in Lab
  Eigen::VectorXd _features;

  // Contains elements that belong to this particular cluster
  SearchResultsContainer* _clusterContents;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeSearchResult)
};


#endif  // ATTRIBUTESEARCHRESULT_H_INCLUDED
