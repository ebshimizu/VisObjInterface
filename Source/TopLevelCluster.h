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
  void addToCluster(shared_ptr<SearchResultContainer> r);

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

  // mainly for distance metrics, constructs a search result container from the
  // top level cluster 
  shared_ptr<SearchResultContainer> constructResultContainer();

  Eigen::VectorXd _scene;
  Eigen::VectorXd _features;
private:
  int _id;
  SearchResultList _contents;
  Viewport* _viewer;

  // representative reults container, a copy of something in _results
  SearchResultContainer* _rep;
};



#endif  // TOPLEVELCLUSTER_H_INCLUDED
