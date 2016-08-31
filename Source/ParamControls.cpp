/*
  ==============================================================================

    ParamControls.cpp
    Created: 15 Dec 2015 5:07:12pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "ParamControls.h"
#include "MainComponent.h"
#include <sstream>

//==============================================================================
// Properties
FloatPropertySlider::FloatPropertySlider(string id, string param, LumiverseFloat* val) :
  SliderPropertyComponent(param, val->getMin(), val->getMax(), 0.001)
{
  _id = id;
  _param = param;

  if (_param == "intensity") {
    slider.setValue(val->asPercent());
    slider.setRange(0, 100, 0.01);
    slider.setTextValueSuffix("%");
  }
  else {
    slider.setValue(val->getVal());
  }
}

FloatPropertySlider::~FloatPropertySlider()
{
}

void FloatPropertySlider::paint(Graphics & g)
{
  LookAndFeel& lf = getLookAndFeel();

  if (isDeviceParamLocked(_id, _param)) {
    g.setColour(Colour(0xFFFF3838));
  }
  else {
    g.setColour(this->findColour(PropertyComponent::backgroundColourId));
  }

  g.fillRect(0, 0, getWidth(), getHeight() - 1);

  lf.drawPropertyComponentLabel(g, getWidth(), getHeight(), *this);
}

void FloatPropertySlider::mouseDown(const MouseEvent & event)
{
  if (event.mods.isRightButtonDown()) {
    if (isDeviceParamLocked(_id, _param)) {
      unlockDeviceParam(_id, _param);
    }
    else {
      lockDeviceParam(_id, _param);
    }

    getApplicationCommandManager()->invokeDirectly(command::REFRESH_PARAMS, false);
  }
}

void FloatPropertySlider::setValue(double newValue)
{
  if (_param == "intensity") {
    getRig()->getDevice(_id)->getParam<LumiverseFloat>(_param)->setValAsPercent((float) newValue / 100.0f);
  }
  else {
    getRig()->getDevice(_id)->setParam(_param, (float)newValue);
  }

  getGlobalSettings()->invalidateCache();
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_ATTR, true);
}

double FloatPropertySlider::getValue() const
{
  LumiverseFloat* val = getRig()->getDevice(_id)->getParam<LumiverseFloat>(_param);

  if (_param == "intensity") {
    return val->asPercent() * 100;
  }
  else {
    return val->getVal();
  }
}

void FloatPropertySlider::sliderDragStarted(Slider * /* s */)
{
  stringstream ss;
  ss << _id << ":" << _param << " change start at " << getValue();
  getRecorder()->log(ACTION, ss.str());
}

void FloatPropertySlider::sliderDragEnded(Slider * /* s */)
{
  stringstream ss;
  ss << _id << ":" << _param << " value changed to " << getValue();
  getStatusBar()->setStatusMessage(ss.str());
  getRecorder()->log(ACTION, ss.str());
}

OrientationPropertySlider::OrientationPropertySlider(string id, string param, LumiverseOrientation * val) :
  SliderPropertyComponent(param, val->getMin(), val->getMax(), 0.001)
{
  _id = id;
  _param = param;
  slider.setValue(val->getVal());
}

OrientationPropertySlider::~OrientationPropertySlider()
{
}

void OrientationPropertySlider::paint(Graphics & g)
{
  LookAndFeel& lf = getLookAndFeel();

  if (isDeviceParamLocked(_id, _param)) {
    g.setColour(Colour(0xFFFF3838));
  }
  else {
    g.setColour(this->findColour(PropertyComponent::backgroundColourId));
  }

  g.fillRect(0, 0, getWidth(), getHeight() - 1);

  lf.drawPropertyComponentLabel(g, getWidth(), getHeight(), *this);
}

void OrientationPropertySlider::mouseDown(const MouseEvent & event)
{
  if (event.mods.isRightButtonDown()) {
    if (isDeviceParamLocked(_id, _param)) {
      unlockDeviceParam(_id, _param);
    }
    else {
      lockDeviceParam(_id, _param);
    }

    getApplicationCommandManager()->invokeDirectly(command::REFRESH_PARAMS, false);
  }
}

void OrientationPropertySlider::setValue(double newValue)
{
  getRig()->getDevice(_id)->setParam(_param, (float)newValue);
  getGlobalSettings()->invalidateCache();
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_ATTR, true);
}

