/*
  ==============================================================================

    AttributeSearch.cpp
    Created: 6 Jan 2016 11:25:42am
    Author:  falindrith

  ==============================================================================
*/

#include "AttributeSearch.h"
#include <random>
#include <list>
//#include <vld.h>

map<EditType, vector<EditConstraint> > editConstraints = {
  { ALL, { EditConstraint(FG_PRIMARY, INTENSITY, D_ALL), EditConstraint(FG_PRIMARY, HUE, D_ALL),
           EditConstraint(FG_PRIMARY, SAT, D_ALL), EditConstraint(FG_PRIMARY, VALUE, D_ALL),
           EditConstraint(FG_PRIMARY, POLAR, D_ALL), EditConstraint(FG_PRIMARY, AZIMUTH, D_ALL),
           EditConstraint(FG_PRIMARY, SOFT, D_ALL),
           EditConstraint(FG_SECONDARY, INTENSITY, D_ALL), EditConstraint(FG_SECONDARY, HUE, D_ALL),
           EditConstraint(FG_SECONDARY, SAT, D_ALL), EditConstraint(FG_SECONDARY, VALUE, D_ALL),
           EditConstraint(FG_SECONDARY, POLAR, D_ALL), EditConstraint(FG_SECONDARY, AZIMUTH, D_ALL),
           EditConstraint(FG_SECONDARY, SOFT, D_ALL),
           EditConstraint(FG_TONER, INTENSITY, D_ALL), EditConstraint(FG_TONER, HUE, D_ALL),
           EditConstraint(FG_TONER, SAT, D_ALL), EditConstraint(FG_TONER, VALUE, D_ALL),
           EditConstraint(FG_TONER, POLAR, D_ALL), EditConstraint(FG_TONER, AZIMUTH, D_ALL),
           EditConstraint(FG_TONER, SOFT, D_ALL),
           EditConstraint(FG_AMBIENT, INTENSITY, D_ALL), EditConstraint(FG_AMBIENT, HUE, D_ALL),
           EditConstraint(FG_AMBIENT, SAT, D_ALL), EditConstraint(FG_AMBIENT, VALUE, D_ALL),
           EditConstraint(FG_AMBIENT, POLAR, D_ALL), EditConstraint(FG_AMBIENT, AZIMUTH, D_ALL),
           EditConstraint(FG_AMBIENT, SOFT, D_ALL),
           EditConstraint(BG_PRIMARY, INTENSITY, D_ALL), EditConstraint(BG_PRIMARY, HUE, D_ALL),
           EditConstraint(BG_PRIMARY, SAT, D_ALL), EditConstraint(BG_PRIMARY, VALUE, D_ALL),
           EditConstraint(BG_PRIMARY, POLAR, D_ALL), EditConstraint(BG_PRIMARY, AZIMUTH, D_ALL),
           EditConstraint(BG_PRIMARY, SOFT, D_ALL),
           EditConstraint(BG_SECONDARY, INTENSITY, D_ALL), EditConstraint(BG_SECONDARY, HUE, D_ALL),
           EditConstraint(BG_SECONDARY, SAT, D_ALL), EditConstraint(BG_SECONDARY, VALUE, D_ALL),
           EditConstraint(BG_SECONDARY, POLAR, D_ALL), EditConstraint(BG_SECONDARY, AZIMUTH, D_ALL),
           EditConstraint(BG_SECONDARY, SOFT, D_ALL),
           EditConstraint(BG_TONER, INTENSITY, D_ALL), EditConstraint(BG_TONER, HUE, D_ALL),
           EditConstraint(BG_TONER, SAT, D_ALL), EditConstraint(BG_TONER, VALUE, D_ALL),
           EditConstraint(BG_TONER, POLAR, D_ALL), EditConstraint(BG_TONER, AZIMUTH, D_ALL),
           EditConstraint(BG_TONER, SOFT, D_ALL),
           EditConstraint(BG_AMBIENT, INTENSITY, D_ALL), EditConstraint(BG_AMBIENT, HUE, D_ALL),
           EditConstraint(BG_AMBIENT, SAT, D_ALL), EditConstraint(BG_AMBIENT, VALUE, D_ALL),
           EditConstraint(BG_AMBIENT, POLAR, D_ALL), EditConstraint(BG_AMBIENT, AZIMUTH, D_ALL),
           EditConstraint(BG_AMBIENT, SOFT, D_ALL)
  } },
  { ALL_INTENSITY,{ EditConstraint(FG_PRIMARY, INTENSITY, D_ALL), 
                    EditConstraint(FG_SECONDARY, INTENSITY, D_ALL), 
                    EditConstraint(FG_TONER, INTENSITY, D_ALL),
                    EditConstraint(FG_AMBIENT, INTENSITY, D_ALL),
                    EditConstraint(BG_PRIMARY, INTENSITY, D_ALL), 
                    EditConstraint(BG_SECONDARY, INTENSITY, D_ALL),
                    EditConstraint(BG_TONER, INTENSITY, D_ALL), 
                    EditConstraint(BG_AMBIENT, INTENSITY, D_ALL) 
  } },
  { ALL_COLOR,{ EditConstraint(FG_PRIMARY, HUE, D_ALL),
                EditConstraint(FG_PRIMARY, SAT, D_ALL), EditConstraint(FG_PRIMARY, VALUE, D_ALL),
                EditConstraint(FG_SECONDARY, HUE, D_ALL),
                EditConstraint(FG_SECONDARY, SAT, D_ALL), EditConstraint(FG_SECONDARY, VALUE, D_ALL),
                EditConstraint(FG_TONER, HUE, D_ALL),
                EditConstraint(FG_TONER, SAT, D_ALL), EditConstraint(FG_TONER, VALUE, D_ALL),
                EditConstraint(FG_AMBIENT, HUE, D_ALL),
                EditConstraint(FG_AMBIENT, SAT, D_ALL), EditConstraint(FG_AMBIENT, VALUE, D_ALL),
                EditConstraint(BG_PRIMARY, HUE, D_ALL),
                EditConstraint(BG_PRIMARY, SAT, D_ALL), EditConstraint(BG_PRIMARY, VALUE, D_ALL),
                EditConstraint(BG_SECONDARY, HUE, D_ALL),
                EditConstraint(BG_SECONDARY, SAT, D_ALL), EditConstraint(BG_SECONDARY, VALUE, D_ALL),
                EditConstraint(BG_TONER, HUE, D_ALL),
                EditConstraint(BG_TONER, SAT, D_ALL), EditConstraint(BG_TONER, VALUE, D_ALL),
                EditConstraint(BG_AMBIENT, HUE, D_ALL),
                EditConstraint(BG_AMBIENT, SAT, D_ALL), EditConstraint(BG_AMBIENT, VALUE, D_ALL)

  } },
  { ALL_HUE,{ EditConstraint(FG_PRIMARY, HUE, D_ALL), EditConstraint(FG_SECONDARY, HUE, D_ALL),
              EditConstraint(FG_TONER, HUE, D_ALL), EditConstraint(FG_AMBIENT, HUE, D_ALL),
              EditConstraint(BG_PRIMARY, HUE, D_ALL), EditConstraint(BG_SECONDARY, HUE, D_ALL),
              EditConstraint(BG_TONER, HUE, D_ALL), EditConstraint(BG_AMBIENT, HUE, D_ALL)

  } },
  { ALL_POSITION, { EditConstraint(FG_PRIMARY, POLAR, D_ALL), EditConstraint(FG_PRIMARY, AZIMUTH, D_ALL),
                    EditConstraint(FG_SECONDARY, POLAR, D_ALL), EditConstraint(FG_SECONDARY, AZIMUTH, D_ALL),
                    EditConstraint(FG_TONER, POLAR, D_ALL), EditConstraint(FG_TONER, AZIMUTH, D_ALL),
                    EditConstraint(FG_AMBIENT, POLAR, D_ALL), EditConstraint(FG_AMBIENT, AZIMUTH, D_ALL),
                    EditConstraint(BG_PRIMARY, POLAR, D_ALL), EditConstraint(BG_PRIMARY, AZIMUTH, D_ALL),
                    EditConstraint(BG_SECONDARY, POLAR, D_ALL), EditConstraint(BG_SECONDARY, AZIMUTH, D_ALL),
                    EditConstraint(BG_TONER, POLAR, D_ALL), EditConstraint(BG_TONER, AZIMUTH, D_ALL),
                    EditConstraint(BG_AMBIENT, POLAR, D_ALL), EditConstraint(BG_AMBIENT, AZIMUTH, D_ALL),
  } },
  { ALL_SOFTNESS, { EditConstraint(FG_PRIMARY, SOFT, D_ALL), EditConstraint(FG_SECONDARY, SOFT, D_ALL),
                    EditConstraint(FG_TONER, SOFT, D_ALL), EditConstraint(FG_AMBIENT, SOFT, D_ALL),
                    EditConstraint(BG_PRIMARY, SOFT, D_ALL), EditConstraint(BG_SECONDARY, SOFT, D_ALL),
                    EditConstraint(BG_TONER, SOFT, D_ALL), EditConstraint(BG_AMBIENT, SOFT, D_ALL)
  } },
  { ALL_PRIMARY, { EditConstraint(FG_PRIMARY, INTENSITY, D_ALL), EditConstraint(FG_PRIMARY, HUE, D_ALL),
                   EditConstraint(FG_PRIMARY, SAT, D_ALL), EditConstraint(FG_PRIMARY, VALUE, D_ALL),
                   EditConstraint(FG_PRIMARY, POLAR, D_ALL), EditConstraint(FG_PRIMARY, AZIMUTH, D_ALL),
                   EditConstraint(FG_PRIMARY, SOFT, D_ALL),
                   EditConstraint(BG_PRIMARY, INTENSITY, D_ALL), EditConstraint(BG_PRIMARY, HUE, D_ALL),
                   EditConstraint(BG_PRIMARY, SAT, D_ALL), EditConstraint(BG_PRIMARY, VALUE, D_ALL),
                   EditConstraint(BG_PRIMARY, POLAR, D_ALL), EditConstraint(BG_PRIMARY, AZIMUTH, D_ALL),
                   EditConstraint(BG_PRIMARY, SOFT, D_ALL)
  } },
  { ALL_SECONDARY, { EditConstraint(FG_SECONDARY, INTENSITY, D_ALL), EditConstraint(FG_SECONDARY, HUE, D_ALL),
                     EditConstraint(FG_SECONDARY, SAT, D_ALL), EditConstraint(FG_SECONDARY, VALUE, D_ALL),
                     EditConstraint(FG_SECONDARY, POLAR, D_ALL), EditConstraint(FG_SECONDARY, AZIMUTH, D_ALL),
                     EditConstraint(FG_SECONDARY, SOFT, D_ALL),
                     EditConstraint(BG_SECONDARY, INTENSITY, D_ALL), EditConstraint(BG_SECONDARY, HUE, D_ALL),
                     EditConstraint(BG_SECONDARY, SAT, D_ALL), EditConstraint(BG_SECONDARY, VALUE, D_ALL),
                     EditConstraint(BG_SECONDARY, POLAR, D_ALL), EditConstraint(BG_SECONDARY, AZIMUTH, D_ALL),
                     EditConstraint(BG_SECONDARY, SOFT, D_ALL)
  } },
  { ALL_TONER, { EditConstraint(FG_TONER, INTENSITY, D_ALL), EditConstraint(FG_TONER, HUE, D_ALL),
                 EditConstraint(FG_TONER, SAT, D_ALL), EditConstraint(FG_TONER, VALUE, D_ALL),
                 EditConstraint(FG_TONER, POLAR, D_ALL), EditConstraint(FG_TONER, AZIMUTH, D_ALL),
                 EditConstraint(FG_TONER, SOFT, D_ALL),
                 EditConstraint(BG_TONER, INTENSITY, D_ALL), EditConstraint(BG_TONER, HUE, D_ALL),
                 EditConstraint(BG_TONER, SAT, D_ALL), EditConstraint(BG_TONER, VALUE, D_ALL),
                 EditConstraint(BG_TONER, POLAR, D_ALL), EditConstraint(BG_TONER, AZIMUTH, D_ALL),
                 EditConstraint(BG_TONER, SOFT, D_ALL)
  } },
  { ALL_AMBIENT, { EditConstraint(FG_AMBIENT, INTENSITY, D_ALL), EditConstraint(FG_AMBIENT, HUE, D_ALL),
                   EditConstraint(FG_AMBIENT, SAT, D_ALL), EditConstraint(FG_AMBIENT, VALUE, D_ALL),
                   EditConstraint(FG_AMBIENT, POLAR, D_ALL), EditConstraint(FG_AMBIENT, AZIMUTH, D_ALL),
                   EditConstraint(FG_AMBIENT, SOFT, D_ALL),
                   EditConstraint(BG_AMBIENT, INTENSITY, D_ALL), EditConstraint(BG_AMBIENT, HUE, D_ALL),
                   EditConstraint(BG_AMBIENT, SAT, D_ALL), EditConstraint(BG_AMBIENT, VALUE, D_ALL),
                   EditConstraint(BG_AMBIENT, POLAR, D_ALL), EditConstraint(BG_AMBIENT, AZIMUTH, D_ALL),
                   EditConstraint(BG_AMBIENT, SOFT, D_ALL)
  } },
  { ALL_PRIMARY_INTENSITY, { EditConstraint(FG_PRIMARY, INTENSITY, D_ALL), EditConstraint(BG_PRIMARY, INTENSITY, D_ALL) } },
  { ALL_PRIMARY_COLOR, { EditConstraint(FG_PRIMARY, HUE, D_ALL), EditConstraint(BG_PRIMARY, HUE, D_ALL), 
                        EditConstraint(FG_PRIMARY, SAT, D_ALL), EditConstraint(BG_PRIMARY, SAT, D_ALL),
                        EditConstraint(FG_PRIMARY, VALUE, D_ALL), EditConstraint(BG_PRIMARY, VALUE, D_ALL) 
  } },
  { ALL_PRIMARY_HUE, { EditConstraint(FG_PRIMARY, HUE, D_ALL), EditConstraint(BG_PRIMARY, HUE, D_ALL) } },
  { ALL_PRIMARY_POSITION, { EditConstraint(FG_PRIMARY, POLAR, D_ALL), EditConstraint(BG_PRIMARY, POLAR, D_ALL),
                            EditConstraint(FG_PRIMARY, AZIMUTH, D_ALL), EditConstraint(BG_PRIMARY, AZIMUTH, D_ALL) 
  } },
  { ALL_PRIMARY_SOFTNESS,{ EditConstraint(FG_PRIMARY, SOFT, D_ALL), EditConstraint(BG_PRIMARY, SOFT, D_ALL) } },
  { ALL_SECONDARY_INTENSITY,{ EditConstraint(FG_SECONDARY, INTENSITY, D_ALL), EditConstraint(BG_SECONDARY, INTENSITY, D_ALL) } },
  { ALL_SECONDARY_COLOR,{ EditConstraint(FG_SECONDARY, HUE, D_ALL), EditConstraint(BG_SECONDARY, HUE, D_ALL),
             EditConstraint(FG_SECONDARY, SAT, D_ALL), EditConstraint(BG_SECONDARY, SAT, D_ALL),
             EditConstraint(FG_SECONDARY, VALUE, D_ALL), EditConstraint(BG_SECONDARY, VALUE, D_ALL)
  } },
  { ALL_SECONDARY_HUE,{ EditConstraint(FG_SECONDARY, HUE, D_ALL), EditConstraint(BG_SECONDARY, HUE, D_ALL) } },
  { ALL_SECONDARY_POSITION,{ EditConstraint(FG_SECONDARY, POLAR, D_ALL), EditConstraint(BG_SECONDARY, POLAR, D_ALL),
             EditConstraint(FG_SECONDARY, AZIMUTH, D_ALL), EditConstraint(BG_SECONDARY, AZIMUTH, D_ALL)
  } },
  { ALL_SECONDARY_SOFTNESS,{ EditConstraint(FG_SECONDARY, SOFT, D_ALL), EditConstraint(BG_SECONDARY, SOFT, D_ALL) } },
  { FG_PRIMARY_ALL, { EditConstraint(FG_SECONDARY, INTENSITY, D_ALL), EditConstraint(FG_PRIMARY, HUE, D_ALL),
                      EditConstraint(FG_SECONDARY, SAT, D_ALL), EditConstraint(FG_PRIMARY, VALUE, D_ALL),
                      EditConstraint(FG_SECONDARY, POLAR, D_ALL), EditConstraint(FG_PRIMARY, AZIMUTH, D_ALL),
                      EditConstraint(FG_SECONDARY, SOFT, D_ALL)
  } }
};

