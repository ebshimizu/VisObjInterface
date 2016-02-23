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
  { ALL, { EditConstraint(L_KEY, RED), EditConstraint(L_KEY, GREEN), EditConstraint(L_KEY, BLUE),
           EditConstraint(L_KEY, INTENSITY), EditConstraint(L_KEY, POLAR), EditConstraint(L_KEY, AZIMUTH),
           EditConstraint(L_FILL, RED), EditConstraint(L_FILL, GREEN), EditConstraint(L_FILL, BLUE),
           EditConstraint(L_FILL, INTENSITY), EditConstraint(L_FILL, POLAR), EditConstraint(L_FILL, AZIMUTH),
           EditConstraint(L_RIM, RED), EditConstraint(L_RIM, GREEN), EditConstraint(L_RIM, BLUE),
           EditConstraint(L_RIM, INTENSITY), EditConstraint(L_RIM, POLAR), EditConstraint(L_RIM, AZIMUTH),
           EditConstraint(L_KEY, SOFT), EditConstraint(L_FILL, SOFT), EditConstraint(L_RIM, SOFT) } },
  { ALL_HSV, { EditConstraint(L_KEY, HUE), EditConstraint(L_KEY, SAT), EditConstraint(L_KEY, VALUE),
               EditConstraint(L_FILL, HUE), EditConstraint(L_FILL, SAT), EditConstraint(L_FILL, VALUE), 
               EditConstraint(L_RIM, HUE), EditConstraint(L_RIM, SAT), EditConstraint(L_RIM, VALUE) } },
  { ALL_RGB, { EditConstraint(L_KEY, RED), EditConstraint(L_KEY, GREEN), EditConstraint(L_KEY, BLUE),
               EditConstraint(L_FILL, RED), EditConstraint(L_FILL, GREEN), EditConstraint(L_FILL, BLUE),
               EditConstraint(L_RIM, RED), EditConstraint(L_RIM, GREEN), EditConstraint(L_RIM, BLUE) } },
  { ALL_SAT, { EditConstraint(L_KEY, SAT), EditConstraint(L_FILL, SAT), EditConstraint(L_RIM, SAT) } },
  { ALL_HUE, { EditConstraint(L_KEY, HUE), EditConstraint(L_FILL, HUE), EditConstraint(L_RIM, HUE) } },
  { ALL_INTENS, { EditConstraint(L_KEY, INTENSITY), EditConstraint(L_FILL, INTENSITY), EditConstraint(L_RIM, INTENSITY) } },
  { ALL_POS, { EditConstraint(L_KEY, POLAR), EditConstraint(L_FILL, POLAR), EditConstraint(L_RIM, POLAR),
               EditConstraint(L_KEY, AZIMUTH), EditConstraint(L_FILL, AZIMUTH), EditConstraint(L_RIM, AZIMUTH) } },
  { KEY_HUE, { EditConstraint(L_KEY, HUE) } },
  { FILL_HUE, { EditConstraint(L_FILL, HUE) } },
  { RIM_HUE, { EditConstraint(L_RIM, HUE) } },
  { KEY_INTENS, { EditConstraint(L_KEY, INTENSITY) } },
  { FILL_INTENS, { EditConstraint(L_FILL, INTENSITY) } },
  { RIM_INTENS, { EditConstraint(L_RIM, INTENSITY) } },
  { KEY_POS, { EditConstraint(L_KEY, AZIMUTH), EditConstraint(L_KEY, POLAR) } },
  { FILL_POS, { EditConstraint(L_FILL, AZIMUTH), EditConstraint(L_FILL, POLAR) } },
  { RIM_POS, { EditConstraint(L_RIM, AZIMUTH), EditConstraint(L_RIM, POLAR) } },
  { KEY_SAT, { EditConstraint(L_KEY, SAT) } },
  { FILL_SAT, { EditConstraint(L_FILL, SAT) } },
  { RIM_SAT, { EditConstraint(L_RIM, SAT) } },
  { KEY_HSV, { EditConstraint(L_KEY, HUE), EditConstraint(L_KEY, SAT), EditConstraint(L_KEY, VALUE) } },
  { FILL_HSV, { EditConstraint(L_FILL, HUE), EditConstraint(L_FILL, SAT), EditConstraint(L_FILL, VALUE) } },
  { RIM_HSV, { EditConstraint(L_RIM, HUE), EditConstraint(L_RIM, SAT), EditConstraint(L_RIM, VALUE) } },
  { KEY_SOFT, { EditConstraint(L_KEY, SOFT) } },
  { FILL_SOFT, { EditConstraint(L_FILL, SOFT) } },
  { RIM_SOFT, { EditConstraint(L_RIM, SOFT) } },
  { ALL_SOFT, { EditConstraint(L_KEY, SOFT), EditConstraint(L_FILL, SOFT), EditConstraint(L_RIM, SOFT) } },
  { KEY_FILL_INTENS, { EditConstraint(L_KEY, INTENSITY), EditConstraint(L_FILL, INTENSITY) } },
  { KEY_RIM_INTENS, { EditConstraint(L_KEY, INTENSITY), EditConstraint(L_RIM, INTENSITY) } },
  { FILL_RIM_INTENS, { EditConstraint(L_FILL, INTENSITY), EditConstraint(L_RIM, INTENSITY) } },
  { KEY_FILL_HUE, { EditConstraint(L_KEY, HUE), EditConstraint(L_FILL, HUE) } },
  { KEY_FILL_SAT, { EditConstraint(L_KEY, SAT), EditConstraint(L_FILL, SAT) } },
  { KEY_FILL_HSV, { EditConstraint(L_KEY, HUE), EditConstraint(L_KEY, SAT), EditConstraint(L_KEY, VALUE),
                    EditConstraint(L_FILL, HUE), EditConstraint(L_FILL, SAT), EditConstraint(L_FILL, VALUE) } },
  { KEY_FILL_POS, { EditConstraint(L_KEY, POLAR), EditConstraint(L_KEY, AZIMUTH), EditConstraint(L_FILL, POLAR), EditConstraint(L_FILL, POLAR) } },
  { KEY_RIM_HSV, { EditConstraint(L_KEY, HUE), EditConstraint(L_KEY, SAT), EditConstraint(L_KEY, VALUE),
                   EditConstraint(L_RIM, HUE), EditConstraint(L_RIM, SAT), EditConstraint(L_RIM, VALUE) } },
  { KEY_RIM_POS, { EditConstraint(L_KEY, POLAR), EditConstraint(L_KEY, AZIMUTH), EditConstraint(L_RIM, POLAR), EditConstraint(L_RIM, AZIMUTH) } },
  { KEY_RGB, { EditConstraint(L_KEY, RED), EditConstraint(L_KEY, GREEN), EditConstraint(L_KEY, BLUE) } },
  { FILL_RGB, { EditConstraint(L_FILL, RED), EditConstraint(L_FILL, GREEN), EditConstraint(L_FILL, BLUE) } },
  { RIM_RGB, { EditConstraint(L_RIM, RED), EditConstraint(L_RIM, GREEN), EditConstraint(L_RIM, BLUE) } },
  { KEY_POS_FILL_POS_MATCH, { EditConstraint(L_KEY, AZIMUTH), EditConstraint(L_KEY, POLAR) } },
  { KEY_INTENS_RIM_CONTRAST_MATCH, { EditConstraint(L_KEY, INTENSITY) } },
  { KEY_INTENS_FILL_CONTRAST_MATCH, { EditConstraint(L_KEY, INTENSITY) } },
  { KEY_HUE_FILL_HUE_MATCH, { EditConstraint(L_KEY, HUE) } },
  { KEY_HUE_FILL_RIM_HUE_MATCH, { EditConstraint(L_KEY, HUE) } }
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
  case ALL_HSV:
    return "All HSV";
  case ALL_RGB:
    return "All RGB";
  case ALL_SAT:
    return "All Saturation";
  case ALL_HUE:
    return "All Hue";
  case ALL_INTENS:
    return "All Intensity";
  case ALL_POS:
    return "All Position";
  case KEY_HUE:
    return "Key Hue";
  case FILL_HUE:
    return "Fill Hue";
  case RIM_HUE:
    return "Rim Hue";
  case KEY_INTENS:
    return "Key Intensity";
  case FILL_INTENS:
    return "Fill Intensity";
  case RIM_INTENS:
    return "Rim Intensity";
  case KEY_POS:
    return "Key Position";
  case FILL_POS:
    return "Fill Position";
  case RIM_POS:
    return "Rim Position";
  case KEY_SAT:
    return "Key Saturation";
  case FILL_SAT:
    return "Fill Saturation";
  case RIM_SAT:
    return "Rim Saturation";
  case KEY_HSV:
    return "Key HSV";
  case FILL_HSV:
    return "Fill HSV";
  case RIM_HSV:
    return "Rim HSV";
  case KEY_SOFT:
    return "Key Softness";
  case FILL_SOFT:
    return "Fill Softness";
  case RIM_SOFT:
    return "Rim Softness";
  case ALL_SOFT:
    return "All Softness";
  case KEY_FILL_INTENS:
    return "Key-Fill Intensity";
  case KEY_RIM_INTENS:
    return "Key-Rim Intensity";
  case FILL_RIM_INTENS:
    return "Fill-Rim Intensity";
  case KEY_FILL_HUE:
    return "Key-Fill Hue";
  case KEY_FILL_SAT:
    return "Key-Fill Saturation";
  case KEY_FILL_HSV:
    return "Key-Fill HSV";
  case KEY_FILL_POS:
    return "Key-Fill Position";
  case KEY_RIM_HSV:
    return "Key-Rim HSV";
  case KEY_RIM_POS:
    return "Key-Rim Position";
  case KEY_RGB:
    return "Key RGB";
  case FILL_RGB:
    return "Fill RGB";
  case RIM_RGB:
    return "Rim RGB";
  case KEY_POS_FILL_POS_MATCH:
    return "Key Position (Fill Match)";
  case KEY_INTENS_RIM_CONTRAST_MATCH:
    return "Key Intensity (Rim Match)";
  case KEY_INTENS_FILL_CONTRAST_MATCH:
    return "Key Intensity (Fill Match)";
  case KEY_HUE_FILL_HUE_MATCH:
    return "Key Hue (Fill Match)";
  case KEY_HUE_FILL_RIM_HUE_MATCH:
    return "Key Hue (Fill, Rim Match)";
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
  // Device order: Key, Fill, Rim, L/R indicator
  int numFeats = 7;
  Eigen::VectorXd features;
  features.resize(numFeats * 3 + 1);
  
  auto& device = s->getRigData();

  auto key = getSpecifiedDevice(L_KEY, s);
  auto fill = getSpecifiedDevice(L_FILL, s);
  auto rim = getSpecifiedDevice(L_RIM, s);

  // Normalize features if needed
  features[0] = key->getParam<LumiverseFloat>("intensity")->asPercent();
  features[1] = key->getParam<LumiverseOrientation>("polar")->asPercent();
  features[2] = key->getParam<LumiverseOrientation>("azimuth")->asPercent();
  features[3] = key->getParam<LumiverseColor>("color")->getColorChannel("Red");
  features[4] = key->getParam<LumiverseColor>("color")->getColorChannel("Green");
  features[5] = key->getParam<LumiverseColor>("color")->getColorChannel("Blue");
  features[6] = key->getParam<LumiverseFloat>("penumbraAngle")->asPercent();
  features[7] = fill->getParam<LumiverseFloat>("intensity")->asPercent();
  features[8] = fill->getParam<LumiverseOrientation>("polar")->asPercent();
  features[9] = fill->getParam<LumiverseOrientation>("azimuth")->asPercent();
  features[10] = fill->getParam<LumiverseColor>("color")->getColorChannel("Red");
  features[11] = fill->getParam<LumiverseColor>("color")->getColorChannel("Green");
  features[12] = fill->getParam<LumiverseColor>("color")->getColorChannel("Blue");
  features[13] = fill->getParam<LumiverseFloat>("penumbraAngle")->asPercent();
  features[14] = rim->getParam<LumiverseFloat>("intensity")->asPercent();
  features[15] = rim->getParam<LumiverseOrientation>("polar")->asPercent();
  features[16] = rim->getParam<LumiverseOrientation>("azimuth")->asPercent();
  features[17] = rim->getParam<LumiverseColor>("color")->getColorChannel("Red");
  features[18] = rim->getParam<LumiverseColor>("color")->getColorChannel("Green");
  features[19] = rim->getParam<LumiverseColor>("color")->getColorChannel("Blue");
  features[20] = rim->getParam<LumiverseFloat>("penumbraAngle")->asPercent();
  features[21] = (key->getId() == "right") ? 1e-6 : -1e-6;  // tiny little sign bit for recreating snapshot

  return features;
}

