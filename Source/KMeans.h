/*
  ==============================================================================

    KMeans.h
    Created: 14 Jun 2016 11:27:12am
    Author:  eshimizu

  ==============================================================================
*/

#ifndef KMEANS_H_INCLUDED
#define KMEANS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "globals.h"
#include "AttributeSearchResult.h"

typedef function<double(SearchResultContainer*, SearchResultContainer*)> distFuncType;
typedef function<double(Eigen::VectorXd, Eigen::VectorXd)> gdistFuncType;

enum InitMode {
  FORGY,    // Forgy init method: randomly select k points as mean
  RND_PART  // Randomly assign each point to a center k
};

// An implementation of K-means for clustering search results
class KMeans {
public:
  KMeans();
  KMeans(distFuncType distFunc);

  Array<SearchResultContainer*> cluster(int k, Array<SearchResultContainer*> points, InitMode init);

private:
  Array<SearchResultContainer*> forgy(int k, Array<SearchResultContainer*>& points);
  Array<SearchResultContainer*> rndpart(int k, Array<SearchResultContainer*>& points);

  // Returns the closest center to the specified point
  int closestCenter(SearchResultContainer* point, Array<SearchResultContainer*>& centers);

  distFuncType _distFunc;
};

// Operates on generic vectors instead of attribute search results
// It's almost the same, except the vectors don't get assigned as children to the centers
class GenericKMeans {
public:
  GenericKMeans();
  GenericKMeans(gdistFuncType distFunc);
  
  vector<Eigen::VectorXd> cluster(int k, vector<pair<Eigen::VectorXd, int> >& points, InitMode init);

private:
  vector<Eigen::VectorXd> forgy(int k, vector<pair<Eigen::VectorXd, int> >& points);
  vector<Eigen::VectorXd> rndpart(int k, vector<pair<Eigen::VectorXd, int> >& points);

  // Returns the closest center to the specified point
  int closestCenter(Eigen::VectorXd point, vector<Eigen::VectorXd>& centers);

  gdistFuncType _distFunc;
};

#endif  // KMEANS_H_INCLUDED
