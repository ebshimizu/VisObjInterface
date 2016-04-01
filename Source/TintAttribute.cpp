/*
  ==============================================================================

    TintAttribute.cpp
    Created: 1 Apr 2016 1:20:16pm
    Author:  falindrith

  ==============================================================================
*/

#include "TintAttribute.h"
#include "BrightAttribute.h"

TintAttribute::TintAttribute() : AttributeControllerBase("Tint")
{
  _targetColor = Colour(255, 255, 255);
}

TintAttribute::~TintAttribute()
{
}

void TintAttribute::paint(Graphics & g)
{
  AttributeControllerBase::paint(g);

  g.setColour(_targetColor);
  g.fillRect(_colorSelect);
}

void TintAttribute::resized()
{
  AttributeControllerBase::resized();
  auto lbounds = getLocalBounds();
  auto top = lbounds.removeFromTop(24);
  top.reduce(2, 2);

  top.removeFromLeft(100);
  _colorSelect = top.removeFromLeft(100);
}

double TintAttribute::evaluateScene(Snapshot * s)
{
  Eigen::Vector3d color(0, 0, 0);
  auto devices = s->getRigData();
  for (const auto& d : devices) {
    auto c = d.second->getColor()->getRGB();
    color += c * _weights[d.first];
  }

  Eigen::Vector3d targetColor(_targetColor.getRed() / 255.0, _targetColor.getGreen() / 255.0, _targetColor.getBlue() / 255.0);
  // calculate difference from target color
  double diff = (color - targetColor).squaredNorm();
  return (1 - diff) * 100;
}

void TintAttribute::preProcess()
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
    BrightAttribute* ba = new BrightAttribute("TINT HELPER");
    ba->preProcess();
    delete ba;
  }

  // now that the metadata exists, copy it
  for (const auto& d : devices) {
    _weights[d->getId()] = stof(d->getMetadata("brightnessAttributeWeight"));
  }
}

void TintAttribute::mouseDown(const MouseEvent & e)
{
  auto pos = e.getPosition();
  if (_colorSelect.contains(pos)) {
    // trigger color selector
    ColourSelector* cs = new ColourSelector(ColourSelector::showColourAtTop | ColourSelector::showSliders | ColourSelector::showColourspace);
    cs->setName("Tint Color");
    cs->setCurrentColour(_targetColor);
    cs->setSize(300, 400);
    cs->addChangeListener(this);
    CallOutBox::launchAsynchronously(cs, this->getScreenBounds(), nullptr);
  }
}

void TintAttribute::changeListenerCallback(ChangeBroadcaster * source)
{
  ColourSelector* cs = dynamic_cast<ColourSelector*>(source);
  _targetColor = cs->getCurrentColour();
  repaint();
}