Snapshot * vectorToSnapshot(Eigen::VectorXd v)
{
  Snapshot* s = new Snapshot(getRig(), nullptr);
  auto devices = s->getRigData();

  auto key = (v[21] > 0) ? devices["right"] : devices["left"];
  auto fill = (v[21] > 0) ? devices["left"] : devices["right"];
  auto rim = devices["rim"];

  key->getParam<LumiverseFloat>("intensity")->setValAsPercent(v[0]);
  key->getParam<LumiverseOrientation>("polar")->setValAsPercent(v[1]);
  key->getParam<LumiverseOrientation>("azimuth")->setValAsPercent(v[2]);
  key->getParam<LumiverseColor>("color")->setColorChannel("Red", v[3]);
  key->getParam<LumiverseColor>("color")->setColorChannel("Green", v[4]);
  key->getParam<LumiverseColor>("color")->setColorChannel("Blue", v[5]);
  key->getParam<LumiverseFloat>("penumbraAngle")->setValAsPercent(v[6]);
  fill->getParam<LumiverseFloat>("intensity")->setValAsPercent(v[7]);
  fill->getParam<LumiverseOrientation>("polar")->setValAsPercent(v[8]);
  fill->getParam<LumiverseOrientation>("azimuth")->setValAsPercent(v[9]);
  fill->getParam<LumiverseColor>("color")->setColorChannel("Red", v[10]);
  fill->getParam<LumiverseColor>("color")->setColorChannel("Green", v[11]);
  fill->getParam<LumiverseColor>("color")->setColorChannel("Blue", v[12]);
  fill->getParam<LumiverseFloat>("penumbraAngle")->setValAsPercent(v[13]);
  rim->getParam<LumiverseFloat>("intensity")->setValAsPercent(v[14]);
  rim->getParam<LumiverseOrientation>("polar")->setValAsPercent(v[15]);
  rim->getParam<LumiverseOrientation>("azimuth")->setValAsPercent(v[16]);
  rim->getParam<LumiverseColor>("color")->setColorChannel("Red", v[17]);
  rim->getParam<LumiverseColor>("color")->setColorChannel("Green", v[18]);
  rim->getParam<LumiverseColor>("color")->setColorChannel("Blue", v[19]);
  rim->getParam<LumiverseFloat>("penumbraAngle")->setValAsPercent(v[20]);

  return s;
}

