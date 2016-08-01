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
    return x->dist(y, PPAVGLAB);
  });
}

KMeans::KMeans(distFuncType distFunc) : _distFunc(distFunc)
{
}

Array<shared_ptr<TopLevelCluster> >KMeans::cluster(int k, Array<shared_ptr<SearchResultContainer> >& points, InitMode init)
{
  // initialize points
  Array<shared_ptr<TopLevelCluster> > centers;

  if (init == FORGY)
    centers = forgy(k, points);
  if (init == RND_PART)
    centers = rndpart(k, points);

  return cluster(points, centers);
}

Array<shared_ptr<TopLevelCluster> > KMeans::cluster(Array<shared_ptr<SearchResultContainer>>& points, Array<shared_ptr<TopLevelCluster>>& centers)
{
  // assign each point to closest center
  // stop when no changes happen
  bool noChangeHappened = false;
  while (!noChangeHappened) {
    noChangeHappened = true;

    for (auto& p : points) {
      // find closest center
      int closest = closestCenter(p, centers);

      // assign center, mark if change
      if ((unsigned long) closest != p->getSearchResult()->_cluster)
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
      feats[c->getClusterId()] = c->getContainer()->getFeatures();
      feats[c->getClusterId()].setZero();
      scenes[c->getClusterId()] = c->getContainer()->getSearchResult()->_scene;
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
      c->getContainer()->setFeatures(feats[c->getClusterId()] / counts[c->getClusterId()]);
      c->getContainer()->getSearchResult()->_scene = scenes[c->getClusterId()] / counts[c->getClusterId()];
    }
  }

  // add points to proper centers
  for (auto& p : points) {
    centers[p->getSearchResult()->_cluster]->addToCluster(p);
  }

  return centers;
}

Array<shared_ptr<TopLevelCluster>> KMeans::divisive(int maxK, Array<shared_ptr<SearchResultContainer>>& points)
{
  // the divisive algorithm starts by assuming all points are in the same cluster, then
  // repeatedly divides them until the maximum number of clusters is reached,
  // or we run out of points to cluster

  // intialization, all points in one cluster
  Array<shared_ptr<TopLevelCluster> > centers;
  shared_ptr<TopLevelCluster> first = shared_ptr<TopLevelCluster>(new TopLevelCluster());
  for (auto& p : points) {
    first->addToCluster(p);
  }

  first->setClusterId(0);
  centers.add(first);

  // calculate distance matrix
  map<int, map<int, double> > dist;

  for (int i = 0; i < points.size(); i++) {
    // we index based on sample number
    int r = points[i]->getSearchResult()->_sampleNo;
    dist[r][r] = 0;
    for (int j = i + 1; j < points.size(); j++) {
      int c = points[j]->getSearchResult()->_sampleNo;
      double d = _distFunc(points[i].get(), points[j].get());
      dist[r][c] = d;
      dist[c][r] = d;
    }
  }

  // loop
  while (centers.size() < maxK && centers.size() < points.size()) {
    // calculate cluster stats
    double max = 0;
    shared_ptr<TopLevelCluster> largest;
    int largestIdx = 0;
    for (int i = 0; i < centers.size(); i++) {
      centers[i]->calculateStats(dist);
      double diam = centers[i]->getDiameter();
      if (diam > max) {
        max = diam;
        largest = centers[i];
        largestIdx = i;
      }
    }

    // have largest cluster, split into two centers
    auto endpoints = largest->getDiameterEndpoints();

    // create the centers
    auto c1 = shared_ptr<TopLevelCluster>(new TopLevelCluster());
    auto c2 = shared_ptr<TopLevelCluster>(new TopLevelCluster());

    // 25% c1 - 75% c2
    c1->getContainer()->getSearchResult()->_scene = endpoints.first->getSearchResult()->_scene * 0.25 + endpoints.second->getSearchResult()->_scene * 0.75;
    c1->getContainer()->setFeatures(endpoints.first->getFeatures() * 0.25 + endpoints.second->getFeatures() * 0.75);

    // 75% c1 - 25% c2
    c2->getContainer()->getSearchResult()->_scene = endpoints.first->getSearchResult()->_scene * 0.75 + endpoints.second->getSearchResult()->_scene * 0.25;
    c2->getContainer()->setFeatures(endpoints.first->getFeatures() * 0.75 + endpoints.second->getFeatures() * 0.25);

    // remove old center
    centers.remove(largestIdx);
    centers.add(c1);
    centers.add(c2);

    // clear results from all centers
    for (int i = 0; i < centers.size(); i++) {
      centers[i]->clear();
      centers[i]->setClusterId(i);
    }

    // cluster with kmeans
    centers = cluster(points, centers);

    // repeat
  }

  return centers;
}

