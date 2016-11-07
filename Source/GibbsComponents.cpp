/*
  ==============================================================================

    GibbsComponents.cpp
    Created: 2 Nov 2016 4:25:39pm
    Author:  falindrith

  ==============================================================================
*/

#include "GibbsComponents.h"
#include "ImageAttribute.h"
#include "hsvPicker.h"

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
  SparseHistogram imgHist(3, { 0, 10, 0, 0.2f, 0, 0.2f });
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

  // temporary short circuit
  return;

  // determine what the top hue bins are, i.e. where most of the colors lie.
  // ideally we'll also want to determine the bandwidth of these histograms as well
  // actually it's basically just fitting a gaussian model to the histogram
  float totalWeight = imgHist.getTotalWeight();
  map<double, double> hues = imgHist.getDimension(0);

  // determine peak hue values and roughly fit a gaussian
  float currentWeight = totalWeight;
  while (currentWeight > totalWeight * 0.2) {
    // find max hue value
    double maxHue = -1;

    for (auto h : hues) {
      if (maxHue < 0) {
        maxHue = h.first;
        continue;
      }

      if (h.second > hues[maxHue]) {
        maxHue = h.first;
      }
    }

    // have peak, determine how long the tail is (generally if constantly decreasing consider part of tail)
    double currentHue = maxHue;
    double nextHue = maxHue - 10;
    if (nextHue < 0)
      nextHue += 360;

    while (hues[currentHue] > hues[nextHue]) {
      currentHue = nextHue;
      double nextHue = currentHue - 10;
      if (nextHue < 0)
        nextHue += 360;
    } 
  }
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

GibbsColorConstraint::GibbsColorConstraint() : _setColor("Select Color")
{
  _color = Colours::white;
  addAndMakeVisible(_setColor);
  _setColor.addListener(this);
}

void GibbsColorConstraint::paint(Graphics & g)
{
  auto lbounds = getLocalBounds();

  g.fillAll(Colour(0xff333333));
  g.setColour(_color);

  g.fillRect(lbounds.removeFromLeft(80).reduced(3));
}

void GibbsColorConstraint::resized()
{
  auto lbounds = getLocalBounds();
  lbounds.removeFromLeft(80);
  _setColor.setBounds(lbounds.removeFromRight(100).reduced(3));
}

void GibbsColorConstraint::buttonClicked(Button * b)
{
  // there's only one button here so
  HSVPicker* cs = new HSVPicker();
  cs->setName("Constraint");
  cs->setCurrentColour(_color);
  cs->setSize(300, 400);
  cs->addChangeListener(this);
  CallOutBox::launchAsynchronously(cs, this->getScreenBounds(), nullptr);
}

void GibbsColorConstraint::getDistributions(normal_distribution<float>& hue, normal_distribution<float>& sat, normal_distribution<float>& val)
{
  hue = normal_distribution<float>(_color.getHue(), 0.03);
  sat = normal_distribution<float>(_color.getSaturation(), 0.1);
  val = normal_distribution<float>(_color.getBrightness(), 0.1);
}


void GibbsColorConstraint::changeListenerCallback(ChangeBroadcaster * source)
{
  HSVPicker* cs = dynamic_cast<HSVPicker*>(source);
  _color = cs->getCurrentColour();

  repaint();
}

GibbsConstraintContainer::GibbsConstraintContainer()
{
  _intens.setName("intens");
  _intens.addListener(this);
  _intens.setRange(0, 1);
  addAndMakeVisible(_intens);

  for (int i = 0; i < 4; i++) {
    GibbsColorConstraint* c1 = new GibbsColorConstraint();
    addAndMakeVisible(c1);
    _colors.add(c1);
  }
}

GibbsConstraintContainer::~GibbsConstraintContainer()
{
  for (auto c : _colors) {
    delete c;
  }
}

void GibbsConstraintContainer::paint(Graphics & g)
{
}

void GibbsConstraintContainer::resized()
{
  int height = 50 * _colors.size() + 30;
  setSize(getWidth(), height);

  auto lbounds = getLocalBounds();
  for (auto c : _colors) {
    c->setBounds(lbounds.removeFromTop(50));
  }

  _intens.setBounds(lbounds.removeFromTop(30));
}

void GibbsConstraintContainer::sliderValueChanged(Slider * s)
{
}

vector<vector<normal_distribution<float>>> GibbsConstraintContainer::getColorDists()
{
  vector<vector<normal_distribution<float>>> dists;
  dists.resize(3);

  for (int i = 0; i < _colors.size(); i++) {
    dists[0].push_back(normal_distribution<float>());
    dists[1].push_back(normal_distribution<float>());
    dists[2].push_back(normal_distribution<float>());

    _colors[i]->getDistributions(dists[0][i], dists[1][i], dists[2][i]);
  }

  return dists;
}

normal_distribution<float> GibbsConstraintContainer::getIntensDist()
{
  return normal_distribution<float>(_intens.getValue());
}
