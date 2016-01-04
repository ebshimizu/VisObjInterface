/*
  ==============================================================================

    AttributeControls.cpp
    Created: 15 Dec 2015 5:07:22pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControls.h"

#include "TestAttribute.h"


AttributeControlsList::AttributeControlsList()
{

}

AttributeControlsList::~AttributeControlsList()
{
  for (const auto kvp : _controls)
  {
    // it would be weird if the component were null but just in case
    if (kvp.second != nullptr)
      delete kvp.second;
  }
}

void AttributeControlsList::setWidth(int width)
{
  _width = width;
  setBounds(0, 0, _width, _height);
}

void AttributeControlsList::paint(Graphics& g)
{
  g.fillAll(Colour(0xff333333));
}

void AttributeControlsList::resized() {
  int numComponents = _controls.size();
  if (numComponents == 0)
    return;

  int height = getBounds().getHeight() / numComponents;
  auto bounds = getBounds();

  for (const auto &kvp : _controls) {
    kvp.second->setBounds(bounds.removeFromTop(height));
  }
}

void AttributeControlsList::addAttributeController(AttributeControllerBase * control)
{
  string name = control->getName().toStdString();

  if (_controls.count(name) > 0)
    delete _controls[name];

  addAndMakeVisible(control);
  _controls[name] = control;

  _height = _controls.size() * _componentHeight;
  setBounds(0, 0, _width, _height);
}

void AttributeControlsList::removeAttributeController(string name)
{
  if (_controls.count(name) > 0)
    delete _controls[name];

  _controls.erase(name);
  _height = _controls.size() * _componentHeight;
  setBounds(0, 0, _width, _height);
}

//==============================================================================
AttributeControls::AttributeControls()
{
  _container = new AttributeControlsList();
  _container->addAttributeController(new TestAttribute());
  _container->setName("attribute list");
  addAndMakeVisible(_container);

  _componentView = new Viewport();
  _componentView->setViewedComponent(_container);
  addAndMakeVisible(_componentView);
}

AttributeControls::~AttributeControls()
{
}

void AttributeControls::paint (Graphics& g)
{
  g.fillAll(Colour(0xff333333));
}

void AttributeControls::resized()
{
  auto bounds = getLocalBounds();

  _container->setWidth(_componentView->getMaximumVisibleWidth());
  _componentView->setBounds(bounds);
}