SearchResult::SearchResult() { }

SearchResult::SearchResult(const SearchResult & other) :
  _scene(other._scene), _editHistory(other._editHistory), _objFuncVal(other._objFuncVal)
{
}

SearchResult::~SearchResult() {
}


// Search functions
// ==============================================================================

list<SearchResult*> attributeSearch(map<string, AttributeControllerBase*>& active, int editDepth)
{
  // If there's no active attribute, just leave
  if (active.size() == 0)
    return list<SearchResult*>();

  string log = "Search attribute params: ";
  for (auto& kvp : active) {
    log = log + " {" + kvp.first + " : ";
    if (kvp.second->getStatus() == A_MORE)
      log = log + "MORE";
    if (kvp.second->getStatus() == A_LESS)
      log = log + "LESS";
    if (kvp.second->getStatus() == A_EQUAL)
      log = log + "SAME";
    log = log + "}";
  }
  getRecorder()->log(ACTION, log);

  AttributeSearchThread* t = new AttributeSearchThread(active, editDepth);
  t->runThread();

  list<SearchResult*> scenes = t->getResults();
  delete t;

  return scenes;
}

string editTypeToString(EditType t) {
  switch (t) {
  case ALL:
    return "All";
  case ALL_INTENSITY:
    return "All Intensity";
  case ALL_COLOR:
    return "All Color";
  case ALL_HUE:
    return "All Hue";
  case ALL_POSITION:
    return "All Position";
  case ALL_SOFTNESS:
    return "All Softness";
  case ALL_PRIMARY:
    return "All Primary";
  case ALL_SECONDARY:
    return "All Secondary";
  case ALL_TONER:
    return "All Toner";
  case ALL_AMBIENT:
    return "All Ambient";
  case ALL_PRIMARY_INTENSITY:
    return "All Primary Intensity";
  case ALL_PRIMARY_COLOR:
    return "All Primary Color";
  case ALL_PRIMARY_HUE:
    return "All Primary Hue";
  case ALL_PRIMARY_POSITION:
    return "All Primary Position";
  case ALL_PRIMARY_SOFTNESS:
    return "All Primary Softness";
  case ALL_SECONDARY_INTENSITY:
    return "All Secondary Intensity";
  case ALL_SECONDARY_COLOR:
    return "All Secondary Color";
  case ALL_SECONDARY_HUE:
    return "All Secondary Hue";
  case ALL_SECONDARY_POSITION:
    return "All Secondary Position";
  case ALL_SECONDARY_SOFTNESS:
    return "All Secondary Softness";
  case FG_PRIMARY_ALL:
    return "Foreground Primary All";
  case CLUSTER_CENTER:
    return "Cluster Center";
  default:
    return "";
  }
}

