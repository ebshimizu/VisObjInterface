/*
  ==============================================================================

    SpectralCluster.cpp
    Created: 16 Jun 2016 11:24:05am
    Author:  eshimizu

  ==============================================================================
*/

#include "SpectralCluster.h"

SpectralCluster::SpectralCluster()
{
  // assign default distance function
  _distFunc = distFuncType([](AttributeSearchResult* x, AttributeSearchResult* y) {
    return x->dist(y);
  });
}

SpectralCluster::SpectralCluster(distFuncType distFunc) : _distFunc(distFunc)
{
}

SpectralCluster::~SpectralCluster()
{
}

Array<AttributeSearchResult*> SpectralCluster::cluster(Array<AttributeSearchResult*>& points, int maxK, double bandwidth)
{
  // construct similarity matrix
  Eigen::MatrixXd W = constructSimilarityMatrix(points, bandwidth);

  // compute D and W (here S is W, fully connected similarity matrix)
  // and D is just points.size() - 1 for each element
  Eigen::MatrixXd D;
  D.resize(points.size(), points.size());

  for (int i = 0; i < points.size(); i++) {
    D(i, i) = points.size() - 1;
  }

  // compute Lrw
  Eigen::MatrixXd I;
  I.resize(points.size(), points.size());
  I.setIdentity();

  // I - D^-1 W
  Eigen::MatrixXd Lrw = I - D.inverse() * W;

  // calculate eigenvectors and eigenvalues
  Eigen::EigenSolver<Eigen::MatrixXd> es(Lrw);
  auto eigenvals = es.eigenvalues();
  auto eigenvectors = es.eigenvectors();

  // need to sort vectors and vals in ascending order and pick k of them to be clusters.
  // we'll maintain references to where each value came from during the sort
  Array<pair<int, double> > sortedEigenvals;
  for (int i = 0; i < eigenvals.size(); i++) {
    sortedEigenvals.add(pair<int, double>(i, eigenvals[i].real()));
  }
  sortedEigenvals.sort(EigenvalueSorter());

  // now we gotta try to figure out how many clusters we have.
  // Up to a specified max, we'll use the eigengap heuristic to determine k
  int maxIdx = 0;
  double maxMag = 0;
  for (int i = 0; i < sortedEigenvals.size() - 1; i++) {
    // check relative distance from first ev to next ev
    double gapMag = sortedEigenvals[i + 1].second - sortedEigenvals[i].second;
    if (maxMag < gapMag) {
      maxIdx = i;
      maxMag = gapMag;
    }
  }

  int k = (maxIdx + 1 > maxK) ? maxK : maxIdx + 1;

  // DEBUG: enforcing additional clusters if too few
  k = (k <= 1) ? 3 : k;

  // assemble feature vectors for clustering
  vector<pair<Eigen::VectorXd, int> > vecs;
  for (int i = 0; i < points.size(); i++) {
    // pull the relevant values from the specified eigenvectors
    Eigen::VectorXd newVec;
    newVec.resize(k);
    for (int j = 0; j < k; j++) {
      // row: sample number, column: specified eigenvector in ascending order
      newVec[j] = eigenvectors(i, sortedEigenvals[j].first).real();
    }
    vecs.push_back(pair<Eigen::VectorXd, int>(newVec, -1));
  }

  // cluster with kmeans

  GenericKMeans clusterer;
  vector<Eigen::VectorXd> centers = clusterer.cluster(k, vecs, InitMode::FORGY);

  // now that we have a clustering, create relevent center attrs
  Array<AttributeSearchResult*> centerResults;
  for (int i = 0; i < centers.size(); i++) {
    SearchResult* r = new SearchResult();
    r->_scene = centers[i];
    r->_cluster = i;

    AttributeSearchResult* newCenter = new AttributeSearchResult(r);
    centerResults.add(newCenter);
  }

  // assign elements to centers
  for (int i = 0; i < points.size(); i++) {
    centerResults[vecs[i].second]->addToCluster(points[i]);
  }

  return centerResults;
}

Eigen::MatrixXd SpectralCluster::constructSimilarityMatrix(Array<AttributeSearchResult*>& points, double bandwidth)
{
  // uses the gaussian similarity function to construct the similarity matrix for the given points.
  Eigen::MatrixXd S;
  S.resize(points.size(), points.size());

  for (int i = 0; i < points.size(); i++) {
    S(i, i) = 0;
    for (int j = i + 1; j < points.size(); j++) {
      // S is symmetric
      double diff = _distFunc(points[i], points[j]);
      double weight = exp(-pow(diff, 2) / (2 * pow(bandwidth, 2)));
      S(i, j) = weight;
      S(j, i) = weight; 
    }
  }

  return S;
}

int EigenvalueSorter::compareElements(pair<int, double> first, pair<int, double> second)
{
  if (first.second < second.second)
    return -1;
  if (first.second > second.second)
    return 1;
  else
    return 0;
}
