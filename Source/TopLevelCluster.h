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
#include "SearchResultList.h"

class SearchResultList;
class SearchResultContainer;

// TopLevelCluster componets contain all elements that are part of the cluster
// TopLevelClusters typically reside in the SearchResultsContainer element
class TopLevelCluster : public Component
{
public:
  TopLevelCluster();
  ~TopLevelCluster();

  void resized() override;
  void paint(Graphics& g) override;

	void mouseDown(const MouseEvent& event);

  // adds and element to this cluster
  void addToCluster(shared_ptr<SearchResultContainer> r);

  // total number of elements in the cluster.
  // includes subclustered elements
  int numElements();

  // Returns cluster id number
  int getClusterId();
  void setClusterId(int id);

  // Returns the elemented contained by this cluster
  Array<shared_ptr<SearchResultContainer> > getChildElements();

  // gets all elements contained by the cluster, including sub cluster elements
  Array<shared_ptr<SearchResultContainer> > getAllChildElements();

  // Looks in _results and picks the best one to represent the cluster
  void setRepresentativeResult();
  shared_ptr<SearchResultContainer> getRepresentativeResult();

  void sort(AttributeSorter* s);

  // Clusters the results based on the Secondary Clustering Mode
  // selected in the options.
  void cluster();

  // Get element at index i in the search results list
  shared_ptr<SearchResultContainer> getElement(int i);

  // Removes all child elements from this container
  void clear();

  // calculates cluster diameter and identifies the points involved
  void calculateStats(function<double(SearchResultContainer*, SearchResultContainer*)> f);

  // calculates cluster diameter and identifies the points involved from a cached distance matrix
  void calculateStats(map<int, map<int, double> >& distanceMatrix);

  double getDiameter() { return _diameter; }
  pair<shared_ptr<SearchResultContainer>,shared_ptr<SearchResultContainer> > getDiameterEndpoints() { 
    return pair<shared_ptr<SearchResultContainer>, shared_ptr<SearchResultContainer> >(_x, _y);
  }
  int clusterSize() { return getAllChildElements().size(); }

  // Returns the search result containe representing this center object
  shared_ptr<SearchResultContainer> getContainer();

private:
  int _id;
  SearchResultList* _contents;
  Viewport* _viewer;

  bool _statsUpdated;                   // indicates that the below stats need to be recalculated
  double _diameter;                     // maximum distance between two points in the cluster
  shared_ptr<SearchResultContainer> _x; // endpoint of diameter
  shared_ptr<SearchResultContainer> _y; // endpoint of diameter
  Eigen::MatrixXd _distMatrix;          // Pairwise distance matrix

  // representative reults container, a copy of something in _results
  shared_ptr<SearchResultContainer> _rep;

  // Contains the _scene and _features for use in distance function computations.
  // it's a bit of 
  shared_ptr<SearchResultContainer> _centerContainer;
};



#endif  // TOPLEVELCLUSTER_H_INCLUDED
