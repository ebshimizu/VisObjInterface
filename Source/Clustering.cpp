/*
  ==============================================================================

    Clustering.cpp
    Created: 22 Jun 2016 1:46:23pm
    Author:  eshimizu

  ==============================================================================
*/

#include "Clustering.h"

namespace Clustering {

Array<shared_ptr<TopLevelCluster> > kmeansClustering(Array<shared_ptr<SearchResultContainer> >& elems, int k, distFuncType f)
{
  // mostly this function is a debug one to test that we can place things in
  // the proper GUI components
  if (elems.size() == 0)
    return Array<shared_ptr<TopLevelCluster> >();

  KMeans clusterer(f);
  return clusterer.cluster(k, elems, InitMode::FORGY);
}

Array<shared_ptr<TopLevelCluster> > meanShiftClustering(Array<shared_ptr<SearchResultContainer> >& elems, double bandwidth)
{
  // place feature vecs into a vector
  vector<Eigen::VectorXd> features;
  for (auto& e : elems) {
    features.push_back(e->getFeatures());
  }

  function<double(Eigen::VectorXd, Eigen::VectorXd)> distFunc = [](Eigen::VectorXd x, Eigen::VectorXd y) {
    double sum = 0;

    // iterate through vector in groups of 3
    for (int i = 0; i < x.size() / 3; i++) {
      int idx = i * 3;

      sum += sqrt(pow(y[idx] - x[idx], 2) +
        pow(y[idx + 1] - x[idx + 1], 2) +
        pow(y[idx + 2] - x[idx + 2], 2));
    }

    return sum / (x.size() / 3.0);
  };

  MeanShift shifter;
  vector<Eigen::VectorXd> centers = shifter.cluster(features, bandwidth, distFunc);

  Array<shared_ptr<TopLevelCluster> > centerContainers;

  // create containers for centers and add to main container
  int i = 0;
  for (auto& c : centers) {
    auto tlc = shared_ptr<TopLevelCluster>(new TopLevelCluster());
    tlc->getContainer()->getSearchResult()->_scene = c;
    tlc->setClusterId(i);

    centerContainers.add(tlc);
    i++;
  }

  // add elements to the proper center
  for (i = 0; i < features.size(); i++) {
    // find closest center
    double minDist = DBL_MAX;
    int minIdx = 0;
    for (int j = 0; j < centers.size(); j++) {
      double dist = distFunc(features[i], centers[j]);
      if (dist < minDist) {
        minDist = dist;
        minIdx = j;
      }
    }

    centerContainers[minIdx]->addToCluster(elems[i]);
  }

  return centerContainers;
}

Array<shared_ptr<TopLevelCluster> > spectralClustering(Array<shared_ptr<SearchResultContainer> >& elems, int k, distFuncType f)
{
  SpectralCluster clusterer(f);
  auto centers = clusterer.cluster(elems, k, getGlobalSettings()->_spectralBandwidth);
  return centers;
}

Array<shared_ptr<TopLevelCluster>> divisiveKMeansClustering(Array<shared_ptr<SearchResultContainer>>& elems, int k, distFuncType f)
{
  KMeans clusterer(f);
  return clusterer.divisive(k, elems);
}

Array<shared_ptr<TopLevelCluster>> thresholdedKMeansClustering(Array<shared_ptr<SearchResultContainer>>& elems, double t, distFuncType f)
{
  KMeans clusterer(f);
  return clusterer.divisive(t, elems);
}

}