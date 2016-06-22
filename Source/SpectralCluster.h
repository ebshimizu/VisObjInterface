/*
  ==============================================================================

    SpectralCluster.h
    Created: 16 Jun 2016 11:24:05am
    Author:  eshimizu

  ==============================================================================
*/

#ifndef SPECTRALCLUSTER_H_INCLUDED
#define SPECTRALCLUSTER_H_INCLUDED

#include "globals.h"
#include "KMeans.h"

class EigenvalueSorter
{
public:
  EigenvalueSorter() {}
  ~EigenvalueSorter() {}

  int compareElements(pair<int, double> first, pair<int, double> second);
};

// Uses the Spectral Clustering method to divide the given points into clusters
class SpectralCluster
{
public:
  SpectralCluster();
  SpectralCluster(distFuncType distFunc);
  ~SpectralCluster();

  Array<shared_ptr<TopLevelCluster> > cluster(Array<shared_ptr<SearchResultContainer> >& points, int maxK, double bandwidth);
private:
  distFuncType _distFunc;

  // Constructs a fully connected similarity matrix using the gaussian similarity function
  Eigen::MatrixXd constructSimilarityMatrix(Array<shared_ptr<SearchResultContainer> >& points, double bandwidth);
};

#endif  // SPECTRALCLUSTER_H_INCLUDED
