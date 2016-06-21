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

Array<shared_ptr<TopLevelCluster> >KMeans::cluster(int k, Array<shared_ptr<SearchResultContainer> > points, InitMode init)
{
  // initialize points
  Array<shared_ptr<TopLevelCluster> > centers;

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
      feats[c->getClusterId()] = c->_features;
      feats[c->getClusterId()].setZero();
      scenes[c->getClusterId()] = c->_scene;
      scenes[c->getClusterId()].setZero();
      counts[c->getClusterId()] = 0;
    }

    // update centers
    for (auto& p : points) {
      SearchResult* r = p->getSearchResult();
      feats[r->_cluster] += p->getFeatures();
      scenes[r->_cluster] += r->_scene;
      counts[r->_cluster] += 1;
    }

    for (auto& c : centers) {
      c->_features = (feats[c->getClusterId()] / counts[c->getClusterId()]);
      c->_scene /= counts[c->getClusterId()];
    }
  }

  // add points to proper centers
  for (auto& p : points) {
    centers[p->getSearchResult()->_cluster]->addToCluster(p);
  }

  return centers;
}

Array<shared_ptr<TopLevelCluster> > KMeans::forgy(int k, Array<shared_ptr<SearchResultContainer> >& points) {
  // pick k random elements for centers, no repeats
  // we'll do this by removing random elements until the number remaining is <= k.
  Array<shared_ptr<SearchResultContainer> > tempPoints(points);

  while (tempPoints.size() > k) {
    // select random element to remove
    tempPoints.remove(rand() % tempPoints.size());
  }

  // create containers for centers
  Array<shared_ptr<TopLevelCluster> > centers;

  int i = 0;
  for (auto& p : tempPoints) {
    auto tlc = shared_ptr<TopLevelCluster>(new TopLevelCluster());
    tlc->setClusterId(i);
    tlc->_scene = p->getSearchResult()->_scene;
    tlc->_features = p->getFeatures();
    centers.add(tlc);

    i++;
  }

  return centers;
}

Array<shared_ptr<TopLevelCluster> > KMeans::rndpart(int k, Array<shared_ptr<SearchResultContainer> >& points)
{
  // randomly sort elements into k clusters
  Array<shared_ptr<SearchResultContainer> > tempPoints(points);

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
  Array<shared_ptr<TopLevelCluster> > centers;

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

    auto tlc = shared_ptr<TopLevelCluster>(new TopLevelCluster());
    tlc->setClusterId(i);
    tlc->_scene = sceneAvg / count;
    tlc->_features = avg / count;
    centers.add(tlc);
  }

  return centers;
}

int KMeans::closestCenter(shared_ptr<SearchResultContainer> point, Array<shared_ptr<TopLevelCluster> >& centers)
{
  int minCenter = -1;
  double minDist = DBL_MAX;

  for (int i = 0; i < centers.size(); i++) {
    auto centerContainer = centers[i]->constructResultContainer();
    double dist = _distFunc(point.get(), centerContainer.get());
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