vector<Eigen::VectorXd> clusterResults(list<SearchResult*> results, int c)
{
  // Special case for 1 requested cluster
  if (c == 1) {
    // 1 cluster, center is average of all other elements
    Eigen::VectorXd avg;
    avg.resize((*results.begin())->_scene.size());
    avg.setZero();

    for (auto r : results) {
      avg += r->_scene;
      r->_cluster = 0;
    }
    avg /= (double)results.size();
    
    vector<Eigen::VectorXd> ret;
    ret.push_back(avg);
    return ret;
  }
  else {
    // kmeans setup
    dlib::kcentroid<kernelType> kkmeansKernel(kernelType(), 0.001);
    dlib::kkmeans<kernelType> k(kkmeansKernel);

    vector<sampleType> samples;
    for (auto result : results) {
      samples.push_back(dlib::mat(result->_scene));
    }
    vector<sampleType> centers;

    // Start at or number of clusters specified by centers
    int numCenters = (c < 0) ? 2 : c;

    // Make sure user gave us something reasonable for centers
    if (results.size() < numCenters)
      return vector<Eigen::VectorXd>();

    double msd = INFINITY;

    while (msd > getGlobalSettings()->_clusterDistThreshold) {
      if (numCenters == samples.size()) {
        vector<Eigen::VectorXd> clusterCenters;
        int i = 0;
        for (auto c : results) {
          clusterCenters.push_back(c->_scene);
          c->_cluster = i;
          i++;
        }
        return clusterCenters;
      }

      // clear vectors
      centers.clear();

      // use dlib to get the initial centers
      dlib::pick_initial_centers(numCenters, centers, samples, k.get_kernel());

      // Run kmeans
      dlib::find_clusters_using_kmeans(samples, centers);

      // assign results to clusters and compute distance
      double sumDist = 0;
      int i = 0;
      for (auto r : results) {
        unsigned long center = dlib::nearest_center(centers, samples[i]);
        r->_cluster = center;

        // get the center and compute the distance
        auto centroid = centers[center];
        sumDist += length(centroid - samples[i]);
        i++;
      }

      msd = sumDist / results.size();
      numCenters++;

      // Immediately return if user specified an exact number of clusters
      if (c > 0)
        break;
    }

    // convert centers to eigen representation
    vector<Eigen::VectorXd> clusterCenters;
    for (auto c : centers)
    {
      Eigen::VectorXd eCenter;
      eCenter.resize(c.nr());

      for (int i = 0; i < c.nr(); i++) {
        eCenter[i] = c(i);
      }

      clusterCenters.push_back(eCenter);
    }

    return clusterCenters;
  }
}

