/*
  ==============================================================================

    SearchResultContainer.h
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
class SearchResultContainer : public Component, public SettableTooltipClient
{
public:
  SearchResultContainer(SearchResult* result);
  ~SearchResultContainer();

  void regenToolTip();

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

  int compareElements(SearchResultContainer* first, SearchResultContainer* second);

  Eigen::VectorXd getFeatures();
  void setFeatures(Eigen::VectorXd features);

  // if true, the clusterContents field has stuff in it
  bool isClusterCenter();

  // Gets the container for cluster elements 
  SearchResultsContainer* getClusterContainer();

  // Returns the number of results contained in this container (including
  // and sub clusters)
  int numResults();

  // Adds the specified element to this cluster
  void addToCluster(SearchResultContainer* elem);

  // convert RGB to Lab, D65 ref illuminant
  Eigen::Vector3d rgbToLab(double r, double g, double b);

  // Calculate the distance between this result and another result
  // Current metric: average pixel difference in Lab
  double dist(SearchResultContainer* y);

  // Per-pixel average Lab pixel difference
  // special case: avgpixdist can ignore mask for purposes of filtering
  double avgPixDist(SearchResultContainer* y, bool overrideMask = false);

  // L2 norm over Lab image vector
  double l2dist(SearchResultContainer* y);
  
  // Maximum per-pixel Lab distance
  double maxPixDist(SearchResultContainer* y);

  // 90th percentile per-pixel Lab distance
  double pctPixDist(SearchResultContainer* y);

  // Parameter-wise L2 distance
  double l2paramDist(SearchResultContainer* y);

  // Does a softmax normalization of the parameter vector and then takes the L2 norm
  double l2paramDistSoftmax(SearchResultContainer* y);

  // Luminance L2 norm
  double l2LuminanceDist(SearchResultContainer* y);

  // Distance in the attribute function space
  double attrDist(SearchResultContainer* y);

private:
  // Search result object from the attribute search
  SearchResult* _result;

  // rendered image 
  Image _render;

  // mask
  Image _mask;

  // vector representation of a scaled down (64 x 64) thumbnail image in Lab
  Eigen::VectorXd _features;

  // Contains elements that belong to this particular cluster
  SearchResultsContainer* _clusterContents;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SearchResultContainer)
};


#endif  // ATTRIBUTESEARCHRESULT_H_INCLUDED
