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

GibbsPalette::GibbsPalette(String name, Image & src) : _img(src), _name(name)
{
  computeSchedule();
  setTooltip(name);
}

void GibbsPalette::paint(Graphics & g)
{
  auto localb = getLocalBounds();
  g.drawImageWithin(_img, localb.getX(), localb.getY(), localb.getWidth(), localb.getHeight(), RectanglePlacement::centred);
}

void GibbsPalette::resized()
{
}

void GibbsPalette::setImg(Image & img)
{
  _img = img;
}

void GibbsPalette::computeSchedule()
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

void GibbsPalette::setCustomSchedule(shared_ptr<GibbsSchedule> sched)
{
  _schedule = sched;
}

void GibbsPalette::mouseDown(const MouseEvent & event)
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

shared_ptr<GibbsSchedule> GibbsPalette::getSchedule()
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

void GibbsPalletContainer::addPallet(GibbsPalette * pallet)
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

Array<GibbsPalette*> GibbsPalletContainer::getPallets()
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
  hue = normal_distribution<float>(_color.getHue(), 0.01);
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

  _add.setName("add");
  _add.addListener(this);
  _add.setButtonText("Add Color");
  addAndMakeVisible(_add);

  for (int i = 0; i < 4; i++) {
    addColorConstraint();
  }
}

GibbsConstraintContainer::~GibbsConstraintContainer()
{
  for (auto c : _colors) {
    delete c;
  }

  for (auto b : _deleteButtons) {
    delete b;
  }
}

void GibbsConstraintContainer::paint(Graphics & g)
{
}

void GibbsConstraintContainer::resized()
{
  // TODO: MAGIC NUMBERS BAD
  int height = 40 * _colors.size() + 30 * 2;
  setSize(getWidth(), height);

  auto lbounds = getLocalBounds();
  int i = 0;
  for (auto c : _colors) {
    auto top = lbounds.removeFromTop(40);
    auto left = top.removeFromLeft(25);
    _deleteButtons[i]->setBounds(left.removeFromTop(25).reduced(2));
    c->setBounds(top);
    i++;
  }

  _intens.setBounds(lbounds.removeFromTop(30));

  _add.setBounds(lbounds.removeFromTop(30).removeFromRight(100).reduced(2));
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

void GibbsConstraintContainer::addColorConstraint()
{
  GibbsColorConstraint* c = new GibbsColorConstraint();
  addAndMakeVisible(c);
  _colors.add(c);
  c->setName(String(_counter));

  TextButton* del = new TextButton();
  del->setName(String(_counter));
  del->setButtonText("x");
  del->setTooltip("Delete Constraint");
  del->addListener(this);
  addAndMakeVisible(del);
  _deleteButtons.add(del);

  _counter++;
  resized();
}

void GibbsConstraintContainer::buttonClicked(Button * b)
{
  if (b->getButtonText() == "x") {
    // delete button
    TextButton* tb = dynamic_cast<TextButton*>(b);

    GibbsColorConstraint* toDelete = nullptr;
    for (auto c : _colors) {
      if (c->getName() == tb->getName()) {
        toDelete = c;
      }
    }

    _deleteButtons.removeAllInstancesOf(tb);
    _colors.removeAllInstancesOf(toDelete);
    delete tb;
    delete toDelete;

    resized();
  }
  else if (b->getName() == "add") {
    addColorConstraint();
  }
}