vector<Eigen::VectorXd> clusterResults(list<Eigen::VectorXd> results, int c) {
  // kmeans setup
  dlib::kcentroid<kernelType> kkmeansKernel(kernelType(), 0.001);
  dlib::kkmeans<kernelType> k(kkmeansKernel);

  vector<sampleType> samples;
  for (auto result : results) {
    samples.push_back(dlib::mat(result));
  }
  vector<sampleType> centers;

  // Start at or number of clusters specified by centers
  int numCenters = (c < 0) ? 2 : c;

  // Make sure user gave us something reasonable for centers
  if (results.size() < numCenters)
    return vector<Eigen::VectorXd>();

  double msd = INFINITY;

  while (msd > getGlobalSettings()->_clusterDistThreshold) {
    if (numCenters == samples.size()) {
      vector<Eigen::VectorXd> clusterCenters;
      int i = 0;
      for (auto c : results) {
        clusterCenters.push_back(c);
        i++;
      }
      return clusterCenters;
    }

    // clear vectors
    centers.clear();

    // use dlib to get the initial centers
    dlib::pick_initial_centers(numCenters, centers, samples, k.get_kernel());

    // Run kmeans
    dlib::find_clusters_using_kmeans(samples, centers);

    // assign results to clusters and compute distance
    double sumDist = 0;
    for (int i = 0; i < results.size(); i++) {
      unsigned long center = dlib::nearest_center(centers, samples[i]);

      // get the center and compute the distance
      auto centroid = centers[center];
      sumDist += length(centroid - samples[i]);
    }

    msd = sumDist / results.size();
    numCenters++;

    // Immediately return if user specified an exact number of clusters
    if (c > 0)
      break;
  }

  // convert centers to eigen representation
  vector<Eigen::VectorXd> clusterCenters;
  for (auto c : centers)
  {
    Eigen::VectorXd eCenter;
    eCenter.resize(c.nr());

    for (int i = 0; i < c.nr(); i++) {
      eCenter[i] = c(i);
    }

    clusterCenters.push_back(eCenter);
  }

  return clusterCenters;
}

