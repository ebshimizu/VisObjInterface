/*
  ==============================================================================

    SaturationAttribute.cpp
    Created: 31 Mar 2016 10:48:48am
    Author:  falindrith

  ==============================================================================
*/

#include "SaturationAttribute.h"
#include "BrightAttribute.h"

SaturationAttribute::SaturationAttribute() : AttributeControllerBase("Saturation")
  {
  }

SaturationAttribute::~SaturationAttribute()
{
}

double SaturationAttribute::evaluateScene(Snapshot * s)
{
  // average saturation of the scene, accoring to the color of the lights
  auto& devices = s->getRigData();

  double sat = 0;
  for (const auto& d : devices) {
    Eigen::Vector3d hsv = d.second->getColor()->getHSV();
    sat += hsv[1] * _weights[d.second->getId()];
  }

  return sat;
}

void SaturationAttribute::preProcess()
{
  auto& devices = getRig()->getDeviceRaw();

  // this attribute attempts to find the pre-computed brightnessAttributeWeight if it exists
  // if it doesn't we kinda just cheat a bit by creating a brightness attribute and preprocessing it
  // and then copying the values resulting from that.
  bool weightsExist = true;
  for (const auto& d : devices) {
    if (!d->metadataExists("brightnessAttributeWeight")) {
      weightsExist = false;
      break;
    }
  }

  if (!weightsExist) {
    // make those things exist
    BrightAttribute* ba = new BrightAttribute("CONTRAST HELPER");
    ba->preProcess();
    delete ba;
  }

  // now that the metadata exists, copy it
  for (const auto& d : devices) {
    _weights[d->getId()] = stof(d->getMetadata("brightnessAttributeWeight"));
  }
}
