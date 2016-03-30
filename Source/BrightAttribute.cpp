/*
  ==============================================================================

    BrightAttribute.cpp
    Created: 20 Jan 2016 6:30:19pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "BrightAttribute.h"

//==============================================================================
BrightAttribute::BrightAttribute() : AttributeControllerBase("Bright")
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

BrightAttribute::BrightAttribute(String name) : AttributeControllerBase(name)
{
}

BrightAttribute::~BrightAttribute()
{
}

double BrightAttribute::evaluateScene(Snapshot* s)
{
  //// testing performance for rendering thumbnails
  //auto p = getAnimationPatch();

  //// Quit early if we can't render things
  //if (p == nullptr)
  //  return 0;

  //int width = getGlobalSettings()->_renderWidth * getGlobalSettings()->_thumbnailRenderScale;
  //int height = getGlobalSettings()->_renderHeight * getGlobalSettings()->_thumbnailRenderScale;
  //p->setDims(width, height);
  //p->setSamples(getGlobalSettings()->_thumbnailRenderSamples);

  //// Render to image
  //Image *img = new Image(Image::ARGB, width, height, true);
  //uint8* bufptr = Image::BitmapData(*img, Image::BitmapData::readWrite).getPixelPointer(0, 0);

  //p->renderSingleFrameToBuffer(s->getDevices(), bufptr);

  //// calculate brightness, average brightness values across image
  //double totalY = 0;
  //for (int x = 0; x < width; x++) {
  //  for (int y = 0; y < height; y++) {
  //    auto& color = img->getPixelAt(x, y);
  //    totalY += (color.getRed() / 255.0) * 0.2126 + (color.getGreen() / 255.0) * 0.7152 + (color.getBlue() / 255.0) * 0.0722;
  //  }
  //}

  //totalY /= (double)(width * height);
  //delete img;
  //return totalY * 100;

  auto& devices = s->getRigData();
  double sum = 0;
  for (auto& d : devices) {
    sum += d.second->getIntensity()->asPercent() * _weights[d.first];
  }

  return sum * 100;
}

void BrightAttribute::preProcess()
{
  _weights.clear();

  // maybe here run a check to see if we've already calculated this for the scene, since
  // we can store anything we want in the device metadata

  map<string, double> brightness;
  double totalBrightness = 0;
  auto p = getAnimationPatch();

  // Quit early if we can't render things
  if (p == nullptr)
    return;

  int width = getGlobalSettings()->_renderWidth * getGlobalSettings()->_thumbnailRenderScale;
  int height = getGlobalSettings()->_renderHeight * getGlobalSettings()->_thumbnailRenderScale;
  p->setDims(width, height);
  p->setSamples(getGlobalSettings()->_thumbnailRenderSamples);

  // for the brightness attribute, we want to see how much brightness each light contributes
  // and weight accordingly. We do this by rendering an image and calculating the brightness of the image
  Snapshot* s = new Snapshot(getRig(), nullptr);
  auto devices = s->getRigData();

  for (auto d : devices) {
    // reset all devices to 0
    for (auto d2 : devices) {
      d2.second->setIntensity(0);
    }

    // set current device to 100% white
    d.second->getIntensity()->setValAsPercent(1);
    d.second->getColor()->setRGB(1, 1, 1, 1);

    // Render to image
    Image img = Image(Image::ARGB, width, height, true);
    uint8* bufptr = Image::BitmapData(img, Image::BitmapData::readWrite).getPixelPointer(0, 0);

    p->renderSingleFrameToBuffer(s->getDevices(), bufptr);

    // calculate brightness, average brightness values across image
    double totalY = 0;
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        auto color = img.getPixelAt(x, y);
        totalY += (color.getRed() / 255.0) * 0.2126 + (color.getGreen() / 255.0) * 0.7152 + (color.getBlue() / 255.0) * 0.0722;
      }
    }
    Lumiverse::Logger::log(LDEBUG, String(img.getReferenceCount()).toStdString());

    totalY /= (double)(width * height);
    brightness[d.first] = totalY;
    totalBrightness += totalY;
  }

  // Normalize brightnesses to get weight.
  for (auto& b : brightness) {
    _weights[b.first] = b.second / totalBrightness;
    getRig()->getDevice(b.first)->setMetadata("brightnessAttributeWeight", String(b.second).toStdString());
  }

  delete s;
}

double BrightAttribute::getLightIntens(Device * d)
{
  return d->getColor()->getLab(ReferenceWhite::D65)[0] * d->getIntensity()->asPercent();
}

