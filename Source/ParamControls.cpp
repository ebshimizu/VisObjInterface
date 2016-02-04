/*
  ==============================================================================

    ParamControls.cpp
    Created: 15 Dec 2015 5:07:12pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "ParamControls.h"
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

ColorPropertySlider::ColorPropertySlider(string id, string param, string channel, LumiverseColor * val) :
  SliderPropertyComponent(channel, 0, 1, 0.001), _id(id), _param(param), _channel(channel)
{
  slider.setValue(val->getColorChannel(_channel));
}

ColorPropertySlider::~ColorPropertySlider()
{
}

void ColorPropertySlider::paint(Graphics & g)
{
  LookAndFeel& lf = getLookAndFeel();

  if (isDeviceParamLocked(_id, _param + _channel)) {
    g.setColour(Colour(0xFFFF3838));
  }
  else {
    g.setColour(this->findColour(PropertyComponent::backgroundColourId));
  }

  g.fillRect(0, 0, getWidth(), getHeight() - 1);

  lf.drawPropertyComponentLabel(g, getWidth(), getHeight(), *this);
}

void ColorPropertySlider::mouseDown(const MouseEvent & event)
{
  if (event.mods.isRightButtonDown()) {
    if (isDeviceParamLocked(_id, _param + _channel)) {
      unlockDeviceParam(_id, _param + _channel);

      // if none of the other color params are locked, unlock HSV
      if (!isDeviceParamLocked(_id, _param + "Red") && !isDeviceParamLocked(_id, _param + "Green") && !isDeviceParamLocked(_id, _param + "Blue")) {
        unlockDeviceParam(_id, _param + "H");
        unlockDeviceParam(_id, _param + "S");
        unlockDeviceParam(_id, _param + "V");
      }
    }
    else {
      lockDeviceParam(_id, _param + _channel);
      // Lock HSV as well
      lockDeviceParam(_id, _param + "H");
      lockDeviceParam(_id, _param + "S");
      lockDeviceParam(_id, _param + "V");
    }

    getApplicationCommandManager()->invokeDirectly(command::REFRESH_PARAMS, false);
  }
}

void ColorPropertySlider::setValue(double newValue)
{
  getRig()->getDevice(_id)->setColorChannel(_param, _channel, newValue);
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_PARAMS, false);
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_ATTR, true);
}

double ColorPropertySlider::getValue() const
{
  LumiverseColor* c = getRig()->getDevice(_id)->getColor(_param);
  return c->getColorChannel(_channel);
}

void ColorPropertySlider::sliderDragStarted(Slider * /* s */)
{
  stringstream ss;
  ss << _id << ":" << _param << "." << _channel << " change start at " << getValue();
  getRecorder()->log(ACTION, ss.str());
}

void ColorPropertySlider::sliderDragEnded(Slider * /* s */)
{
  stringstream ss;
  ss << _id << ":" << _param << "." << _channel << " value changed to " << getValue();
  getStatusBar()->setStatusMessage(ss.str());
  getRecorder()->log(ACTION, ss.str());
}

HSVColorPropertySlider::HSVColorPropertySlider(string id, string param, string channel, LumiverseColor * val) :
  SliderPropertyComponent(channel, 0, (channel == "H") ? 360 : 1, 0.001), _id(id), _param(param), _channel(channel)
{
  auto hsv = val->getHSV();

  if (channel == "H")
    slider.setValue(hsv[0]);
  if (channel == "S")
    slider.setValue(hsv[1]);
  if (channel == "V")
    slider.setValue(hsv[2]);
}

HSVColorPropertySlider::~HSVColorPropertySlider()
{
}

void HSVColorPropertySlider::paint(Graphics & g)
{
  LookAndFeel& lf = getLookAndFeel();

  if (isDeviceParamLocked(_id, _param + _channel)) {
    g.setColour(Colour(0xFFFF3838));
  }
  else {
    g.setColour(this->findColour(PropertyComponent::backgroundColourId));
  }

  g.fillRect(0, 0, getWidth(), getHeight() - 1);

  lf.drawPropertyComponentLabel(g, getWidth(), getHeight(), *this);
}

void HSVColorPropertySlider::mouseDown(const MouseEvent & event)
{
  if (event.mods.isRightButtonDown()) {
    if (isDeviceParamLocked(_id, _param + _channel)) {
      unlockDeviceParam(_id, _param + _channel);
      // if none of the other hsv params are locked, unlock rgb
      if (!isDeviceParamLocked(_id, _param + "H") && !isDeviceParamLocked(_id, _param + "S") && !isDeviceParamLocked(_id, _param + "V")) {
        unlockDeviceParam(_id, _param + "Red");
        unlockDeviceParam(_id, _param + "Green");
        unlockDeviceParam(_id, _param + "Blue");
      }
    }
    else {
      lockDeviceParam(_id, _param + _channel);
      // Lock RGB as well
      lockDeviceParam(_id, _param + "Red");
      lockDeviceParam(_id, _param + "Green");
      lockDeviceParam(_id, _param + "Blue");
    }
    
    getApplicationCommandManager()->invokeDirectly(command::REFRESH_PARAMS, false);
  }
}

void HSVColorPropertySlider::setValue(double newValue)
{
  LumiverseColor* c = getRig()->getDevice(_id)->getColor(_param);
  auto hsv = c->getHSV();

  if (_channel == "H")
    hsv[0] = newValue;
  if (_channel == "S")
    hsv[1] = newValue;
  if (_channel == "V")
    hsv[2] = newValue;

  c->setHSV(hsv[0], hsv[1], hsv[2]);
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_PARAMS, false);
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_ATTR, true);
}

double HSVColorPropertySlider::getValue() const
{
  LumiverseColor* c = getRig()->getDevice(_id)->getColor(_param);
  auto hsv = c->getHSV();

  if (_channel == "H")
    return hsv[0];
  if (_channel == "S")
    return hsv[1];
  if (_channel == "V")
    return hsv[2];
}

void HSVColorPropertySlider::sliderDragStarted(Slider * s)
{
  stringstream ss;
  ss << _id << ":" << _param << "." << _channel << " change start at " << getValue();
  getRecorder()->log(ACTION, ss.str());
}

void HSVColorPropertySlider::sliderDragEnded(Slider * s)
{
  stringstream ss;
  ss << _id << ":" << _param << "." << _channel << " value changed to " << getValue();
  getStatusBar()->setStatusMessage(ss.str());
  getRecorder()->log(ACTION, ss.str());
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
    components.add(new ColorPropertySlider(d->getId(), "color", "Red", d->getColor()));
    components.add(new ColorPropertySlider(d->getId(), "color", "Green", d->getColor()));
    components.add(new ColorPropertySlider(d->getId(), "color", "Blue", d->getColor()));
    components.add(new HSVColorPropertySlider(d->getId(), "color", "H", d->getColor()));
    components.add(new HSVColorPropertySlider(d->getId(), "color", "S", d->getColor()));
    components.add(new HSVColorPropertySlider(d->getId(), "color", "V", d->getColor()));
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