Array<shared_ptr<TopLevelCluster>> KMeans::divisive(double t, Array<shared_ptr<SearchResultContainer>>& points)
{
  // this does the divisive algorithm as normal, except it stops when all cluster
  // diameters are below the threshold t instead of a pre-defined k

  // intialization, all points in one cluster
  Array<shared_ptr<TopLevelCluster> > centers;
  shared_ptr<TopLevelCluster> first = shared_ptr<TopLevelCluster>(new TopLevelCluster());
  for (auto& p : points) {
    first->addToCluster(p);
  }

  first->setClusterId(0);
  centers.add(first);

  // calculate distance matrix
  map<int, map<int, double> > dist;

  for (int i = 0; i < points.size(); i++) {
    // we index based on sample number
    int r = points[i]->getSearchResult()->_sampleNo;
    dist[r][r] = 0;
    for (int j = i + 1; j < points.size(); j++) {
      int c = points[j]->getSearchResult()->_sampleNo;
      double d = _distFunc(points[i].get(), points[j].get());
      dist[r][c] = d;
      dist[c][r] = d;
    }
  }

  // loop
  while (centers.size() < points.size()) {
    // calculate cluster stats
    double max = 0;
    shared_ptr<TopLevelCluster> largest;
    int largestIdx = 0;
    bool allDiamsBelowT = true;
    
    for (int i = 0; i < centers.size(); i++) {
      centers[i]->calculateStats(dist);
      double diam = centers[i]->getDiameter();
      if (diam > max) {
        max = diam;
        largest = centers[i];
        largestIdx = i;
      }

      if (diam >= t) {
        allDiamsBelowT = false;
      }
    }

    if (allDiamsBelowT)
      break;

    // have largest cluster, split into two centers
    auto endpoints = largest->getDiameterEndpoints();

    // create the centers
    auto c1 = shared_ptr<TopLevelCluster>(new TopLevelCluster());
    auto c2 = shared_ptr<TopLevelCluster>(new TopLevelCluster());

    // 25% c1 - 75% c2
    c1->getContainer()->getSearchResult()->_scene = endpoints.first->getSearchResult()->_scene * 0.25 + endpoints.second->getSearchResult()->_scene * 0.75;
    c1->getContainer()->setFeatures(endpoints.first->getFeatures() * 0.25 + endpoints.second->getFeatures() * 0.75);

    // 75% c1 - 25% c2
    c2->getContainer()->getSearchResult()->_scene = endpoints.first->getSearchResult()->_scene * 0.75 + endpoints.second->getSearchResult()->_scene * 0.25;
    c2->getContainer()->setFeatures(endpoints.first->getFeatures() * 0.75 + endpoints.second->getFeatures() * 0.25);;

    // remove old center
    centers.remove(largestIdx);
    centers.add(c1);
    centers.add(c2);

    // clear results from all centers
    for (int i = 0; i < centers.size(); i++) {
      centers[i]->clear();
      centers[i]->setClusterId(i);
    }

    // cluster with kmeans
    centers = cluster(points, centers);

    // repeat
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
    tlc->getContainer()->getSearchResult()->_scene = p->getSearchResult()->_scene;
    tlc->getContainer()->setFeatures(p->getFeatures());
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
    unsigned long count = 0;
    for (auto& p : points) {
      if (p->getSearchResult()->_cluster == (unsigned long) i) {
        avg += p->getFeatures();
        sceneAvg += p->getSearchResult()->_scene;
        count++;
      }
    }

    auto tlc = shared_ptr<TopLevelCluster>(new TopLevelCluster());
    tlc->setClusterId(i);
    tlc->getContainer()->getSearchResult()->_scene = sceneAvg / count;
    tlc->getContainer()->setFeatures(avg / count);
    centers.add(tlc);
  }

  return centers;
}

int KMeans::closestCenter(shared_ptr<SearchResultContainer> point, Array<shared_ptr<TopLevelCluster> >& centers)
{
  int minCenter = -1;
  double minDist = DBL_MAX;

  for (int i = 0; i < centers.size(); i++) {
    auto centerContainer = centers[i]->getContainer();
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
