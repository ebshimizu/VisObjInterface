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
#include "KMeans.h"
#include "MainComponent.h"

GibbsPalette::GibbsPalette(String name, Image & src) : _img(src)
{
  _name = name.replaceCharacter(' ', '_');
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
  else if (event.mods.isRightButtonDown()) {
    PopupMenu menu;
    menu.addItem(1, "New Concept");

    int result = menu.show();

    if (result == 1) {
      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

      if (mc != nullptr) {
        mc->createIdea(_img, _name);
      }
    }
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
  int rows = (int)ceil((float)_pallets.size() / _cols);

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
  _weight = 1;
  _color = Colours::white;
  addAndMakeVisible(_setColor);
  _setColor.addListener(this);

  _weightInput.addListener(this);
  _weightInput.setMultiLine(false, false);
  _weightInput.setInputRestrictions(10, "0123456789.");
  _weightInput.setText(String(_weight), dontSendNotification);
  addAndMakeVisible(_weightInput);
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

  auto weight = lbounds.removeFromRight(80);
  auto weightBox = weight.removeFromBottom(24);
  _weightInput.setBounds(weightBox.reduced(2));
}

void GibbsColorConstraint::buttonClicked(Button * /* b */)
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
  hue = normal_distribution<float>(_color.getHue(), 0.01f);
  sat = normal_distribution<float>(_color.getSaturation(), 0.1f);
  val = normal_distribution<float>(_color.getBrightness(), 0.1f);
}


void GibbsColorConstraint::changeListenerCallback(ChangeBroadcaster * source)
{
  HSVPicker* cs = dynamic_cast<HSVPicker*>(source);
  _color = cs->getCurrentColour();

  repaint();
}

void GibbsColorConstraint::setSelectedColor(Colour c)
{
  _color = c;
}

void GibbsColorConstraint::setColorWeight(float weight)
{
  _weight = weight;
  _weightInput.setText(String(weight), false);
}

void GibbsColorConstraint::textEditorTextChanged(TextEditor & e)
{
  _weight = e.getText().getFloatValue();
}

float GibbsColorConstraint::getColorWeight()
{
  return _weight;
}

GibbsConstraintContainer::GibbsConstraintContainer()
{
  _intens.setName("intens");
  _intens.setSliderStyle(Slider::SliderStyle::TwoValueHorizontal);
  _intens.addListener(this);
  _intens.setRange(0, 1);
  _intens.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
  _intens.setPopupDisplayEnabled(true, nullptr);
  addAndMakeVisible(_intens);

  _numLights.setName("bright");
  _numLights.addListener(this);
  _numLights.setRange(0, (double)getRig()->getAllDevices().size(), 1);
  addAndMakeVisible(_numLights);

  _add.setName("add");
  _add.addListener(this);
  _add.setButtonText("Add Color");
  addAndMakeVisible(_add);

  _intens.setMinAndMaxValues(0.3, 0.8);
  _k = 2;

  _scope.setName("scope");
  _scope.addItem("Lights", 1);
  _scope.addItem("Systems", 2);
  _scope.setSelectedId(1, dontSendNotification);
  _scope.addListener(this);
  addAndMakeVisible(_scope);

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
  auto lbounds = getLocalBounds();
  lbounds.removeFromTop(40 * _colors.size());

  // labels
  auto intensLabel = lbounds.removeFromTop(30).removeFromLeft(60);
  g.setColour(Colours::white);
  g.drawFittedText("Target Intensity", intensLabel, Justification::centredLeft, 2);

  auto lightLabel = lbounds.removeFromTop(30).removeFromLeft(60);
  g.drawFittedText("Birght Lights", lightLabel, Justification::centredLeft, 2);
}

void GibbsConstraintContainer::resized()
{
  // TODO: MAGIC NUMBERS BAD
  int height = 40 * _colors.size() + 30 * 3;
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

  auto intensRow = lbounds.removeFromTop(30);
  intensRow.removeFromLeft(60);
  _intens.setBounds(intensRow);

  auto lightsRow = lbounds.removeFromTop(30);
  lightsRow.removeFromLeft(60);
  _numLights.setBounds(lightsRow);

  auto bot = lbounds.removeFromTop(30);
  _add.setBounds(bot.removeFromRight(100).reduced(2));
  _scope.setBounds(bot.removeFromLeft(100).reduced(2));
}

void GibbsConstraintContainer::sliderValueChanged(Slider * /* s */)
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
  return normal_distribution<float>((float)_intens.getMinValue());
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

void GibbsConstraintContainer::addColors(vector<Eigen::VectorXd> colors, double intens, vector<float> weights)
{
  for (auto c : _colors)
    delete c;
  _colors.clear();

  for (auto b : _deleteButtons)
    delete b;
  _deleteButtons.clear();

  for (int i = 0; i < colors.size(); i++) {
    GibbsColorConstraint* gc = new GibbsColorConstraint();
    addAndMakeVisible(gc);
    gc->setSelectedColor(Colour((float)colors[i][0], (float)colors[i][1], (float)colors[i][2], 1.0f));
    _colors.add(gc);
    gc->setName(String(_counter));
    gc->setColorWeight(weights[i]);

    TextButton* del = new TextButton();
    del->setName(String(_counter));
    del->setButtonText("x");
    del->setTooltip("Delete Constraint");
    del->addListener(this);
    addAndMakeVisible(del);
    _deleteButtons.add(del);

    _counter++;
  }

  _intens.setMinValue(intens, dontSendNotification);

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
    getParentComponent()->resized();
  }
  else if (b->getName() == "add") {
    addColorConstraint();
    getParentComponent()->resized();
  }
}

void GibbsConstraintContainer::getIntensDistProps(double & avg, double & max, int & k)
{
  avg = _intens.getMinValue();
  max = _intens.getMaxValue();
  k = (int) _numLights.getValue();
}

bool GibbsConstraintContainer::useSystems()
{
  return _scope.getSelectedId() == 2;
}

void GibbsConstraintContainer::updateBounds()
{
  if (_scope.getSelectedId() == 1)
    _numLights.setRange(0, (double)getRig()->getAllDevices().size(), 1);

  else if (_scope.getSelectedId() == 2)
    _numLights.setRange(0, (double)getRig()->getMetadataValues("system").size(), 1);
}

void GibbsConstraintContainer::comboBoxChanged(ComboBox * /* b */)
{
  updateBounds();
}

vector<float> GibbsConstraintContainer::getColorWeights()
{
  // normalize
  vector<float> weights;
  weights.resize(_colors.size());
  double sum = 0;
  for (auto c : _colors) {
    sum += c->getColorWeight();
  }

  for (int i = 0; i < weights.size(); i++) {
    weights[i] = (float)(_colors[i]->getColorWeight() / sum);
  }

  return weights;
}