list<SearchResult*> filterResults(list<SearchResult*>& results, vector<Eigen::VectorXd>& centers)
{
  vector<multimap<double, SearchResult*> > res;
  for (int i = 0; i < centers.size(); i++)
    res.push_back(multimap<double, SearchResult*>());

  // Place scenes sorted by distance to center into their clusters
  for (auto r : results) {
    double dist = (r->_scene - centers[r->_cluster]).norm();
    res[r->_cluster].insert(pair<double, SearchResult*>(dist, r));
  }

  // For each cluster
  for (auto& c : res) {
    // starting at the first element
    for (auto it = c.begin(); it != c.end(); it++) {
      // See how close all other elements are
      for (auto it2 = c.begin(); it2 != c.end(); ) {
        if (it == it2) {
          it2++;
          continue;
        }

        double dist = (it->second->_scene - it2->second->_scene).norm();

        // delete element if it's too close
        if (dist < getGlobalSettings()->_clusterDiffThreshold) {
          delete it2->second;
          c.erase(it2++);
        }
        else {
          it2++;
        }
      }
    }
  }

  // put results back in to list
  list<SearchResult*> filteredResults;
  for (auto c : res) {
    for (auto kvp : c) {
      filteredResults.push_back(kvp.second);
    }
  }

  return filteredResults;
}

void filterResults(list<Eigen::VectorXd>& results, double t)
{
  // starting at the first element
  for (auto it = results.begin(); it != results.end(); it++) {
    // See how close all other elements are
    for (auto it2 = results.begin(); it2 != results.end(); ) {
      if (it == it2) {
        it2++;
        continue;
      }
      
      double dist = (*it - *it2).norm();

      // delete element if it's too close
      if (dist < t) {
        it2 = results.erase(it2);
      }
      else {
        it2++;
      }
    }
  }
}

void filterResults(list<SearchResult*>& results, double t)
{
  // starting at the first element
  for (auto it = results.begin(); it != results.end(); it++) {
    // See how close all other elements are
    for (auto it2 = results.begin(); it2 != results.end(); ) {
      if (it == it2) {
        it2++;
        continue;
      }

      double dist = ((*it)->_scene - (*it2)->_scene).norm();

      // delete element if it's too close
      if (dist < t) {
        delete *it2;
        it2 = results.erase(it2);
      }
      else {
        it2++;
      }
    }
  }
}

list<SearchResult*> getClosestScenesToCenters(list<SearchResult*>& results, vector<Eigen::VectorXd>& centers)
{
  vector<multimap<double, SearchResult*> > res;
  for (int i = 0; i < centers.size(); i++)
    res.push_back(multimap<double, SearchResult*>());

  // Place scenes sorted by distance to center into their clusters
  for (auto r : results) {
    double dist = (r->_scene - centers[r->_cluster]).norm();
    res[r->_cluster].insert(pair<double, SearchResult*>(dist, r));
  }

  // put first element in to results vector, delete the rest
  list<SearchResult*> filteredResults;
  for (auto c : res) {
    bool first = true;
    for (auto kvp : c) {
      if (first) {
        filteredResults.push_back(kvp.second);
        first = false;
      }
      else {
        delete kvp.second;
      }
    }
  }

  return filteredResults;
}

Eigen::VectorXd snapshotToVector(Snapshot * s)
{
  // Param order: Intensity, polar, azimuth, R, G, B, Softness (penumbra angle)
  // Device order: Alphabetical
  auto& devices = s->getRigData();
  int numFeats = 7;
  Eigen::VectorXd features;
  features.resize(numFeats * devices.size());
  
  int idx = 0;

  // This is a std::map which is always sorted, and thus always traversed
  // in ascending sort order. We'll use this to construct and reconstruct
  // vectors reliably without knowing a lot of details about the lights
  for (const auto& d : devices) {
    int base = idx * numFeats;
    features[base] = d.second->getParam<LumiverseFloat>("intensity")->asPercent();
    features[base + 1] = d.second->getParam<LumiverseOrientation>("polar")->asPercent();
    features[base + 2] = d.second->getParam<LumiverseOrientation>("azimuth")->asPercent();
    features[base + 3] = d.second->getParam<LumiverseColor>("color")->getColorChannel("Red");
    features[base + 4] = d.second->getParam<LumiverseColor>("color")->getColorChannel("Green");
    features[base + 5] = d.second->getParam<LumiverseColor>("color")->getColorChannel("Blue");
    features[base + 6] = d.second->getParam<LumiverseFloat>("penumbraAngle")->asPercent();
    idx++;
  }

  return features;
}

Snapshot * vectorToSnapshot(Eigen::VectorXd v)
{
  Snapshot* s = new Snapshot(getRig(), nullptr);
  auto devices = s->getRigData();
  int numFeats = 7;

  int idx = 0;

  for (const auto& d : devices) {
    int base = idx * numFeats;
    d.second->getParam<LumiverseFloat>("intensity")->setValAsPercent(v[base]);
    d.second->getParam<LumiverseOrientation>("polar")->setValAsPercent(v[base + 1]);
    d.second->getParam<LumiverseOrientation>("azimuth")->setValAsPercent(v[base + 2]);
    d.second->getParam<LumiverseColor>("color")->setColorChannel("Red", v[base + 3]);
    d.second->getParam<LumiverseColor>("color")->setColorChannel("Green", v[base + 4]);
    d.second->getParam<LumiverseColor>("color")->setColorChannel("Blue", v[base + 5]);
    d.second->getParam<LumiverseFloat>("penumbraAngle")->setValAsPercent(v[base + 6]);
    idx++;
  }

  return s;
}

String vectorToString(Eigen::VectorXd v)
{
  String ret = "";

  bool first = true;
  for (int i = 0; i < v.size(); i++) {
    if (!first) {
      ret += ", ";
    }
    ret += String(v[i]);
    first = false;
  }

  return ret;
}

//=============================================================================

