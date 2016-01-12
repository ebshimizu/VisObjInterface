/*
  ==============================================================================

    AttributeSearch.cpp
    Created: 6 Jan 2016 11:25:42am
    Author:  falindrith

  ==============================================================================
*/

#include "AttributeSearch.h"

map<EditType, vector<EditConstraint> > editConstraints = {
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
  { RIM_HSV, { EditConstraint(L_RIM, HUE), EditConstraint(L_RIM, SAT), EditConstraint(L_RIM, VALUE) } }
};

SearchResult::SearchResult() : _scene (nullptr) { }

SearchResult::SearchResult(Snapshot * s, Array<EditType> eh, map<string, double> av) :
  _scene(s), _editHistory(eh), _attrVals(av)
{
}

SearchResult::SearchResult(const SearchResult & other) :
  _scene(other._scene), _editHistory(other._editHistory), _attrVals(other._attrVals)
{
}

SearchResult::~SearchResult() {
  if (_scene != nullptr)
    delete _scene;
}


// Search functions
// ==============================================================================

vector<SearchResult*> attributeSearch(map<string, AttributeControllerBase*> active, int editDepth)
{
  // If there's no active attribute, just leave
  if (active.size() == 0)
    return vector<SearchResult*>();

  AttributeSearchThread* t = new AttributeSearchThread(active, editDepth);
  t->runThread();

  vector<SearchResult*> scenes = t->getResults();
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
  case KEY_FILL_INTENS:
    return "Key-Fill Intensity";
  default:
    return "";
  }
}

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
  start->_scene = new Snapshot(*_original);
  _results.push_back(start);

  for (int i = 0; i < _editDepth; i++) {
    setProgress((float)i / _editDepth);

    vector<SearchResult*> newResults = runSingleLevelSearch(_results, i);

    if (threadShouldExit())
      return;

    // delete old results
    for (auto r : _results)
    {
      delete r;
    }
    _results.clear();
    _results = newResults;

    // Filter if needed
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

vector<SearchResult*> AttributeSearchThread::runSingleLevelSearch(vector<SearchResult*> startScenes, int level)
{
  vector<SearchResult*> searchResults;

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

  float seqPct = ((float)level / _editDepth);

  int totalOps = startScenes.size() * editConstraints.size();
  int opCt = 0;

  int i = 0, j = 0;

  // For each scene in the initial set
  for (const auto& scene : startScenes) {
    // For each edit, get a list of scenes returned and just add it to the overall list.
    for (const auto& edits : editConstraints) {
      opCt++;
      setStatusMessage("Scene " + String(i) + ": Running Edit " + String(j) + "/" + String(editConstraints.size()));
      vector<Snapshot*> editScenes = performEdit(edits.first, scene->_scene, f);
      
      if (threadShouldExit())
        return vector<SearchResult*>();

      for (auto s : editScenes) {
        SearchResult* r = new SearchResult();
        r->_scene = s;
        r->_editHistory.addArray(scene->_editHistory);
        r->_editHistory.add(edits.first);
        for (const auto& kvp : _active) {
          r->_attrVals[kvp.first] = kvp.second->evaluateScene(s->getRigData());
        }
        searchResults.push_back(r);
      }

      setProgress(seqPct + ((float)opCt/(float)totalOps)* (1.0/_editDepth));
      j++;
    }
    i++;
  }

  return searchResults;
}

vector<Snapshot*> AttributeSearchThread::performEdit(EditType t, Snapshot * orig, attrObjFunc f)
{
  // note of interest: which light is they key may vary though the course of this function,
  // potentially causing discontinuitous jumps in the objective function.
  // whether or not this is fatal remains to be seen.

  // duplicate initial state for internal use
  Snapshot* s = new Snapshot(*orig);

  // load settings
  double minDist = getGlobalSettings()->_minEditDist;   // may want to set min dist based on how large deriv changes are at start point
  int numScenes = getGlobalSettings()->_numEditScenes;
  double gamma = getGlobalSettings()->_searchGDGamma;
  double thresh = getGlobalSettings()->_searchGDTol;

  // Save start value
  double startAttrVal = f(s);
  double attrVal = startAttrVal;

  int vecSize = editConstraints[t].size();
  Eigen::VectorXd oldX;
  Eigen::VectorXd newX;
  Eigen::VectorXd G;
  vector<Snapshot*> scenes;

  oldX.resize(vecSize);
  newX.resize(vecSize);
  G.resize(vecSize);

  // Initalize the x (variable) vector
  int i = 0;
  for (const auto& c : editConstraints[t]) {
    newX[i] = getDeviceValue(c, s);
    G[i] = 0;
    i++;
  }

  // run gradient descent until number of scenes to return
  // meets minimum, or the optimization is done.
  do {
    if (threadShouldExit())
      return scenes;

    oldX = newX;
    
    // Get derivative
    Eigen::VectorXd dX;
    dX.resize(vecSize);
    i = 0;
    for (const auto& c : editConstraints[t]) {
      dX[i] = numericDeriv(c, s, f);
      G[i] += dX[i] * dX[i];
      i++;
    }
    
    // Descent
    Eigen::VectorXd Gr = G.array().pow(-0.5).matrix();
    newX = oldX - dX.cwiseProduct(Gr * gamma);

    // Update scene
    i = 0;
    for (const auto& c : editConstraints[t]) {
      setDeviceValue(c, newX[i], s);
      i++;
    }

    // Determine if scene should go in the list of scenes to return
    attrVal = f(s);
    if (abs(attrVal - startAttrVal) > minDist && scenes.size() < numScenes) {
      scenes.push_back(new Snapshot(*s));
    }
    if (scenes.size() >= numScenes) {
      // scenes full, stop
      break;      
    }
  // continue loop while we're making sufficient progress and the attribute value is actually changing
  } while ((oldX - newX).norm() > thresh && abs(attrVal - startAttrVal) > thresh);
  
  return scenes;
}

double AttributeSearchThread::numericDeriv(EditConstraint c, Snapshot* s, attrObjFunc f)
{
  // load the appropriate settings and get the proper device.
  double h = getGlobalSettings()->_searchDerivDelta;
  Device* d = getSpecifiedDevice(c._light, s);
  double f1 = f(s);
  double f2;

  switch (c._param) {
  case INTENSITY:
  {
    float o = d->getIntensity()->asPercent();
    if (o >= 1)
      h = -h;

    d->getIntensity()->setValAsPercent(o + h);
    f2 = f(s);
    d->setIntensity(o);
    break;
  }
  case HUE:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    // hue wraps around, derivative should be fine if hue is at max/min
    d->getColor()->setHSV(hsv[0] + h, hsv[1], hsv[2]);
    f2 = f(s);
    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2]);
    break;
  }
  case SAT:
  {
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
    LumiverseOrientation* val = (LumiverseOrientation*)d->getParam("polar");
    float p = val->asPercent();
    if (p >= 1)
      h = -h;

    val->setVal(p + h);
    f2 = f(s);
    val->setVal(p);
    break;
  }
  case AZIMUTH:
  {
    LumiverseOrientation* val = (LumiverseOrientation*)d->getParam("azimuth");
    float a = val->asPercent();
    if (a >= 1)
      h = -h;

    val->setVal(a + h);
    f2 = f(s);
    val->setVal(a);
    break;
  }
  default:
    break;
  }

  return (f2 - f1) / h;
}

