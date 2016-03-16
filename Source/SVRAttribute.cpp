/*
  ==============================================================================

    SVRAttribute.cpp
    Created: 3 Mar 2016 4:27:53pm
    Author:  falindrith

  ==============================================================================
*/

#include "SVRAttribute.h"

SVRAttribute::SVRAttribute(string filename, string name) : AttributeControllerBase(name)
{
  _model = svm_load_model(filename.c_str());
}

SVRAttribute::~SVRAttribute()
{
  delete _model;
}

Eigen::VectorXd SVRAttribute::devicesToVector(Device * key, Device * fill, Device * rim)
{
  // Param order: Intensity, polar, azimuth, R, G, B, Softness (penumbra angle)
  // Device order: Key, Fill, Rim, L/R indicator
  int numFeats = 7;
  Eigen::VectorXd features;
  features.resize(numFeats * 3 + 1);

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
  return Eigen::VectorXd();
}

double SVRAttribute::evaluateScene(Device * key, Device * fill, Device * rim)
{
  Eigen::VectorXd v = devicesToVector(key, fill, rim);
  
  vector<svm_node> sv;

  // Last parameter in feature vector is for reconstruction, not needed here
  for (int i = 0; i < v.size() - 1; i++) {
    svm_node n;
    n.index = i;
    n.value = v[i];
    sv.push_back(n);
  }

  svm_node end;
  end.index = -1;
  sv.push_back(end);

  return -svm_predict(_model, &(sv.front())) * 100;
}
