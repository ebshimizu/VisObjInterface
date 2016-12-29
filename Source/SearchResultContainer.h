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
#include "SearchResultList.h"

class SearchResultsContainer;
class SearchResultList;

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
  SearchResultContainer(shared_ptr<SearchResult> result, bool isHistoryItem = false);
  ~SearchResultContainer();

  void regenToolTip();

  void paint (Graphics&);
  void resized();

  // Sets the image for this component.
  void setImage(Image img);
	void computeGradient();
  Image getImage() { return _render; }

  shared_ptr<SearchResult> getSearchResult() { return _result; }
  void setSearchResult(shared_ptr<SearchResult> r) { _result = r; }
 
  // Removes references to SearchResult objects in this GUI element. 
  // Note that this can cause memory leaks if used improperly
  void clearSearchResult();

  // Used by the SystemExplorer
  void setSystem(DeviceSet selected);

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

  // Returns the number of results contained in this container (including
  // and sub clusters)
  int numResults();

  // Adds the specified element to this cluster
  void addToCluster(shared_ptr<SearchResultContainer> elem);

  // Returns the results contained within search result container.
  // this will either nothing, or all elements contained
  // within clusters contained by this element
  Array<shared_ptr<SearchResultContainer> > getResults();

  // Calculate the distance between this result and another result
  // Current metric: average pixel difference in Lab
  double dist(SearchResultContainer* y, DistanceMetric metric, bool overrideMask = false, bool invert = false);

  // Per-pixel average Lab pixel difference
  // special case: avgpixdist can ignore mask for purposes of filtering
  double avgPixDist(SearchResultContainer* y, bool overrideMask = false, bool invert = false);

  // L2 norm over Lab image vector
  double l2dist(SearchResultContainer* y, bool overrideMask = false, bool invert = false);
  
  // Maximum per-pixel Lab distance
  double maxPixDist(SearchResultContainer* y, bool overrideMask = false, bool invert = false);

  // 90th percentile per-pixel Lab distance
  double pctPixDist(SearchResultContainer* y, bool overrideMask = false, bool invert = false);

  // Parameter-wise L2 distance
  double l2paramDist(SearchResultContainer* y);

  // Does a softmax normalization of the parameter vector and then takes the L2 norm
  double l2paramDistSoftmax(SearchResultContainer* y);

  // Luminance L2 norm
  double l2LuminanceDist(SearchResultContainer* y, bool overrideMask = false, bool invert = false);

  // Distance in the attribute function space
  double attrDist(SearchResultContainer* y);

	// Lab + Gradient average per-pixel difference
	double directedAvgPixDist(SearchResultContainer* y, bool overrideMask = false, bool invert = false);

	// Gradient direction average per-pixel difference
	double directedOnlyAvgPixDist(SearchResultContainer* y, bool overrideMask = false, bool invert = false);

  // L2 norm of selected lighting parameters (selected list found in globals)
  double l2SelectedParamDist(SearchResultContainer* y);

  // Since each light is basically a color, this treats each param as a brightness and calculates L2 norm
  double l2GrayParamDist(SearchResultContainer* y);

  // sort cluster, if it exists.
  void sort(AttributeSorter* s);

	// updates the mask used by the container
	void updateMask();

  // Publicly accessible map in the event a container wants to precompute some order
  // and store the results in each element for easy sorting
  map<string, double> _sortVals;

  // Returns time the element was selected
  long long getTime();

  // Sets or unsets the element as most recent
  void setMostRecent(bool isRecent);

  // Returns true if the item has even been selected for something
  bool wasSelected();

private:
  // Search result object from the attribute search
  shared_ptr<SearchResult> _result;

  DeviceSet _selected;

  // rendered image 
  Image _render;

  // mask
  Image _mask;

  // vector representation of a scaled down (64 x 64) thumbnail image in Lab
  Eigen::VectorXd _features;

  // Contains elements that belong to this particular cluster
  SearchResultList* _clusterContents;

  // Indicates if this is a history item. If true, is not placed into history stack
  // when transferred to stage.
  bool _isHistoryItem;

  // indicates when an element was selected
  long long _selectTime;
  bool _isMostRecent;
  bool _wasSelected;

  // was formerly the tooltip
  String _debugInfo;

  class FullThumbRenderThread : public Thread {
  public:
    FullThumbRenderThread(SearchResultContainer* parent);
    ~FullThumbRenderThread();

    void run() override;

  private:
    SearchResultContainer* _parent;
  };

  FullThumbRenderThread _renderer;
  bool _fullResAvailable;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SearchResultContainer)
};

double repulsionTerm(SearchResultContainer * s, Array<shared_ptr<SearchResultContainer>>& pts, double c, double r, DistanceMetric metric);

#endif  // ATTRIBUTESEARCHRESULT_H_INCLUDED