AttributeSearchThread::AttributeSearchThread(map<string, AttributeControllerBase*> active, int editDepth) :
  ThreadWithProgressWindow("Searching", true, true, 2000), _active(active), _editDepth(editDepth)
{
  Rig* rig = getRig();
  _original = new Snapshot(rig, nullptr);

  // flag for special casing searching for the same of a single attribute
  if (_active.size() == 1 && _active.begin()->second->getStatus() == A_EQUAL) {
    _singleSame = true;
  }
  else {
    _singleSame = false;
  }

  setProgress(-1);
}

AttributeSearchThread::~AttributeSearchThread()
{
  // don't delete anything, other objects need the results allocated here.
  // except for internally used variables only
  delete _original;
}

void AttributeSearchThread::run()
{
  _results.clear();
  SearchResult* start = new SearchResult();
  start->_scene = snapshotToVector(_original);
  _results.push_back(start);

  // objective function for combined set of active attributes.
  attrObjFunc f = [this](Snapshot* s) {
    double sum = 0;
    for (const auto& kvp : _active) {
      if (kvp.second->getStatus() == A_LESS)
        sum += kvp.second->evaluateScene(s);
      else if (kvp.second->getStatus() == A_MORE)
        sum -= kvp.second->evaluateScene(s);
      else if (kvp.second->getStatus() == A_EQUAL)
        sum += pow(kvp.second->evaluateScene(s) - kvp.second->evaluateScene(_original), 2);
    }

    return sum;
  };

  if (_singleSame) {
    // TODO: Reimplement single same for arbitrary number of devices
    // Reassign starting scene is working with same for a single attribute
    //Snapshot* sStart = new Snapshot(*_original);
    //for (auto c : editConstraints[ALL]) {
    //  if (!isParamLocked(c, ALL, sStart, )) {
    //    double dx = numericDeriv(c, ALL, sStart, f);
    //    double x = getDeviceValue(c, sStart);
    //    setDeviceValue(c, ALL, x - dx * 0.5, sStart);
    //  }
    //}
    //start->_scene = snapshotToVector(sStart);
    //delete sStart;
  }

  // save the original attribute function valuee
  _fc = f(_original);

  for (int i = 0; i < _editDepth; i++) {
    setProgress((float)i / _editDepth);

    list<SearchResult*> newResults = runSingleLevelSearch(_results, i, f);

    // delete old results
    for (auto r : _results)
    {
      delete r;
    }
    _results.clear();

    // cluster and filter for final run
    if (i == _editDepth - 1) {
      filterResults(newResults, getGlobalSettings()->_jndThreshold);
      _results = newResults;
    }
    else {
      auto centers = clusterResults(newResults, getGlobalSettings()->_numEditScenes);
      auto filtered = getClosestScenesToCenters(newResults, centers);
      _results = filtered;
    }

    if (threadShouldExit()) {
      // delete results before cancelling search
      for (auto r : _results)
      {
        delete r;
      }
      _results.clear();
      
      return;
    }
  }

  setProgress(1);
}

void AttributeSearchThread::threadComplete(bool userPressedCancel)
{
  if (userPressedCancel) {
    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
      "Exploratory Search",
      "Search canceled");
  }
}

list<SearchResult*> AttributeSearchThread::runSingleLevelSearch(list<SearchResult*> startScenes, int level, attrObjFunc f)
{
  list<SearchResult*> searchResults;

  float seqPct = ((float)level / _editDepth);

  int totalOps = startScenes.size() * editConstraints.size();
  int opCt = 0;

  int i = 0, j = 0;

  // For each scene in the initial set
  for (const auto& scene : startScenes) {
    // For each edit, get a list of scenes returned and just add it to the overall list.
    j = 0;
    for (const auto& edits : editConstraints) {
      opCt++;
      setStatusMessage("Level " + String(level) + "\nScene " + String(i+1) + "/" + String(startScenes.size()) + "\nRunning Edit " + String(j+1) + "/" + String(editConstraints.size()));
      list<Eigen::VectorXd> editScenes = performEdit(edits.first, vectorToSnapshot(scene->_scene), f);
      
      if (threadShouldExit())
        return list<SearchResult*>();

      for (auto s : editScenes) {
        SearchResult* r = new SearchResult();
        r->_scene = s;
        r->_editHistory.addArray(scene->_editHistory);
        r->_editHistory.add(edits.first);
        Snapshot* sn = vectorToSnapshot(s);
        r->_objFuncVal = f(sn);
        delete sn;

        // We evaluate the function value on demand and just save the function itself
        searchResults.push_back(r);
      }

      setProgress(seqPct + ((float)opCt/(float)totalOps)* (1.0/_editDepth));
      j++;
    }
    i++;
  }

  return searchResults;
}

list<Eigen::VectorXd> AttributeSearchThread::performEdit(EditType t, Snapshot* orig, attrObjFunc f) {
  // Determine accept parameters
  double targetAcceptRate = 0.5;  // +/- 5%
  double sigma = 0.05;
  // limit number of tuning iterations
  //for (int i = 0; i < 20; i++) {
  //  auto res = doMCMC(t, orig, f, 100, sigma, false);
  //  double acceptRate = res.second / 100.0;
    
  //  if (abs(acceptRate - targetAcceptRate) <= 0.05) {
  //    break;
  //  }
    
  //  if (acceptRate > targetAcceptRate)
  //    sigma -= 0.005;
  //  if (acceptRate < targetAcceptRate)
  //    sigma += 0.01;

  //  if (sigma <= 0) {
      // if we can't solve the problem with sigma, for now we'll just set it to default and continue
  //    sigma = 0.05;
  //    break;
  // }
  //}

  auto res = doMCMC(t, orig, f, getGlobalSettings()->_maxMCMCIters, sigma, true);
  delete orig;
  return res.first;
}