double OrientationPropertySlider::getValue() const
{
  LumiverseOrientation* val = getRig()->getDevice(_id)->getParam<LumiverseOrientation>(_param);
  return val->getVal();
}

void OrientationPropertySlider::sliderDragStarted(Slider * /* s */)
{
  stringstream ss;
  ss << _id << ":" << _param << " change start at " << getValue();
  getRecorder()->log(ACTION, ss.str());
}

void OrientationPropertySlider::sliderDragEnded(Slider * /* s */)
{
  stringstream ss;
  ss << _id << ":" << _param << " value changed to " << getValue();
  getStatusBar()->setStatusMessage(ss.str());
  getRecorder()->log(ACTION, ss.str());
}

ColorPickerButton::ColorPickerButton(string id, string param, LumiverseColor * val) :
  _id(id), _param(param), _val(val)
{
	_button = new ColoredTextButton("Set Color");
	addAndMakeVisible(_button);
	_button->setTriggeredOnMouseDown(false);
	_button->addListener(this);

	Eigen::Vector3d c;
	c[0] = _val->getColorChannel("Red");
	c[1] = _val->getColorChannel("Green");
	c[2] = _val->getColorChannel("Blue");
	c *= 255;

	_button->setColor(Colour((uint8) c[0], (uint8) c[1], (uint8) c[2]));
}

ColorPickerButton::~ColorPickerButton()
{
	delete _button;
}

void ColorPickerButton::paint(Graphics & g)
{
  LookAndFeel& lf = getLookAndFeel();

  if (isDeviceParamLocked(_id, _param)) {
    g.setColour(Colour(0xFFFF3838));
  }
}

void ColorPickerButton::changeListenerCallback(ChangeBroadcaster * source)
{
  ColourSelector* cs = dynamic_cast<ColourSelector*>(source);
  if (!isDeviceParamLocked(_id, _param) && cs != nullptr) {
    Colour current = cs->getCurrentColour();
    _val->setColorChannel("Red", current.getFloatRed());
    _val->setColorChannel("Green", current.getFloatGreen());
    _val->setColorChannel("Blue", current.getFloatBlue());

    stringstream ss;
    ss << _id << ":" << _param << " value changed to " << _val->asString();
    getStatusBar()->setStatusMessage(ss.str());
    getRecorder()->log(ACTION, ss.str());

    getGlobalSettings()->invalidateCache();

    MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

    if (mc != nullptr) {
      mc->refreshParams();
      mc->refreshAttr();
    }
  }
}

void ColorPickerButton::changeId(string newId)
{
  _id = newId;
  _val = getRig()->getDevice(_id)->getColor();

  refresh();
}

void ColorPickerButton::resized()
{
  _button->setBounds(getLocalBounds().reduced(2));
}

void ColorPickerButton::buttonClicked(Button* /* b */)
{
  Eigen::Vector3d c;
  c[0] = _val->getColorChannel("Red");
  c[1] = _val->getColorChannel("Green");
  c[2] = _val->getColorChannel("Blue");
  c *= 255;

  ColourSelector* cs = new ColourSelector(ColourSelector::showColourAtTop|ColourSelector::showSliders|ColourSelector::showColourspace);
  cs->setName(_id + " " + _param);
  cs->setCurrentColour(Colour((uint8)c[0], (uint8)c[1], (uint8)c[2]));
  cs->setSize(300, 400);
  cs->addChangeListener(this);
  CallOutBox::launchAsynchronously(cs, this->getScreenBounds(), nullptr);
}

void ColorPickerButton::refresh()
{
	Eigen::Vector3d c;
	c[0] = _val->getColorChannel("Red");
	c[1] = _val->getColorChannel("Green");
	c[2] = _val->getColorChannel("Blue");
	c *= 255;

	_button->setColor(Colour((uint8) c[0], (uint8) c[1], (uint8) c[2]));

	_button->repaint();
}

