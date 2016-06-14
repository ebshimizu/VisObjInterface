/*
  ==============================================================================

    KMeans.cpp
    Created: 14 Jun 2016 11:27:12am
    Author:  eshimizu

  ==============================================================================
*/

#include "KMeans.h"

KMeans::KMeans()
{
  // assign default distance function
  _distFunc = distFuncType([](AttributeSearchResult* x, AttributeSearchResult* y) {
    return x->dist(y);
  });
}

KMeans::KMeans(distFuncType distFunc) : _distFunc(distFunc)
{
}

Array<AttributeSearchResult*> KMeans::cluster(int k, Array<AttributeSearchResult*> points, InitMode init)
{
  // initialize points
  Array<AttributeSearchResult*> centers;

  if (init == FORGY)
    centers = forgy(k, points);
  if (init == RND_PART)
    centers = rndpart(k, points);

  // assign each point to closest center
  // stop when no changes happen
  bool noChangeHappened = false;
  while (!noChangeHappened) {
    noChangeHappened = true;

    for (auto& p : points) {
      // find closest center
      int closest = closestCenter(p, centers);

      // assign center, mark if change
      if (closest != p->getSearchResult()->_cluster)
      {
        noChangeHappened = false;
        p->getSearchResult()->_cluster = closest;
      }
    }

    // reset centers
    map<int, int> counts;
    map<int, Eigen::VectorXd> feats;
    for (auto& c : centers) {
      feats[c->getSearchResult()->_cluster] = c->getFeatures();
      feats[c->getSearchResult()->_cluster].setZero();
      counts[c->getSearchResult()->_cluster] = 0;
    }

    // update centers
    for (auto& p : points) {
      SearchResult* r = p->getSearchResult();
      feats[r->_cluster] += p->getFeatures();
      counts[r->_cluster] += 1;
    }

    for (auto& c : centers) {
      c->setFeatures(feats[c->getSearchResult()->_cluster] / counts[c->getSearchResult()->_cluster]);
    }
  }

  // add points to proper centers
  for (auto& p : points) {
    centers[p->getSearchResult()->_cluster]->addToCluster(p);
  }

  return centers;
}

Array<AttributeSearchResult*> KMeans::forgy(int k, Array<AttributeSearchResult*>& points) {
  // pick k random elements for centers, no repeats
  // we'll do this by removing random elements until the number remaining is <= k.
  Array<AttributeSearchResult*> tempPoints(points);

  while (tempPoints.size() > k) {
    // select random element to remove
    tempPoints.remove(rand() % tempPoints.size());
  }

  // create containers for centers
  Array<AttributeSearchResult*> centers;

  int i = 0;
  for (auto& p : tempPoints) {
    SearchResult* r = new SearchResult();
    r->_cluster = i;
    
    AttributeSearchResult* c = new AttributeSearchResult(r);
    c->setFeatures(p->getFeatures());
    centers.add(c);

    i++;
  }

  return centers;
}

Array<AttributeSearchResult*> KMeans::rndpart(int k, Array<AttributeSearchResult*>& points)
{
  // randomly sort elements into k clusters
  Array<AttributeSearchResult*> tempPoints(points);

  // first make sure that we place at least one point in each cluster
  for (int i = 0; i < k; i++) {
    int selected = rand() % tempPoints.size();
    tempPoints[selected]->getSearchResult()->_cluster = i;
    tempPoints.remove(selected);
  }

  while (tempPoints.size() > 0) {
    int selected = rand() % tempPoints.size();
    tempPoints[selected]->getSearchResult()->_cluster = rand() % k;
    tempPoints.remove(selected);
  }

  // calculate centers
  Array<AttributeSearchResult*> centers;

  for (int i = 0; i < k; i++) {
    Eigen::VectorXd avg;
    int count = 0;
    for (auto& p : points) {
      if (p->getSearchResult()->_cluster == i) {
        avg += p->getFeatures();
        count++;
      }
    }

    SearchResult* r = new SearchResult();
    r->_cluster = i;

    AttributeSearchResult* c = new AttributeSearchResult(r);
    c->setFeatures(avg / count);
    centers.add(c);
  }

  return centers;
}

int KMeans::closestCenter(AttributeSearchResult * point, Array<AttributeSearchResult*>& centers)
{
  int minCenter = -1;
  double minDist = DBL_MAX;

  for (int i = 0; i < centers.size(); i++) {
    double dist = _distFunc(point, centers[i]);
    if (dist < minDist) {
      minCenter = i;
      minDist = dist;
    }
  }

  return minCenter;
}
