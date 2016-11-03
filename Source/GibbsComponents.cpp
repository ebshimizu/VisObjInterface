/*
  ==============================================================================

    GibbsComponents.cpp
    Created: 2 Nov 2016 4:25:39pm
    Author:  falindrith

  ==============================================================================
*/

#include "GibbsComponents.h"
#include "ImageAttribute.h"

GibbsPallet::GibbsPallet(String name, Image & src) : _img(src), _name(name)
{
  computeSchedule();
  setTooltip(name);
}

void GibbsPallet::paint(Graphics & g)
{
  auto localb = getLocalBounds();
  g.drawImageWithin(_img, localb.getX(), localb.getY(), localb.getWidth(), localb.getHeight(), RectanglePlacement::centred);
}

void GibbsPallet::resized()
{
}

void GibbsPallet::setImg(Image & img)
{
  _img = img;
}

void GibbsPallet::computeSchedule()
{
  // compute a HSV histogram and pull out stats about the intensity and color
  SparseHistogram imgHist(3, { 10, 0, 0.2f, 0, 0.2f, 0 });
  Image i = _img.rescaled(100, 100);

  for (int y = 0; y < i.getHeight(); y++) {
    for (int x = 0; x < i.getWidth(); x++) {
      auto p = i.getPixelAt(x, y);
      vector<float> hsv;
      hsv.resize(3);

      p.getHSB(hsv[0], hsv[1], hsv[2]);
      hsv[0] *= 360;

      imgHist.add(hsv);
    }
  }

  // determine what the top hue bins are, i.e. where most of the colors lie.
  // ideally we'll also want to determine the bandwidth of these histograms as well
  // actually it's basically just fitting a gaussian model to the histogram
  imgHist;
}

void GibbsPallet::setCustomSchedule(shared_ptr<GibbsSchedule> sched)
{
  _schedule = sched;
}

void GibbsPallet::mouseDown(const MouseEvent & event)
{
  if (event.mods.isLeftButtonDown()) {
    Viewport* vp = new Viewport();
    ImageDrawer* id = new ImageDrawer(_img);
    id->setSize(500, 500);
    vp->setViewedComponent(id, true);
    vp->setSize(id->getWidth(), id->getHeight());

    CallOutBox::launchAsynchronously(vp, this->getScreenBounds(), nullptr);
  }
}

shared_ptr<GibbsSchedule> GibbsPallet::getSchedule()
{
  return _schedule;
}

//=============================================================================

GibbsPalletContainer::GibbsPalletContainer(int c) : _cols(c)
{
}

GibbsPalletContainer::~GibbsPalletContainer()
{
  for (auto p : _pallets)
    delete p;
}

void GibbsPalletContainer::paint(Graphics & g)
{
  g.fillAll(Colour(0xff333333));
}

void GibbsPalletContainer::resized()
{
  // this component maintains its own height
  int elemWidth = getWidth() / _cols;
  int elemHeight = elemWidth;
  int rows = ceil((float)_pallets.size() / _cols);

  setSize(getWidth(), elemHeight * rows);

  // place components
  auto lbounds = getLocalBounds();
  Rectangle<int> row;
  for (int i = 0; i < _pallets.size(); i++) {
    if (i % _cols == 0)
      row = lbounds.removeFromTop(elemHeight);

    _pallets[i]->setBounds(row.removeFromLeft(elemWidth).reduced(3));
  }
}

void GibbsPalletContainer::addPallet(GibbsPallet * pallet)
{
  _pallets.add(pallet);
  addAndMakeVisible(pallet);
  resized();
}

void GibbsPalletContainer::setCols(int c)
{
  _cols = c;
  resized();
}

void GibbsPalletContainer::clearPallets()
{
  for (auto p : _pallets)
    delete p;

  _pallets.clear();
  resized();
}

Array<GibbsPallet*> GibbsPalletContainer::getPallets()
{
  return _pallets;
}