Device* getSpecifiedDevice(EditLightType l, Snapshot * s)
{
  auto& devices = s->getRigData();

  // Rim is easy to identify
  if (l == L_RIM)
    return devices["rim"];

  Device* key = devices["right"];
  Device* fill = devices["left"];

  // If right is key assumption is false, swap
  if (fill->getIntensity()->getVal() > key->getIntensity()->getVal()) {
    Device* temp = key;
    key = fill;
    fill = temp;
  }

  if (l == L_FILL)
    return fill;
  else if (l == L_KEY)
    return key;

  // If this ever happens, something's gone terribly wrong
  return nullptr;
}

//=============================================================================

AttributeSearchThread::AttributeSearchThread(map<string, AttributeControllerBase*> active, int editDepth) :
  ThreadWithProgressWindow("Searching", true, true, 2000), _active(active), _editDepth(editDepth)
{
  Rig* rig = getRig();
  _original = new Snapshot(rig, nullptr);

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
        sum += kvp.second->evaluateScene(s->getRigData());
      else if (kvp.second->getStatus() == A_MORE)
        sum -= kvp.second->evaluateScene(s->getRigData());
      else if (kvp.second->getStatus() == A_EQUAL)
        sum += pow(kvp.second->evaluateScene(s->getRigData()) - kvp.second->evaluateScene(_original->getRigData()), 2);
    }

    return sum;
  };

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
  for (int i = 0; i < 20; i++) {
    auto res = doMCMC(t, orig, f, 100, sigma, false);
    double acceptRate = res.second / 100.0;
    
    if (abs(acceptRate - targetAcceptRate) <= 0.05) {
      break;
    }
    
    if (acceptRate > targetAcceptRate)
      sigma -= 0.005;
    if (acceptRate < targetAcceptRate)
      sigma += 0.01;

    if (sigma <= 0) {
      // if we can't solve the problem with sigma, for now we'll just set it to default and continue
      sigma = 0.05;
      break;
   }
  }

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
  int vecSize = editConstraints[t].size();
  Eigen::VectorXd x;
  x.resize(vecSize);

  // Parameter restrictions
  vector<bool> canEdit;
  bool cantEditAll = true;

  int i = 0;
  for (const auto& c : editConstraints[t]) {
    x[i] = getDeviceValue(c, s);
    bool lock = isParamLocked(c, t, s);
    canEdit.push_back(!lock);
    cantEditAll = cantEditAll & lock;
    i++;
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
        // The next line acts as a physically based clamp function of sorts.
        // It updates the lighting scene and also returns the value of the parameter after the update.
        xp[j] = setDeviceValue(editConstraints[t][j], t, xp[j], sp);
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
      if (saveSamples && fxp < _fc && abs(fxp - _fc) >= minEditDist) {
        // save sample in list
        results.push_back(snapshotToVector(sp));
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

double AttributeSearchThread::numericDeriv(EditConstraint c, EditType t, Snapshot* s, attrObjFunc f)
{
  // load the appropriate settings and get the proper device.
  double h = getGlobalSettings()->_searchDerivDelta;
  Device* d = getSpecifiedDevice(c._light, s);
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

    if (c._light == L_KEY) {
      if (t == KEY_INTENS_RIM_CONTRAST_MATCH) {
        Device* rim = getSpecifiedDevice(L_RIM, s);
        double r = rim->getIntensity()->asPercent() / o;
        rim->getIntensity()->setValAsPercent((o * r) + h);
      }
      else if (t == KEY_INTENS_FILL_CONTRAST_MATCH) {
        Device* fill = getSpecifiedDevice(L_FILL, s);
        double r = fill->getIntensity()->asPercent() / o;
        fill->getIntensity()->setValAsPercent((o * r) + h);
      }
    }

    d->getIntensity()->setValAsPercent(o + h);
    f2 = f(s);
    d->getIntensity()->setValAsPercent(o);

    if (c._light == L_KEY) {
      if (t == KEY_INTENS_RIM_CONTRAST_MATCH) {
        Device* rim = getSpecifiedDevice(L_RIM, s);
        double r = rim->getIntensity()->asPercent() / o;
        rim->getIntensity()->setValAsPercent(o * r);
      }
      else if (t == KEY_INTENS_FILL_CONTRAST_MATCH) {
        Device* fill = getSpecifiedDevice(L_FILL, s);
        double r = fill->getIntensity()->asPercent() / o;
        fill->getIntensity()->setValAsPercent((o * r));
      }
    }

    break;
  }
  case HUE:
  {
    if (isDeviceParamLocked(d->getId(), "colorH"))
      return 0;

    Eigen::Vector3d hsv = d->getColor()->getHSV();
    float huePct = hsv[0] / 360.0;

    if (c._light == L_KEY) {
      if (t == KEY_HUE_FILL_HUE_MATCH || t == KEY_HUE_FILL_RIM_HUE_MATCH) {
        auto fill = getSpecifiedDevice(L_FILL, s);
        Eigen::Vector3d fhsv = fill->getColor()->getHSV();
        float fhuePct = fhsv[0] / 360.0;
        fill->getColor()->setHSV((fhuePct + h) * 360.0, fhsv[1], fhsv[2]);
      }
      if (t == KEY_HUE_FILL_RIM_HUE_MATCH) {
        auto rim = getSpecifiedDevice(L_RIM, s);
        Eigen::Vector3d rhsv = rim->getColor()->getHSV();
        float rhuePct = rhsv[0] / 360.0;
        rim->getColor()->setHSV((rhuePct + h) * 360.0, rhsv[1], rhsv[2]);
      }
    }

    // hue wraps around, derivative should be fine if hue is at max/min
    d->getColor()->setHSV((huePct + h) * 360.0, hsv[1], hsv[2]);
    f2 = f(s);
    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2]);

    if (c._light == L_KEY) {
      if (t == KEY_HUE_FILL_HUE_MATCH || t == KEY_HUE_FILL_RIM_HUE_MATCH) {
        auto fill = getSpecifiedDevice(L_FILL, s);
        Eigen::Vector3d fhsv = fill->getColor()->getHSV();
        float fhuePct = fhsv[0] / 360.0;
        fill->getColor()->setHSV((fhuePct - h) * 360.0, fhsv[1], fhsv[2]);
      }
      if (t == KEY_HUE_FILL_RIM_HUE_MATCH) {
        auto rim = getSpecifiedDevice(L_RIM, s);
        Eigen::Vector3d rhsv = rim->getColor()->getHSV();
        float rhuePct = rhsv[0] / 360.0;
        rim->getColor()->setHSV((rhuePct - h) * 360, rhsv[1], rhsv[2]);
      }
    }

    break;
  }
  case SAT:
  {
    if (isDeviceParamLocked(d->getId(), "colorS"))
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
    if (isDeviceParamLocked(d->getId(), "colorV"))
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
    if (isDeviceParamLocked(d->getId(), "colorRed"))
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
    if (isDeviceParamLocked(d->getId(), "colorBlue"))
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
    if (isDeviceParamLocked(d->getId(), "colorGreen"))
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

    if (c._light == L_KEY && t == KEY_POS_FILL_POS_MATCH) {
      auto fill = getSpecifiedDevice(L_FILL, s);
      fill->getParam<LumiverseOrientation>("polar")->setValAsPercent(-p + h);
    }

    val->setValAsPercent(p + h);
    f2 = f(s);
    val->setValAsPercent(p);

    if (c._light == L_KEY && t == KEY_POS_FILL_POS_MATCH) {
      auto fill = getSpecifiedDevice(L_FILL, s);
      fill->getParam<LumiverseOrientation>("polar")->setValAsPercent(-p);
    }

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

    if (c._light == L_KEY && t == KEY_POS_FILL_POS_MATCH) {
      auto fill = getSpecifiedDevice(L_FILL, s);
      fill->getParam<LumiverseOrientation>("polar")->setValAsPercent(a + h);
    }

    val->setValAsPercent(a + h);
    f2 = f(s);
    val->setValAsPercent(a);

    if (c._light == L_KEY && t == KEY_POS_FILL_POS_MATCH) {
      auto fill = getSpecifiedDevice(L_FILL, s);
      fill->getParam<LumiverseOrientation>("polar")->setValAsPercent(a);
    }
    break;
  }
  default:
    break;
  }

  return (f2 - f1) / h;
}

