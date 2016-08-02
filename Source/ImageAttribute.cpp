/*
  ==============================================================================

    ImageAttribute.cpp
    Created: 1 Aug 2016 5:24:38pm
    Author:  eshimizu

  ==============================================================================
*/

#include "ImageAttribute.h"

ImageAttribute::ImageAttribute(string name, string filepath, int n) : HistogramAttribute(name, 100, 100),
  _sourceHist(Histogram3D(n))
{
  File img(filepath);
  FileInputStream in(img);

  if (in.openedOk()) {
    // load image
    PNGImageFormat pngReader;
    _sourceImg = pngReader.decodeImage(in);
    _sourceImg = _sourceImg.rescaled(_canonicalWidth, _canonicalHeight);

    getRecorder()->log(SYSTEM, "Loaded image for attribute " + name);
  }
  else {
    getRecorder()->log(SYSTEM, "Failed to load image for attribute " + name);
  }
}

ImageAttribute::ImageAttribute(string name, Image img, int n) : HistogramAttribute(name, 100, 100),
  _sourceHist(Histogram3D(n))
{
  _sourceImg = img.rescaled(_canonicalWidth, _canonicalHeight);
}

ImageAttribute::~ImageAttribute()
{
}

double ImageAttribute::evaluateScene(Snapshot * s)
{
  Image current = generateImage(s);
  Histogram3D currentHist = getLabHist(current, 10);

  double diff = currentHist.L2dist(_sourceHist);

  return 100 - (diff / 100.0);
}

void ImageAttribute::preProcess()
{
  _sourceHist = getLabHist(_sourceImg, 10);
}
