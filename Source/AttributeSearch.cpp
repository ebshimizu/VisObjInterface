/*
  ==============================================================================

    AttributeSearch.cpp
    Created: 6 Jan 2016 11:25:42am
    Author:  falindrith

  ==============================================================================
*/

#include "AttributeSearch.h"

map<EditType, vector<EditConstraint> > editConstraints = {
  { KEY_HUE, { EditConstraint(L_KEY, HUE) } }
};

Array<SearchResult> attributeSearch(map<string, AttributeControllerBase*> active)
{
  // If there's no active attribute, just leave
  if (active.size() == 0)
    return Array<SearchResult>();

  // Save current state of rig.
  Rig* rig = getRig();
  Snapshot* original = new Snapshot(rig, nullptr);

  // for each edit, get list of scenes that match attribute criteria
  // Filter out scenes that are too similar
  // repeat with set of scenes for larger edit depths

}

Array<Snapshot*> performEdit(EditType t, Snapshot * s, map<string, AttributeControllerBase*> active)
{
  double minDist = getGlobalSettings()->_minEditDist;
  int numScenes = getGlobalSettings()->_numEditScenes;
  double gamam = getGlobalSettings()->_searchGDGamma;
  
  // run gradient descent/ascent (more/less) until number of scenes to return
  // meets minimum, or the optimization is done.
}

double numericDeriv(EditConstraint c, Snapshot* s, AttributeControllerBase* attr)
{
  // load the appropriate settings and get the proper device.
  double h = getGlobalSettings()->_searchDerivDelta;
  Device* d = getSpecifiedDevice(c._light, s);
  double f1 = attr->evaluateScene(s->getDevices());
  double f2;

  switch (c._param) {
  case INTENSITY:
  {
    float o = d->getIntensity()->getVal();
    d->setIntensity(o + h);
    f2 = attr->evaluateScene(s->getDevices());
    d->setIntensity(o);
    break;
  }
  case HUE:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(hsv[0] + h, hsv[1], hsv[2]);
    f2 = attr->evaluateScene(s->getDevices());
    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2]);
    break;
  }
  case SAT:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(hsv[0], hsv[1] + h, hsv[2]);
    f2 = attr->evaluateScene(s->getDevices());
    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2]);
    break;
  }
  case VALUE:
  {
    Eigen::Vector3d hsv = d->getColor()->getHSV();
    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2] + h);
    f2 = attr->evaluateScene(s->getDevices());
    d->getColor()->setHSV(hsv[0], hsv[1], hsv[2]);
    break;
  }
  case RED:
  {
    double r = d->getColor()->getColorChannel("Red");
    d->getColor()->setColorChannel("Red", r + h);
    f2 = attr->evaluateScene(s->getDevices());
    d->getColor()->setColorChannel("Red", r);
    break;
  }
  case BLUE:
  {
    double b = d->getColor()->getColorChannel("Blue");
    d->getColor()->setColorChannel("Blue", b + h);
    f2 = attr->evaluateScene(s->getDevices());
    d->getColor()->setColorChannel("Blue", b);
    break;
  }
  case GREEN:
  {
    double g = d->getColor()->getColorChannel("Green");
    d->getColor()->setColorChannel("Green", g + h);
    f2 = attr->evaluateScene(s->getDevices());
    d->getColor()->setColorChannel("Green", g);
    break;
  }
  case POLAR:
  {
    LumiverseOrientation* val = (LumiverseOrientation*)d->getParam("polar");
    float p = val->getVal();
    val->setVal(p + h);
    f2 = attr->evaluateScene(s->getDevices());
    val->setVal(p);
    break;
  }
  case AZIMUTH:
  {
    LumiverseOrientation* val = (LumiverseOrientation*)d->getParam("azimuth");
    float a = val->getVal();
    val->setVal(a + h);
    f2 = attr->evaluateScene(s->getDevices());
    val->setVal(a);
    break;
  }
  default:
    break;
  }

  return (f2 - f1) / h;
}

void setUpdatedValue(EditConstraint c, double val, Snapshot * s)
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

Device* getSpecifiedDevice(EditLightType l, Snapshot * s)
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