pair<list<Eigen::VectorXd>, int> AttributeSearchThread::doMCMC(EditType t, Snapshot * start, attrObjFunc f, int iters, double sigma, bool saveSamples)
{
  // duplicate initial state
  Snapshot* s = new Snapshot(*start);

  // Set up return list
  list<Eigen::VectorXd> results;

  // RNG
  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
  default_random_engine gen(seed1);
  normal_distribution<double> gdist(0, getGlobalSettings()->_editStepSize);  // start with sdev 2
  uniform_real_distribution<double> udist(0.0, 1.0);

  // Constants
  double minEditDist = getGlobalSettings()->_minEditDist;
  int maxIters = iters;

  // Set up relevant feature vector
  // Length of feature vector determined by type and number of devices in the rig
  int vecSize = getVecLength(t, start);
  Eigen::VectorXd x;
  x.resize(vecSize);

  // Parameter restrictions
  vector<bool> canEdit;
  bool cantEditAll = true;

  // This map is now necessary to track which devices correspond to which parameters in the
  // vector used in the search space, and tells the system how to modify the parameter during
  // search
  map<int, DeviceInfo> deviceLookup;

  int i = 0;
  for (auto& c : editConstraints[t]) {
    // Add all device parameters individually to the list
    auto devices = getDeviceGroup(c._light).getDevices();
    if (c._qty == D_ALL) {
      for (auto d : devices) {
        x[i] = getDeviceValue(c, s, d->getId());
        deviceLookup[i] = DeviceInfo(c, d->getId());
        bool lock = isParamLocked(c, t, s, d->getId());
        canEdit.push_back(!lock);

        cantEditAll = cantEditAll & lock;
        i++;
      }
    }
    // Joint adds a delta to every light's parameter, while leaving the
    // starting value unaffected.
    else if (c._qty == D_JOINT) {
      x[i] = 0;
      deviceLookup[i] = DeviceInfo(c, string());
      i++;

      // Check for locks
      for (auto d : devices) {
        bool lock = isParamLocked(c, t, s, d->getId());
        canEdit.push_back(!lock);

        cantEditAll = cantEditAll & lock;
      }
    }
    // Single runs a separate search for every device in the rig
  }

  // if we can't actually edit any parameters at all just exit now
  if (cantEditAll) {
    delete s;
    return pair<list<Eigen::VectorXd>, int>(results, 0);
  }

  // diagnostics
  int accepted = 0;
  Snapshot* sp = new Snapshot(*start);
  double fx = f(s);

  for (int i = 0; i < maxIters; i++) {
    // generate candidate x'
    Eigen::VectorXd xp = x;

    // displace by gaussian dist
    for (int j = 0; j < xp.size(); j++) {
      if (canEdit[j]) {
        xp[j] += gdist(gen);
        if (deviceLookup[j]._c._qty == D_JOINT) {
          // Joint adds the delta (xp[j]) to the start value to get the new value
          // for all devices affected by the joint param
          auto devices = getDeviceGroup(deviceLookup[j]._c._light).getDevices();
          for (auto& d : devices) {
            double initVal = getDeviceValue(deviceLookup[i]._c, start, d->getId());
            setDeviceValue(deviceLookup[j], xp[j] + initVal, sp);
          }
        }
        else {
          // The next line acts as a physically based clamp function of sorts.
          // It updates the lighting scene and also returns the value of the parameter after the update.
          xp[j] = setDeviceValue(deviceLookup[j], xp[j], sp);
        }
      }
    }

    // check for acceptance
    double fxp = f(sp);
    double diff = abs(fxp - fx);
    double a = 0;

    if (getGlobalSettings()->_randomMode || fxp < fx)
      a = 1;
    else {
      // Rescale a based on normal distribution with a std dev decided on by
      // tuning (or in this case, compeltely arbitrarily for now)
      a = 1 - (0.5 * erfc(-diff / (sqrt(2) * sigma)) - 0.5 * erfc(-diff / (sqrt(2) * -sigma)));
    }

    // accept if a >= 1 or with probability a
    if (a >= 1 || udist(gen) <= a) {
      if (_singleSame) {
        if (saveSamples && abs(fxp - _fc) < 2) {
          // save sample in list
          results.push_back(snapshotToVector(sp));
        }
      }
      else {
        if (saveSamples && fxp < _fc && abs(fxp - _fc) >= minEditDist) {
          // save sample in list
          results.push_back(snapshotToVector(sp));
        }
      }
      // update x
      x = xp;
      accepted++;
      fx = fxp;
    }
  }

  if (s != nullptr)
    delete s;
  if (s != sp && sp != nullptr)
    delete sp;

  if (saveSamples)
    getRecorder()->log(SYSTEM, "[Debug] " + editTypeToString(t) + " accepted " + String(((float)accepted / (float)maxIters) * 100).toStdString() + "% of proposals");

  // filter results
  filterResults(results, getGlobalSettings()->_jndThreshold);

  return pair<list<Eigen::VectorXd>, int>(results, accepted);
}

