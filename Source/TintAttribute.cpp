/*
  ==============================================================================

    TintAttribute.cpp
    Created: 1 Apr 2016 1:20:16pm
    Author:  falindrith

  ==============================================================================
*/

#include "TintAttribute.h"

TintAttribute::TintAttribute(int w, int h) : HistogramAttribute("Tint", w, h)
{
  _targetColor = Colour(255, 255, 255);
  _autoLockParams.insert(INTENSITY);
  _autoLockParams.insert(POLAR);
  _autoLockParams.insert(AZIMUTH);
  _autoLockParams.insert(SOFT);
}

TintAttribute::~TintAttribute()
{
}

void TintAttribute::paint(Graphics & g)
{
  HistogramAttribute::paint(g);

  g.setColour(_targetColor);
  g.fillRect(_colorSelect);
}

void TintAttribute::resized()
{
  HistogramAttribute::resized();
  auto lbounds = getLocalBounds();
  auto top = lbounds.removeFromTop(24);
  top.reduce(2, 2);

  top.removeFromLeft(100);
  _colorSelect = top.removeFromLeft(100);
}

double TintAttribute::evaluateScene(Snapshot * s)
{
  Image i = generateImage(s);

  //Eigen::Vector3d color = getAverageColor(i);
  double targetHue = _targetColor.getHue();
  double err = 0;

  for (int y = 0; y < i.getHeight(); y++) {
    for (int x = 0; x < i.getWidth(); x++) {
      auto c = i.getPixelAt(x, y);
      err += min(abs(c.getHue() - targetHue), abs(c.getHue() + 1 - targetHue));
    }
  }

  double maxErr = i.getHeight() * i.getWidth() * 0.5;

  return ((maxErr - err) / maxErr) * 100;

  // color diff
  //Eigen::Vector3d targetColor(_targetColor.getRed() / 255.0, _targetColor.getGreen() / 255.0, _targetColor.getBlue() / 255.0);

  // hue diff
  //return 2 * (0.5 - min(abs(hue - _targetColor.getHue()), abs(hue + 1 - _targetColor.getHue())));

  //return ((sqrt(3) - (color - targetColor).norm()) / sqrt(3)) * 100;
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