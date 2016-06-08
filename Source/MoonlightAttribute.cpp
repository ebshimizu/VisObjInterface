/*
  ==============================================================================

    MoonlightAttribute.cpp
    Created: 26 Apr 2016 4:55:46pm
    Author:  falindrith

  ==============================================================================
*/

#include "MoonlightAttribute.h"

MoonlightAttribute::MoonlightAttribute(int w, int h) : HistogramAttribute("Moonlight", w, h)
{
  _setForegroundButton = new TextButton("Set Foreground", "Set foreground region to the currently selected region");
  _setForegroundButton->addListener(this);
  addAndMakeVisible(_setForegroundButton);
}

MoonlightAttribute::~MoonlightAttribute()
{
  delete _setForegroundButton;
}

double MoonlightAttribute::evaluateScene(Snapshot * s)
{
  // moonlight here is more of a theatrical moonlight
  // -the forground region should be illuminated by pale blue-white
  // -the background should be dark, but not black, and should have a blue-purple tint

  // make sure the foreground is actually defined.
  if (_foreground.getWidth() == 0 || _foreground.getHeight() == 0)
    return 0;

  Image i = generateImage(s);

  double fgScore = 0;
  double bgScore = 0;
  int fgCount = 0;
  int bgCount = 0;

  Rectangle<int> fgPixelBounds;
  fgPixelBounds.setWidth(_foreground.getWidth() * _canonicalWidth);
  fgPixelBounds.setHeight(_foreground.getHeight() * _canonicalHeight);
  fgPixelBounds.setX(_foreground.getX() * _canonicalWidth);
  fgPixelBounds.setY(_foreground.getY() * _canonicalHeight);

  // target colors
  Eigen::Vector3d targetFg(121 / 255.0, 173 / 255.0, 166 / 255.0);
  Eigen::Vector3d targetBg(7 / 255.0, 51 / 255.0, 105 / 255.0);

  // accuracy requirement
  int fgAccurate = 0;
  int bgAccurate = 0;

  for (int y = 0; y < _canonicalHeight; y++) {
    for (int x = 0; x < _canonicalWidth; x++) {
      Colour px = i.getPixelAt(x, y);
      Colour nrm = _fullWhite.getPixelAt(x, y);

      Eigen::Vector3d rgbnrm(nrm.getRed() / 255.0, nrm.getGreen() / 255.0, nrm.getBlue() / 255.0);
      Eigen::Vector3d rgbpx(px.getRed() / 255.0, px.getGreen() / 255.0, px.getBlue() / 255.0);

      for (int i = 0; i < 3; i++) {
        if (isnan(rgbpx[i])) {
          rgbpx[i] = 0;
        }

        if (rgbpx[i] > 1)
          rgbpx[i] = 1;
      }

      if (fgPixelBounds.contains(x, y)) {
        // fg score is just the distance from a target color
        // target hue: 196, low sat, high bright
        //float hueDist = min(abs(hue - .54), abs((hue + 1) - .54));
        //float sat = max(0.25f, 1 - px.getSaturation());

        //fgScore += pow(0.5 - hueDist, 2) * 2 * px.getBrightness() * sat;
        float score = (sqrt(3) - (targetFg - rgbpx).norm()) / sqrt(3);
        
        if (score >= 0.85)
          fgAccurate++;

        fgScore += score * score;
        fgCount++;
      }
      else {
        // bg score
        // target hue: 238, highish sat, low bright
        //float hueDist = min(abs(hue - .66), abs((hue + 1) - .66));
        //float bri = max(0.25f, 1 - px.getBrightness());

        //bgScore += pow(0.5 - hueDist, 2) * 2 * bri * px.getSaturation();
        float score = (sqrt(3) - (targetBg - rgbpx).norm()) / sqrt(3);
        bgScore += score * score;

        if (score >= 0.85)
          bgAccurate++;

        bgCount++;
      }
    }
  }

  bgScore /= (double)bgCount;
  fgScore /= (double)fgCount;

  // accuracy
  bgScore = bgScore * (bgAccurate / (double)bgCount);
  fgScore = fgScore * (fgAccurate / (double)fgCount);

  return (bgScore * 0.5 + fgScore * 0.5) * 100;
}

void MoonlightAttribute::preProcess()
{
  auto systems = getRig()->getMetadataValues("system");

  for (auto& s : systems) {
    _systemCache[s] = getRig()->select("$system=" + s);
  }

  // generate all lights at full image to attempt to normalize colors
  Snapshot* temp = new Snapshot(getRig());
  auto& devices = temp->getRigData();

  for (auto& d : devices) {
    d.second->getIntensity()->setValAsPercent(1);
    d.second->getColor()->setRGB(1, 1, 1, 1);
  }

  _fullWhite = generateImage(temp);
  delete temp;
}

void MoonlightAttribute::buttonClicked(Button * b)
{
  HistogramAttribute::buttonClicked(b);

  if (b->getName() == "Set Foreground") {
    setForegroundArea();
    repaint();
  }
}

void MoonlightAttribute::resized()
{
  HistogramAttribute::resized();

  auto lbounds = getLocalBounds();
  auto top = lbounds.removeFromTop(24);

  top.removeFromRight(80);
  _setForegroundButton->setBounds(top.removeFromRight(80).reduced(2));
}

void MoonlightAttribute::setForegroundArea()
{
  _foreground = getGlobalSettings()->_focusBounds;
}

float MoonlightAttribute::getMaxSystemBrightness(string sys, Snapshot * s)
{
  auto devices = _systemCache[sys].getDevices();
  auto r = s->getRigData();

  float max = 0;
  for (auto& d : devices) {
    if (r[d->getId()]->getIntensity()->asPercent() > max)
      max = r[d->getId()]->getIntensity()->asPercent();
  }
  return max;
}

float MoonlightAttribute::getAvgSystemBrightness(string sys, Snapshot * s)
{
  auto r = s->getRigData();
  auto devices = _systemCache[sys].getDevices();

  float avg = 0;
  for (auto& d : devices) {
    avg += r[d->getId()]->getIntensity()->asPercent();
  }
  return avg / devices.size();
}

Eigen::Vector3d MoonlightAttribute::getAvgColor(string sys, Snapshot * s)
{
  auto r = s->getRigData();
  auto devices = _systemCache[sys].getDevices();

  Eigen::Vector3d avg(0, 0, 0);
  for (auto& d : devices) {
    avg += r[d->getId()]->getColor()->getHSV();
  }
  return avg / devices.size();
}
