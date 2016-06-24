/*
  ==============================================================================

    Clustering.h
    Created: 22 Jun 2016 1:46:23pm
    Author:  eshimizu

  ==============================================================================
*/

#ifndef CLUSTERING_H_INCLUDED
#define CLUSTERING_H_INCLUDED

#include "globals.h"
#include "TopLevelCluster.h"
#include "SearchResultContainer.h"
#include "KMeans.h"
#include "MeanShift.h"
#include "SpectralCluster.h"

namespace Clustering {
  // cluster elements using k-means. K is specified arbitrarily.
  // Returns the centers.
  Array<shared_ptr<TopLevelCluster> > kmeansClustering(Array<shared_ptr<SearchResultContainer> >& elems, int k, distFuncType f);

  // Mean Shift clustering
  Array<shared_ptr<TopLevelCluster> > meanShiftClustering(Array<shared_ptr<SearchResultContainer> >& elems, double bandwidth);

  // Spectral clustering
  Array<shared_ptr<TopLevelCluster> > spectralClustering(Array<shared_ptr<SearchResultContainer >>& elems, int k, distFuncType f);

  // Divisive clustering (hierarchical) using K-Means
  Array<shared_ptr<TopLevelCluster> > divisiveKMeansClustering(Array<shared_ptr<SearchResultContainer> >& elems, int k, distFuncType f);

  // Divisive clustering with a threshold
  Array<shared_ptr<TopLevelCluster> > thresholdedKMeansClustering(Array<shared_ptr<SearchResultContainer> >& elems, double t, distFuncType f);
}



#endif  // CLUSTERING_H_INCLUDED
