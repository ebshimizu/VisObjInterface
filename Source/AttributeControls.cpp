/*
  ==============================================================================

    AttributeControls.cpp
    Created: 15 Dec 2015 5:07:22pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControls.h"

#include "AttributeControllers.h"


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
  auto lbounds = getBounds();

  for (const auto &kvp : _controls) {
    kvp.second->setBounds(lbounds.removeFromTop(height));
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

map<string, AttributeControllerBase*> AttributeControlsList::getActiveAttribues()
{
  map<string, AttributeControllerBase*> active;

  for (const auto& kvp : _controls) {
    if (kvp.second->getStatus() != A_IGNORE) {
      active[kvp.first] = kvp.second;
    }
  }

  return active;
}

//==============================================================================
AttributeControls::AttributeControls()
{
  _container = new AttributeControlsList();
  _container->addAttributeController(new TestAttribute());
  _container->addAttributeController(new BrightAttribute());
  _container->addAttributeController(new BacklitAttribute());
  _container->addAttributeController(new SoftAttribute());
  _container->setName("attribute list");
  addAndMakeVisible(_container);

  _componentView = new Viewport();
  _componentView->setViewedComponent(_container);
  addAndMakeVisible(_componentView);

  _search = new TextButton("Search", "Perform a search with the current attribute constraints");
  _search->addListener(this);
  addAndMakeVisible(_search);
}

AttributeControls::~AttributeControls()
{
  delete _container;
  _componentView->setViewedComponent(nullptr);
  delete _componentView;
  delete _search;
}

void AttributeControls::paint (Graphics& g)
{
  g.fillAll(Colour(0xff333333));
}

void AttributeControls::resized()
{
  auto lbounds = getLocalBounds();

  _search->setBounds(lbounds.removeFromBottom(30).removeFromRight(150).reduced(5));

  _componentView->setBounds(lbounds);
  _container->setWidth(_componentView->getMaximumVisibleWidth());
}

void AttributeControls::buttonClicked(Button * b)
{
  if (b->getName() == "Search") {
    // perform a search action
    getApplicationCommandManager()->invokeDirectly(SEARCH, true);
  }
}

map<string, AttributeControllerBase*> AttributeControls::getActiveAttributes()
{
  return _container->getActiveAttribues();
}
