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

void FloatPropertySlider::setValue(double newValue)
{
  getRig()->getDevice(_id)->setParam(_param, (float)newValue);
}

double FloatPropertySlider::getValue() const
{
  LumiverseFloat* val = (LumiverseFloat*)getRig()->getDevice(_id)->getParam(_param);
  return val->getVal();
}

void FloatPropertySlider::sliderDragStarted(Slider * s)
{
  stringstream ss;
  ss << _id << ":" << _param << " change start at " << getValue();
  getRecorder()->log(ACTION, ss.str());
}

void FloatPropertySlider::sliderDragEnded(Slider * s)
{
  stringstream ss;
  ss << _id << ":" << _param << " value changed to " << getValue();
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

void OrientationPropertySlider::setValue(double newValue)
{
  getRig()->getDevice(_id)->setParam(_param, (float)newValue);
}

double OrientationPropertySlider::getValue() const
{
  LumiverseOrientation* val = (LumiverseOrientation*)getRig()->getDevice(_id)->getParam(_param);
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

void ColorPropertySlider::setValue(double newValue)
{
  getRig()->getDevice(_id)->setColorChannel(_param, _channel, newValue);
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_PARAMS, false);
}

double ColorPropertySlider::getValue() const
{
  LumiverseColor* c = getRig()->getDevice(_id)->getColor(_param);
  return c->getColorChannel(_channel);
}

void ColorPropertySlider::sliderDragStarted(Slider * s)
{
  stringstream ss;
  ss << _id << ":" << _param << "." << _channel << " change start at " << getValue();
  getRecorder()->log(ACTION, ss.str());
}

void ColorPropertySlider::sliderDragEnded(Slider * s)
{
  stringstream ss;
  ss << _id << ":" << _param << "." << _channel << " value changed to " << getValue();
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
  g.fillAll(Colour(0xff333333));
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
    components.add(new FloatPropertySlider(d->getId(), "distance", (LumiverseFloat*)d->getParam("distance")));
    components.add(new ColorPropertySlider(d->getId(), "color", "Red", d->getColor()));
    components.add(new ColorPropertySlider(d->getId(), "color", "Green", d->getColor()));
    components.add(new ColorPropertySlider(d->getId(), "color", "Blue", d->getColor()));
    components.add(new HSVColorPropertySlider(d->getId(), "color", "H", d->getColor()));
    components.add(new HSVColorPropertySlider(d->getId(), "color", "S", d->getColor()));
    components.add(new HSVColorPropertySlider(d->getId(), "color", "V", d->getColor()));

    
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