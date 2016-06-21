/*
  ==============================================================================

    TopLevelCluster.h
    Created: 21 Jun 2016 1:40:40pm
    Author:  eshimizu

  ==============================================================================
*/

#ifndef TOPLEVELCLUSTER_H_INCLUDED
#define TOPLEVELCLUSTER_H_INCLUDED

#include "globals.h"
#include "SearchResultContainer.h"

// container that lays stuff out in grids
// doesn't delete the components
class GridLayout : public Component
{
public:
  GridLayout(Array<SearchResultContainer*> components, int cols, float heightRatio);
  ~GridLayout();

  void resized();
  void paint(Graphics& g);

  // Sets the width of this component. Should not use set bounds
  void setWidth(int width);
  float setHeightRatio(float heightRatio);

  void updateComponents(Array<SearchResultContainer*> newComponents);

private:
  Array<SearchResultContainer*> _components;
  int _cols;
  float _heightRatio;
};

// TopLevelCluster componets contain all elements that are part of the cluster
// TopLevelClusters typically reside in the SearchResultsContainer element
class TopLevelCluster : public Component
{
public:
  TopLevelCluster();
  ~TopLevelCluster();

  void resized() override;
  void paint(Graphics& g) override;

  // adds and element to this cluster
  void addToCluster(SearchResultContainer* r);

  // total number of elements in the cluster.
  // includes subclustered elements
  int numElements();

  // Returns cluster id number
  int getClusterId();
  void setClusterId(int id);

  // Returns the elemented contained by this cluster
  Array<SearchResultContainer*> getChildElements();

  // Looks in _results and picks the best one to represent the cluster
  void setRepresentativeResult();

private:
  int _id;
  Array<SearchResultContainer*> _results;
  Viewport* _viewer;
  GridLayout* _grid;

  // representative reults container, a copy of something in _results
  SearchResultContainer* _rep;
};



#endif  // TOPLEVELCLUSTER_H_INCLUDED
