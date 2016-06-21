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
  _distFunc = distFuncType([](SearchResultContainer* x, SearchResultContainer* y) {
    return x->dist(y);
  });
}

KMeans::KMeans(distFuncType distFunc) : _distFunc(distFunc)
{
}

Array<SearchResultContainer*> KMeans::cluster(int k, Array<SearchResultContainer*> points, InitMode init)
{
  // initialize points
  Array<SearchResultContainer*> centers;

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
    map<int, Eigen::VectorXd> scenes;
    for (auto& c : centers) {
      feats[c->getSearchResult()->_cluster] = c->getFeatures();
      feats[c->getSearchResult()->_cluster].setZero();
      scenes[c->getSearchResult()->_cluster] = c->getSearchResult()->_scene;
      scenes[c->getSearchResult()->_cluster].setZero();
      counts[c->getSearchResult()->_cluster] = 0;
    }

    // update centers
    for (auto& p : points) {
      SearchResult* r = p->getSearchResult();
      feats[r->_cluster] += p->getFeatures();
      scenes[r->_cluster] += r->_scene;
      counts[r->_cluster] += 1;
    }

    for (auto& c : centers) {
      c->setFeatures(feats[c->getSearchResult()->_cluster] / counts[c->getSearchResult()->_cluster]);
      c->getSearchResult()->_scene /= counts[c->getSearchResult()->_cluster];
    }
  }

  // add points to proper centers
  for (auto& p : points) {
    centers[p->getSearchResult()->_cluster]->addToCluster(p);
  }

  return centers;
}

Array<SearchResultContainer*> KMeans::forgy(int k, Array<SearchResultContainer*>& points) {
  // pick k random elements for centers, no repeats
  // we'll do this by removing random elements until the number remaining is <= k.
  Array<SearchResultContainer*> tempPoints(points);

  while (tempPoints.size() > k) {
    // select random element to remove
    tempPoints.remove(rand() % tempPoints.size());
  }

  // create containers for centers
  Array<SearchResultContainer*> centers;

  int i = 0;
  for (auto& p : tempPoints) {
    SearchResult* r = new SearchResult();
    r->_cluster = i;
    r->_scene = p->getSearchResult()->_scene;
    
    SearchResultContainer* c = new SearchResultContainer(r);
    c->setFeatures(p->getFeatures());
    centers.add(c);

    i++;
  }

  return centers;
}

Array<SearchResultContainer*> KMeans::rndpart(int k, Array<SearchResultContainer*>& points)
{
  // randomly sort elements into k clusters
  Array<SearchResultContainer*> tempPoints(points);

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
  Array<SearchResultContainer*> centers;

  for (int i = 0; i < k; i++) {
    Eigen::VectorXd avg;
    Eigen::VectorXd sceneAvg;
    int count = 0;
    for (auto& p : points) {
      if (p->getSearchResult()->_cluster == i) {
        avg += p->getFeatures();
        sceneAvg += p->getSearchResult()->_scene;
        count++;
      }
    }

    SearchResult* r = new SearchResult();
    r->_cluster = i;
    r->_scene = sceneAvg / count;

    SearchResultContainer* c = new SearchResultContainer(r);
    c->setFeatures(avg / count);
    centers.add(c);
  }

  return centers;
}

int KMeans::closestCenter(SearchResultContainer * point, Array<SearchResultContainer*>& centers)
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

GenericKMeans::GenericKMeans()
{
  _distFunc = [](Eigen::VectorXd x, Eigen::VectorXd y) {
    return (x - y).norm();
  };
}

GenericKMeans::GenericKMeans(gdistFuncType distFunc) : _distFunc(distFunc)
{
}

vector<Eigen::VectorXd> GenericKMeans::cluster(int k, vector<pair<Eigen::VectorXd, int> >& points, InitMode init)
{
  // initialize points, vector and assigned center
  vector<Eigen::VectorXd> centers;

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
      int closest = closestCenter(p.first, centers);

      // assign center, mark if change
      if (closest != p.second)
      {
        noChangeHappened = false;
        p.second = closest;
      }
    }

    // reset centers
    vector<int> counts;
    for (auto& c : centers) {
      c.setZero();
      counts.push_back(0);
    }

    // update centers
    for (auto& p : points) {
      centers[p.second] += p.first;
      counts[p.second] += 1;
    }

    for (int i = 0; i < centers.size(); i++) {
      centers[i] /= counts[i];
    }
  }

  // points should contain mapping from point to assigned center as well
  return centers;
}

vector<Eigen::VectorXd> GenericKMeans::forgy(int k, vector<pair<Eigen::VectorXd, int>>& points)
{
  // pick k random elements for centers, no repeats
  // we'll do this by removing random elements until the number remaining is <= k.
  vector<pair<Eigen::VectorXd, int> > tempPoints(points);

  while (tempPoints.size() > k) {
    // select random element to remove
    tempPoints.erase(tempPoints.begin() + (rand() % tempPoints.size()));
  }

  // create centers
  vector<Eigen::VectorXd> centers;

  for (auto& p : tempPoints) {
    centers.push_back(p.first);
  }

  return centers;
}

vector<Eigen::VectorXd> GenericKMeans::rndpart(int k, vector<pair<Eigen::VectorXd, int>>& points)
{
  // randomly sort elements into k clusters
  vector<pair<Eigen::VectorXd, int> > tempPoints(points);

  // first make sure that we place at least one point in each cluster
  for (int i = 0; i < k; i++) {
    int selected = rand() % tempPoints.size();
    points[selected].second = i;
    tempPoints.erase(tempPoints.begin() + selected);
  }

  while (tempPoints.size() > 0) {
    int selected = rand() % tempPoints.size();
    points[selected].second = rand() % k;
    tempPoints.erase(tempPoints.begin() + selected);
  }

  // calculate centers
  vector<Eigen::VectorXd> centers;

  for (int i = 0; i < k; i++) {
    Eigen::VectorXd avg;
    int count = 0;
    for (auto& p : points) {
      if (p.second == i) {
        avg += p.first;
        count++;
      }
    }

    centers.push_back(avg / count);
  }

  return centers;
}

int GenericKMeans::closestCenter(Eigen::VectorXd point, vector<Eigen::VectorXd>& centers)
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