void AttributeSearchThread::setDeviceValue(EditConstraint c, double val, Snapshot * s)
{
  Device* d = getSpecifiedDevice(c._light, s);

  switch (c._param) {
  case INTENSITY:
    d->setIntensity(val);
    break;
  case HUE:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(val, hsv[1], hsv[2]);
    break;
  }
  case SAT:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(hsv[0], val, hsv[2]);
    break;
  }
  case VALUE:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(hsv[0], hsv[1], val);
    break;
  }
  case RED:
    d->getColor()->setColorChannel("Red", val);
    break;
  case BLUE:
    d->getColor()->setColorChannel("Blue", val);
    break;
  case GREEN:
    d->getColor()->setColorChannel("Green", val);
    break;
  case POLAR:
  {
    LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("polar");
    o->setVal(val);
    break;
  }
  case AZIMUTH:
  {
    LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("azimuth");
    o->setVal(val);
    break;
  }
  default:
    break;
  }

}

double AttributeSearchThread::getDeviceValue(EditConstraint c, Snapshot * s)
{
  Device* d = getSpecifiedDevice(c._light, s);

  switch (c._param) {
  case INTENSITY:
    return d->getIntensity()->getVal();
  case HUE:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    return hsv[0];
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
    return o->getVal();
  }
  case AZIMUTH:
  {
    LumiverseOrientation* o = (LumiverseOrientation*)d->getParam("azimuth");
    return o->getVal();
  }
  default:
    break;
  }
}

Device* AttributeSearchThread::getSpecifiedDevice(EditLightType l, Snapshot * s)
{
  auto devices = s->getRigData();
  
  // Rim is easy to identify
  if (l == L_RIM)
    return devices["rim"];
  
  Device* key;
  Device* fill;

  // Determine which light is key/fill
  if (devices["right"]->getIntensity()->getVal() > devices["left"]->getIntensity()->getVal()) {
    key = devices["right"];
    fill = devices["left"];
  }
  else {
    key = devices["left"];
    fill = devices["right"];
  }

  if (l == L_FILL)
    return fill;
  else if (l == L_KEY)
    return key;

  // If this ever happens, something's gone terribly wrong
  return nullptr;
}
