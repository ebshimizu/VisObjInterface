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
