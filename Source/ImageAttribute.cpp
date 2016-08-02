/*
  ==============================================================================

    ImageAttribute.cpp
    Created: 1 Aug 2016 5:24:38pm
    Author:  eshimizu

  ==============================================================================
*/

#include "ImageAttribute.h"

ImageDrawer::ImageDrawer(Image i) : _img(i) {
}

ImageDrawer::~ImageDrawer()
{
}

void ImageDrawer::resized()
{
}

void ImageDrawer::paint(Graphics & g)
{
  g.drawImageWithin(_img, 0, 0, getWidth(), getHeight(), RectanglePlacement::centred);
}

ImageAttribute::ImageAttribute(string name, string filepath, int n) : HistogramAttribute(name, 100, 100),
  _sourceHist(Histogram3D(n)), _n(n)
{
  File img(filepath);
  FileInputStream in(img);

  if (in.openedOk()) {
    // load image
    PNGImageFormat pngReader;
    _originalImg = pngReader.decodeImage(in);
    _sourceImg = _originalImg.rescaled(_canonicalWidth, _canonicalHeight);

    getRecorder()->log(SYSTEM, "Loaded image for attribute " + name);
  }
  else {
    getRecorder()->log(SYSTEM, "Failed to load image for attribute " + name);
  }

  _showImgButton.setButtonText("Show Image");
  _showImgButton.setName("Show Image");
  _showImgButton.addListener(this);
  addAndMakeVisible(_showImgButton);
}

ImageAttribute::ImageAttribute(string name, Image img, int n) : HistogramAttribute(name, 100, 100),
  _sourceHist(Histogram3D(n)), _n(n)
{
  _originalImg = img;
  _sourceImg = img.rescaled(_canonicalWidth, _canonicalHeight);

  _showImgButton.setButtonText("Show Image");
  _showImgButton.setName("Show Image");
  _showImgButton.addListener(this);
  addAndMakeVisible(_showImgButton);
}

ImageAttribute::~ImageAttribute()
{
}

double ImageAttribute::evaluateScene(Snapshot * s)
{
  Image current = generateImage(s);
  Histogram3D currentHist = getLabHist(current, _n);

  double diff = currentHist.L2dist(_sourceHist);

  return 100 - (diff / 100.0);
}

void ImageAttribute::preProcess()
{
  _sourceHist = getLabHist(_sourceImg, _n);
}

void ImageAttribute::resized()
{
  HistogramAttribute::resized();

  auto lbounds = getLocalBounds();
  auto top = lbounds.removeFromTop(24);

  top.removeFromRight(80);
  _showImgButton.setBounds(top.removeFromRight(80).reduced(2));
}

void ImageAttribute::buttonClicked(Button * b)
{
  HistogramAttribute::buttonClicked(b);

  if (b->getName() == "Show Image") {
    Viewport* vp = new Viewport();
    ImageDrawer* id = new ImageDrawer(_originalImg);
    id->setSize(500, 500);
    vp->setViewedComponent(id, true);
    vp->setSize(id->getWidth(), id->getHeight());

    CallOutBox::launchAsynchronously(vp, _showImgButton.getScreenBounds(), nullptr);
  }
}