double AttributeSearchThread::setDeviceValue(EditConstraint c, EditType t, double val, Snapshot * s)
{
  Device* d = getSpecifiedDevice(c._light, s);

  switch (c._param) {
  case INTENSITY:
  {
    d->getIntensity()->setValAsPercent(val);
    
    if (c._light == L_KEY) {
      if (t == KEY_INTENS_RIM_CONTRAST_MATCH) {
        Device* rim = getSpecifiedDevice(L_RIM, s);
        double r = rim->getIntensity()->asPercent() / d->getIntensity()->asPercent();
        rim->getIntensity()->setValAsPercent(val * r);
      }
      if (t == KEY_INTENS_FILL_CONTRAST_MATCH) {
        Device* fill = getSpecifiedDevice(L_FILL, s);
        double r = fill->getIntensity()->asPercent() / d->getIntensity()->asPercent();
        fill->getIntensity()->setValAsPercent(val * r);
      }
    }
    
    return d->getIntensity()->asPercent();
  }
  case HUE:
  {
    val *= 360;
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(val, hsv[1], hsv[2]);

    if (c._light == L_KEY) {
      if (t == KEY_HUE_FILL_HUE_MATCH || t == KEY_HUE_FILL_RIM_HUE_MATCH) {
        auto fill = getSpecifiedDevice(L_FILL, s);
        Eigen::Vector3d fhsv = fill->getColor()->getHSV();
        fill->getColor()->setHSV(fhsv[0] + (val - hsv[0]), fhsv[1], fhsv[2]);
      }
      if (t == KEY_HUE_FILL_RIM_HUE_MATCH) {
        auto rim = getSpecifiedDevice(L_RIM, s);
        Eigen::Vector3d rhsv = rim->getColor()->getHSV();
        rim->getColor()->setHSV(rhsv[0] + (val - hsv[0]), rhsv[1], rhsv[2]);
      }
    }

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

    if (c._light == L_KEY && t == KEY_POS_FILL_POS_MATCH) {
      auto fill = getSpecifiedDevice(L_FILL, s);
      fill->getParam<LumiverseOrientation>("polar")->setValAsPercent(-val);
    }

    return o->asPercent();
  }
  case AZIMUTH:
  {
    LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("azimuth");
    o->setValAsPercent(val);

    if (c._light == L_KEY && t == KEY_POS_FILL_POS_MATCH) {
      auto fill = getSpecifiedDevice(L_FILL, s);
      fill->getParam<LumiverseOrientation>("polar")->setValAsPercent(val);
    }

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

double AttributeSearchThread::getDeviceValue(EditConstraint c, Snapshot * s)
{
  Device* d = getSpecifiedDevice(c._light, s);

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

bool AttributeSearchThread::isParamLocked(EditConstraint c, EditType t, Snapshot* s)
{
  Device* d = getSpecifiedDevice(c._light, s);

  switch (c._param) {
  case INTENSITY:
    return isDeviceParamLocked(d->getId(), "intensity");
  case HUE:
    return isDeviceParamLocked(d->getId(), "colorH");
  case SAT:
    return isDeviceParamLocked(d->getId(), "colorS");
  case VALUE:
    return isDeviceParamLocked(d->getId(), "colorV");
  case RED:
    return isDeviceParamLocked(d->getId(), "colorRed");
  case GREEN:
    return isDeviceParamLocked(d->getId(), "colorGreen");
  case BLUE:
    return isDeviceParamLocked(d->getId(), "colorBlue");
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