//==============================================================================
ParamControls::ParamControls() : _groupColor("Group Color", true)
{
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.

  addAndMakeVisible(_table);
  _table.setModel(this);

  _table.getHeader().addColumn("ID", 1, 100, 30, -1, TableHeaderComponent::ColumnPropertyFlags::notSortable);
  _table.getHeader().addColumn("Intensity", 2, 100, TableHeaderComponent::ColumnPropertyFlags::notSortable);
  _table.getHeader().addColumn("IL", 3, 20, TableHeaderComponent::ColumnPropertyFlags::notSortable | TableHeaderComponent::ColumnPropertyFlags::notResizable);
  _table.getHeader().addColumn("Color", 4, 100, TableHeaderComponent::ColumnPropertyFlags::notSortable);
  _table.getHeader().addColumn("CL", 5, 20, TableHeaderComponent::ColumnPropertyFlags::notSortable | TableHeaderComponent::ColumnPropertyFlags::notResizable);


  _table.setMultipleSelectionEnabled(true);
  _table.setColour(ListBox::ColourIds::backgroundColourId, Colour(0xff222222));

  _groupIntens.addListener(this);
  _groupIntens.setName("Group Intensity");
  _groupIntens.setSliderStyle(Slider::SliderStyle::LinearBar);
  _groupIntens.setRange(0, 100, 0.01);
  _groupIntens.setColour(Slider::ColourIds::backgroundColourId, Colour(0xff929292));
  addAndMakeVisible(_groupIntens);

  _groupColor.addListener(this);
  addAndMakeVisible(_groupColor);
}

ParamControls::~ParamControls()
{
}

void ParamControls::paint (Graphics& g)
{
  auto b = getLocalBounds();

  auto top = b.removeFromTop(100);

  g.fillAll(Colour(0xff333333));

  g.setColour(Colours::white);
  g.drawFittedText("Selected: " + String(_selected.size()), top.removeFromTop(26).reduced(5), Justification::left, 2);

  g.drawFittedText("Intensity", top.removeFromTop(26).removeFromLeft(80).reduced(5), Justification::left, 1);

  g.drawFittedText("Color", top.removeFromTop(26).removeFromLeft(80).reduced(5), Justification::left, 1);
}

void ParamControls::resized()
{
  auto b = getLocalBounds();

  b.removeFromTop(26);
  auto intens = b.removeFromTop(26);
  intens.removeFromLeft(80);
  _groupIntens.setBounds(intens.reduced(2));

  auto color = b.removeFromTop(26);
  color.removeFromLeft(80);
  _groupColor.setBounds(color.reduced(2));

  _table.setBounds(b);
}

void ParamControls::initProperties()
{
  Rig* rig = getRig();
  auto devices = rig->getAllDevices();

  for (const auto d : devices.getDevices()) {
    _ids.add(d->getId());
  }

  _ids.sort(true);
}

void ParamControls::refreshParams() {
  _properties.refreshAll();
  _table.updateContent();
}

int ParamControls::getNumRows()
{
  return _ids.size();
}

void ParamControls::paintRowBackground(Graphics & g, int rowNumber, int width, int height, bool rowIsSelected)
{
  if (rowIsSelected)
    g.fillAll(Colours::lightblue);
  else
    g.fillAll(Colour(0xff222222));
}

void ParamControls::paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
  if (rowIsSelected) {
    g.setColour(Colours::black);
  }
  else {
    g.setColour(Colours::white);
  }

  g.setFont(14);

  if (columnId == 1) {
    String text(_ids[rowNumber]);
    g.drawText(text, 2, 0, width - 4, height, Justification::centredLeft, true);
  }
  else if (columnId == 2) {
    g.setColour(Colour(0xff929292));
    g.fillRect(0, 0, width, height);
  }
  else if (columnId == 3) {
    if (isDeviceParamLocked(_ids[rowNumber].toStdString(), "intensity")) {
      g.setColour(Colours::red);
      g.fillRect(2, 2, width - 2, height - 2);
    }
    g.setColour(Colours::white);
    g.drawRect(2, 2, width - 2, height - 2, 1);
  }
  else if (columnId == 5) {
    if (isDeviceParamLocked(_ids[rowNumber].toStdString(), "color")) {
      g.setColour(Colours::red);
      g.fillRect(2, 2, width - 2, height - 2);
    }
    g.setColour(Colours::white);
    g.drawRect(2, 2, width - 2, height - 2, 1);
  }
}