double AttributeSearchThread::numericDeriv(EditConstraint c, Snapshot* s, attrObjFunc f, string& id)
{
  // load the appropriate settings and get the proper device.
  double h = getGlobalSettings()->_searchDerivDelta;
  Device* d = s->getRigData()[id];
  double f1 = f(s);
  double f2;

  switch (c._param) {
  case INTENSITY:
  {
    if (isDeviceParamLocked(d->getId(), "intensity"))
      return 0;

    float o = d->getIntensity()->asPercent();
    if (o >= 1)
      h = -h;

    d->getIntensity()->setValAsPercent(o + h);
    f2 = f(s);
    d->getIntensity()->setValAsPercent(o);
    break;
  }
  case HUE:
  {
    if (isDeviceParamLocked(d->getId(), "color"))
      return 0;

    Eigen::Vector3d hsv = d->getColor()->getHSV();
    float huePct = hsv[0] / 360.0;

    // hue wraps around, derivative should be fine if hue is at max/min
    d->getColor()->setHSV((huePct + h) * 360.0, hsv[1], hsv[2]);
    f2 = f(s);
    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2]);

    break;
  }
  case SAT:
  {
    if (isDeviceParamLocked(d->getId(), "color"))
      return 0;

    Eigen::Vector3d hsv = d->getColor()->getHSV();
    if (hsv[1] >= 1)
      h = -h;

    d->getColor()->setHSV(hsv[0], hsv[1] + h, hsv[2]);
    f2 = f(s);
    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2]);
    break;
  }
  case VALUE:
  {
    if (isDeviceParamLocked(d->getId(), "color"))
      return 0;

    Eigen::Vector3d hsv = d->getColor()->getHSV();
    if (hsv[2] >= 1)
      h = -h;

    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2] + h);
    f2 = f(s);
    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2]);
    break;
  }
  case RED:
  {
    if (isDeviceParamLocked(d->getId(), "color"))
      return 0;

    double r = d->getColor()->getColorChannel("Red");
    if (r >= 1)
      h = -h;

    d->getColor()->setColorChannel("Red", r + h);
    f2 = f(s);
    d->getColor()->setColorChannel("Red", r);
    break;
  }
  case BLUE:
  {
    if (isDeviceParamLocked(d->getId(), "color"))
      return 0;

    double b = d->getColor()->getColorChannel("Blue");
    if (b >= 1)
      h = -h;

    d->getColor()->setColorChannel("Blue", b + h);
    f2 = f(s);
    d->getColor()->setColorChannel("Blue", b);
    break;
  }
  case GREEN:
  {
    if (isDeviceParamLocked(d->getId(), "color"))
      return 0;

    double g = d->getColor()->getColorChannel("Green");
    if (g >= 1)
      h = -h;

    d->getColor()->setColorChannel("Green", g + h);
    f2 = f(s);
    d->getColor()->setColorChannel("Green", g);
    break;
  }
  case POLAR:
  {
    if (isDeviceParamLocked(d->getId(), "polar"))
      return 0;

    LumiverseOrientation* val = (LumiverseOrientation*)d->getParam("polar");
    float p = val->asPercent();
    if (p >= 1)
      h = -h;

    val->setValAsPercent(p + h);
    f2 = f(s);
    val->setValAsPercent(p);

    break;
  }
  case AZIMUTH:
  {
    if (isDeviceParamLocked(d->getId(), "azimuth"))
      return 0;

    LumiverseOrientation* val = (LumiverseOrientation*)d->getParam("azimuth");
    float a = val->asPercent();
    if (a >= 1)
      h = -h;

    val->setValAsPercent(a + h);
    f2 = f(s);
    val->setValAsPercent(a);

    break;
  }
  default:
    break;
  }

  return (f2 - f1) / h;
}

double AttributeSearchThread::setDeviceValue(DeviceInfo& info, double val, Snapshot * s)
{
  Device* d = s->getRigData()[info._id];
  
  switch (info._c._param) {
  case INTENSITY:
  {
    d->getIntensity()->setValAsPercent(val);
    return d->getIntensity()->asPercent();
  }
  case HUE:
  {
    val *= 360;
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(val, hsv[1], hsv[2]);
    return d->getColor()->getHSV()[0] / 360.0;
  }
  case SAT:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(hsv[0], val, hsv[2]);
    return d->getColor()->getHSV()[1];
  }
  case VALUE:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(hsv[0], hsv[1], val);
    return d->getColor()->getHSV()[2];
  }
  case RED:
    d->getColor()->setColorChannel("Red", val);
    return d->getColor()->getColorChannel("Red");
  case BLUE:
    d->getColor()->setColorChannel("Blue", val);
    return d->getColor()->getColorChannel("Blue");
  case GREEN:
    d->getColor()->setColorChannel("Green", val);
    return d->getColor()->getColorChannel("Green");
  case POLAR:
  {
    LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("polar");
    o->setValAsPercent(val);
    return o->asPercent();
  }
  case AZIMUTH:
  {
    LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("azimuth");
    o->setValAsPercent(val);

    return o->asPercent();
  }
  case SOFT:
  {
    LumiverseFloat* s = d->getParam<LumiverseFloat>("penumbraAngle");
    s->setValAsPercent(val);
    return s->asPercent();
  }
  default:
    return 0;
  }

}

double AttributeSearchThread::getDeviceValue(EditConstraint c, Snapshot * s, string& id)
{
  Device* d = s->getRigData()[id];

  switch (c._param) {
  case INTENSITY:
    return d->getIntensity()->asPercent();
  case HUE:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    return hsv[0] / 360.0;
  }
  case SAT:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    return hsv[1];
  }
  case VALUE:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    return hsv[2];
  }
  case RED:
    return d->getColor()->getColorChannel("Red");
  case BLUE:
    return d->getColor()->getColorChannel("Blue");
  case GREEN:
    return d->getColor()->getColorChannel("Green");
  case POLAR:
  {
    LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("polar");
    return o->asPercent();
  }
  case AZIMUTH:
  {
    LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("azimuth");
    return o->asPercent();
  }
  case SOFT:
  {
    LumiverseFloat* s = d->getParam<LumiverseFloat>("penumbraAngle");
    return s->asPercent();
  }
  default:
    break;
  }
}

bool AttributeSearchThread::isParamLocked(EditConstraint c, EditType t, Snapshot* s, string& id)
{
  Device* d = s->getRigData()[id];

  switch (c._param) {
  case INTENSITY:
    return isDeviceParamLocked(d->getId(), "intensity");
  case HUE:
    return isDeviceParamLocked(d->getId(), "color");
  case SAT:
    return isDeviceParamLocked(d->getId(), "color");
  case VALUE:
    return isDeviceParamLocked(d->getId(), "color");
  case RED:
    return isDeviceParamLocked(d->getId(), "color");
  case GREEN:
    return isDeviceParamLocked(d->getId(), "color");
  case BLUE:
    return isDeviceParamLocked(d->getId(), "color");
  case POLAR:
    return isDeviceParamLocked(d->getId(), "polar");
  case AZIMUTH:
    return isDeviceParamLocked(d->getId(), "azimuth");
  case SOFT:
    return isDeviceParamLocked(d->getId(), "penumbraAngle");
  default:
    return false;
  }
}

int AttributeSearchThread::getVecLength(EditType t, Snapshot * s)
{
  int size = 0;
  for (auto& c : editConstraints[t]) {
    if (c._qty == D_ALL) {
      size += getDeviceGroup(c._light).getDevices().size();
    }
    else if (c._qty == D_JOINT) {
      size += 1;
    }
  }

  return size;
}