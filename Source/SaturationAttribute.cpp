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

  return sat * 100;
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
    if (d->metadataExists("brightnessAttributeWeight")) {
      _weights[d->getId()] = stof(d->getMetadata("brightnessAttributeWeight"));
    }
  }
}

list<Snapshot*> SaturationAttribute::nonSemanticSearch()
{
  if (getStatus() == A_EQUAL || getStatus() == A_IGNORE)
    return list<Snapshot*>();

  // increase the saturation of each light by 0.05, do 10 times for good spread.
  Snapshot* start = new Snapshot(getRig(), nullptr);
  list<Snapshot*> results;
  
  for (int i = 0; i < 10; i++) {
    auto devices = start->getRigData();
    for (auto d : devices) {
      if (!isDeviceParamLocked(d.first, "color")) {
        auto color = d.second->getColor();
        if (color != nullptr) {
          Eigen::Vector3d hsv = color->getHSV();
          if (getStatus() == A_MORE) {
            color->setHSV(hsv[0], hsv[1] + 0.05, hsv[2]);
          }
          else if (getStatus() == A_LESS) {
            color->setHSV(hsv[0], hsv[1] - 0.05, hsv[2]);
          }
        }
      }
    }

    results.push_back(new Snapshot(*start));
  }

  return results;
}
