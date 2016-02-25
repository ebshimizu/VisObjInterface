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
  slider.setValue(val->getVal());
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
  getRig()->getDevice(_id)->setParam(_param, (float)newValue);
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_ATTR, true);
}

double FloatPropertySlider::getValue() const
{
  LumiverseFloat* val = getRig()->getDevice(_id)->getParam<LumiverseFloat>(_param);
  return val->getVal();
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
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_ATTR, true);
}

double OrientationPropertySlider::getValue() const
{
  LumiverseOrientation* val = getRig()->getDevice(_id)->getParam<LumiverseOrientation>(_param);
  return val->getVal();
}

void OrientationPropertySlider::sliderDragStarted(Slider * s)
{
  stringstream ss;
  ss << _id << ":" << _param << " change start at " << getValue();
  getRecorder()->log(ACTION, ss.str());
}

void OrientationPropertySlider::sliderDragEnded(Slider * s)
{
  stringstream ss;
  ss << _id << ":" << _param << " value changed to " << getValue();
  getStatusBar()->setStatusMessage(ss.str());
  getRecorder()->log(ACTION, ss.str());
}

ColorPropertyPicker::ColorPropertyPicker(string id, string param, LumiverseColor * val) :
  ButtonPropertyComponent("color", false), _id(id), _param(param), _val(val)
{
}

ColorPropertyPicker::~ColorPropertyPicker()
{
}

void ColorPropertyPicker::paint(Graphics & g)
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

void ColorPropertyPicker::mouseDown(const MouseEvent & event)
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

void ColorPropertyPicker::changeListenerCallback(ChangeBroadcaster * source)
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

    MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

    if (mc != nullptr) {
      mc->refreshParams();
      mc->refreshAttr();
    }
  }
}

void ColorPropertyPicker::buttonClicked()
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

String ColorPropertyPicker::getButtonText() const
{
  return "Pick Color";
}



//==============================================================================
ParamControls::ParamControls()
{
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.

  addAndMakeVisible(_properties);
}

ParamControls::~ParamControls()
{
}

void ParamControls::paint (Graphics& g)
{
  g.fillAll(Colour(0xff929292));
}

void ParamControls::resized()
{
  // This method is where you should set the bounds of any child
  // components that your component contains..
  _properties.setBounds(getLocalBounds());
}

void ParamControls::initProperties()
{
  _properties.clear();
  Rig* rig = getRig();

  auto devices = rig->getAllDevices();

  for (const auto d : devices.getDevices()) {
    Array<PropertyComponent*> components;

    components.add(new FloatPropertySlider(d->getId(), "intensity", (LumiverseFloat*)d->getParam("intensity")));
    components.add(new OrientationPropertySlider(d->getId(), "polar", (LumiverseOrientation*)d->getParam("polar")));
    components.add(new OrientationPropertySlider(d->getId(), "azimuth", (LumiverseOrientation*)d->getParam("azimuth")));
    //components.add(new ColorPropertySlider(d->getId(), "color", "Red", d->getColor()));
    //components.add(new ColorPropertySlider(d->getId(), "color", "Green", d->getColor()));
    //components.add(new ColorPropertySlider(d->getId(), "color", "Blue", d->getColor()));
    //components.add(new HSVColorPropertySlider(d->getId(), "color", "H", d->getColor()));
    //components.add(new HSVColorPropertySlider(d->getId(), "color", "S", d->getColor()));
    //components.add(new HSVColorPropertySlider(d->getId(), "color", "V", d->getColor()));
    components.add(new ColorPropertyPicker(d->getId(), "color", d->getParam<LumiverseColor>("color")));
    components.add(new FloatPropertySlider(d->getId(), "penumbraAngle", d->getParam<LumiverseFloat>("penumbraAngle")));

    
    // these are really internal components and probably shouldn't be user editable
    //components.add(new FloatPropertySlider(d->getId(), "lookAtX", (LumiverseFloat*)d->getParam("lookAtX")));
    //components.add(new FloatPropertySlider(d->getId(), "lookAtY", (LumiverseFloat*)d->getParam("lookAtY")));
    //components.add(new FloatPropertySlider(d->getId(), "lookAtZ", (LumiverseFloat*)d->getParam("lookAtZ")));

    _properties.addSection(d->getId(), components);
  }
}

void ParamControls::refreshParams() {
  _properties.refreshAll();
}