Component * ParamControls::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component * existingComponentToUpdate)
{
  if (columnId == 2) {
    Slider* intensSlider = static_cast<Slider*>(existingComponentToUpdate);

    if (intensSlider == nullptr) {
      intensSlider = new Slider();
      intensSlider->setRange(0, 100, 0.01);
      intensSlider->setSliderStyle(Slider::SliderStyle::LinearBar);
      intensSlider->setColour(Slider::ColourIds::backgroundColourId, Colour(0xa0929292));
      intensSlider->addListener(this);
    }

    intensSlider->setName(_ids[rowNumber]);
    intensSlider->setValue(getRig()->getDevice(_ids[rowNumber].toStdString())->getIntensity()->asPercent() * 100, dontSendNotification);
    return intensSlider;
  }
  if (columnId == 4) {
    ColorPickerButton* button = static_cast<ColorPickerButton*>(existingComponentToUpdate);

    string id = _ids[rowNumber].toStdString();
    LumiverseColor* targetVal = getRig()->getDevice(id)->getColor();

    if (button == nullptr) {
      button = new ColorPickerButton(id, "color", targetVal);
    }

    button->changeId(id);
    return button;
  }

  return nullptr;
}

void ParamControls::selectedRowsChanged(int lastRowSelected)
{
  _selected.clear();

  for (int i = 0; i < _table.getNumSelectedRows(); i++) {
    _selected.add(_ids[_table.getSelectedRow(i)]);
  }

  // pull most recently selected device's intensity and color
  string recentId = _ids[lastRowSelected].toStdString();
  _recentIntens = getRig()->getDevice(recentId)->getIntensity()->asPercent();

  LumiverseColor* recentColor = getRig()->getDevice(recentId)->getColor();
  Eigen::Vector3d c;
  c[0] = recentColor->getColorChannel("Red");
  c[1] = recentColor->getColorChannel("Green");
  c[2] = recentColor->getColorChannel("Blue");
  c *= 255;
  _recentColor = Colour((uint8)c[0], (uint8)c[1], (uint8)c[2]);

  _groupColor.setColor(_recentColor);
  _groupIntens.setValue(_recentIntens * 100, dontSendNotification);

  repaint();
}

void ParamControls::cellClicked(int rowNumber, int columnId, const MouseEvent & e)
{
  if (columnId == 3) {
    toggleDeviceParamLock(_ids[rowNumber].toStdString(), "intensity");
    _table.updateContent();
    repaint();
  }
  else if (columnId == 5) {
    toggleDeviceParamLock(_ids[rowNumber].toStdString(), "color");
    _table.updateContent();
    repaint();
  }
}

void ParamControls::sliderValueChanged(Slider * s)
{
  if (s->getName() == "Group Intensity") {
    float val = s->getValue() / 100.0f;
    for (auto& id : _selected) {
      getRig()->getDevice(id.toStdString())->getIntensity()->setValAsPercent(val);
    }

    refreshParams();
  }
  else {
    getRig()->getDevice(s->getName().toStdString())->getIntensity()->setValAsPercent(s->getValue() / 100);
  }

  getGlobalSettings()->invalidateCache();
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_ATTR, true);
}

void ParamControls::buttonClicked(Button * b)
{
  if (b->getName() == "Group Color") {
    ColourSelector* cs = new ColourSelector(ColourSelector::showColourAtTop | ColourSelector::showSliders | ColourSelector::showColourspace);
    cs->setName("Group Color");
    cs->setCurrentColour(_recentColor);
    cs->setSize(300, 400);
    cs->addChangeListener(this);
    CallOutBox::launchAsynchronously(cs, _groupColor.getScreenBounds(), nullptr);
  }
}

void ParamControls::changeListenerCallback(ChangeBroadcaster * source)
{
  ColourSelector* cs = dynamic_cast<ColourSelector*>(source);
  if (cs != nullptr) {
    _recentColor = cs->getCurrentColour();

    for (auto& id : _selected) {
      LumiverseColor* val = getRig()->getDevice(id.toStdString())->getColor();

      val->setColorChannel("Red", _recentColor.getFloatRed());
      val->setColorChannel("Green", _recentColor.getFloatGreen());
      val->setColorChannel("Blue", _recentColor.getFloatBlue());
    }

    _groupColor.setColor(_recentColor);
    getGlobalSettings()->invalidateCache();

    MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

    if (mc != nullptr) {
      mc->refreshParams();
      mc->refreshAttr();
    }
  }
}
