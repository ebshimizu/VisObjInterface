/*
  ==============================================================================

    ContrastAttribute.cpp
    Created: 29 Feb 2016 11:25:00am
    Author:  falindrith

  ==============================================================================
*/

#include "ContrastAttribute.h"
#include "BrightAttribute.h"

ContrastAttribute::ContrastAttribute(string area1, string area2) : AttributeControllerBase("Contrast: " + area1 + " - " + area2)
{
  _area1 = area1;
  _area2 = area2;
  _autoLockParams.insert("polar");
  _autoLockParams.insert("azimuth");
}

ContrastAttribute::~ContrastAttribute()
{
}

double ContrastAttribute::evaluateScene(Snapshot* s)
{
  // in this eval function we don't use the relative values of the intensities of the lights
  // in order to capture larger differences in relative brightness
  auto& rigData = s->getRigData();

  // compute total brightness for area 1
  double area1Br = 0;
  for (const auto& d : _area1Weights) {
    double intens = rigData[d.first]->getIntensity()->getVal();
    area1Br += intens * d.second;
  }

  // compute total brightness for area 2
  double area2Br = 0;
  for (const auto& d : _area2Weights) {
    double intens = rigData[d.first]->getIntensity()->getVal();
    area2Br += intens * d.second;
  }

  // contrast is max / min
  double contrast = (area1Br > area2Br) ? (area1Br / area2Br) : (area2Br / area1Br);

  // something something normalize this...

  return contrast * 10;
}

void ContrastAttribute::preProcess()
{
  DeviceSet area1Devices = getRig()->select("$area=" + _area1);
  DeviceSet area2Devices = getRig()->select("$area=" + _area2);

  _area1Weights.clear();
  _area2Weights.clear();

  // this attribute attempts to find the pre-computed brightnessAttributeWeight if it exists
  // if it doesn't we kinda just cheat a bit by creating a brightness attribute and preprocessing it
  // and then copying the values resulting from that.
  auto a1d = area1Devices.getDevices();
  bool area1WeightExists = true;
  for (const auto& d : a1d) {
    if (!d->metadataExists("brightnessAttributeWeight")) {
      area1WeightExists = false;
      break;
    }
  }

  auto a2d = area2Devices.getDevices();
  bool area2WeightExists = true;
  for (const auto& d : a2d) {
    if (!d->metadataExists("brightnessAttributeWeight")) {
      area2WeightExists = false;
      break;
    }
  }

  if (!area1WeightExists || !area2WeightExists) {
    // make those things exist
    BrightAttribute* ba = new BrightAttribute("CONTRAST HELPER");
    ba->preProcess();
    delete ba;
  }

  // now that the metadata exists, copy it
  for (const auto& d : a1d) {
    _area1Weights[d->getId()] = stof(d->getMetadata("brightnessAttributeWeight"));
  }

  for (const auto& d : a2d) {
    _area2Weights[d->getId()] = stof(d->getMetadata("brightnessAttributeWeight"));
  }
}
