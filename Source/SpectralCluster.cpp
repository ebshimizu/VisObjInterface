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
  _distFunc = distFuncType([](SearchResultContainer* x, SearchResultContainer* y) {
    return x->dist(y);
  });
}

SpectralCluster::SpectralCluster(distFuncType distFunc) : _distFunc(distFunc)
{
}

SpectralCluster::~SpectralCluster()
{
}

Array<shared_ptr<TopLevelCluster> > SpectralCluster::cluster(Array<shared_ptr<SearchResultContainer> >& points, int maxK, double bandwidth)
{
  // DEBUG: OUTPUT TO FILE TO SEE MATRICES
  ofstream debugFile("debug.log", ios::trunc);

  // construct similarity matrix
  Eigen::MatrixXd W = constructSimilarityMatrix(points, bandwidth);

  // compute D and W (here S is W, fully connected similarity matrix)
  Eigen::MatrixXd D;
  D.resize(points.size(), points.size());
  D.setZero();

  for (int i = 0; i < points.size(); i++) {
    D(i, i) = W.row(i).sum();
  }

  // compute Lrw
  Eigen::MatrixXd I;
  I.resize(points.size(), points.size());
  I.setIdentity();

  // I - D^-1 W
  Eigen::MatrixXd Lrw = I - D.inverse() * W;

  debugFile << "W:\n" << W << "\n";
  debugFile << "D:\n" << D << "\n";
  debugFile << "Lrw:\n" << Lrw << "\n";

  // calculate eigenvectors and eigenvalues
  Eigen::EigenSolver<Eigen::MatrixXd> es(Lrw);
  auto eigenvals = es.eigenvalues();
  auto eigenvectors = es.eigenvectors();

  debugFile << "Eigenvalues:\n" << eigenvals << "\n";
  debugFile << "Eigenvectors:\n" << eigenvectors << "\n";

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

  debugFile << "Proposed K: " << maxIdx + 1 << "\n";
  int k = maxIdx + 1;

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
  Array<shared_ptr<TopLevelCluster> > centerResults;
  for (int i = 0; i < centers.size(); i++) {
    auto tlc = shared_ptr<TopLevelCluster>(new TopLevelCluster());
    tlc->_scene = centers[i];
    tlc->setClusterId(i);

    centerResults.add(tlc);
  }

  // assign elements to centers
  for (int i = 0; i < points.size(); i++) {
    centerResults[vecs[i].second]->addToCluster(points[i]);
  }

  return centerResults;
}

Eigen::MatrixXd SpectralCluster::constructSimilarityMatrix(Array<shared_ptr<SearchResultContainer> >& points, double bandwidth)
{
  // uses the gaussian similarity function to construct the similarity matrix for the given points.
  Eigen::MatrixXd S;
  S.resize(points.size(), points.size());

  for (int i = 0; i < points.size(); i++) {
    S(i, i) = 0;
    for (int j = i + 1; j < points.size(); j++) {
      // S is symmetric
      double diff = _distFunc(points[i].get(), points[j].get());
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
