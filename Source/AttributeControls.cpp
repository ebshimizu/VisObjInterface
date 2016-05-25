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
#include "MainComponent.h"

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

void AttributeControlsList::removeAllControllers()
{
  for (auto& c : _controls) {
    delete c.second;
  }

  _controls.clear();
  _height = 0;
  setBounds(0, 0, _width, _height);
}

void AttributeControlsList::runPreprocess()
{
  for (auto& a : _controls) {
    a.second->preProcess();
  }
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
  initAttributes();
  _container->setName("attribute list");
  addAndMakeVisible(_container);

  _componentView = new Viewport();
  _componentView->setViewedComponent(_container);
  addAndMakeVisible(_componentView);

  _search = new TextButton("Search", "Perform a search with the current attribute constraints");
  _search->addListener(this);
  addAndMakeVisible(_search);

  _sortButton = new TextButton("Sort", "Sort the search results according to the selected sort method");
  _sortButton->addListener(this);
  addAndMakeVisible(_sortButton);

  _cleanUpButton = new TextButton("Clean Up", "Remove duplicate results and resume search");
  _cleanUpButton->addListener(this);
  addAndMakeVisible(_cleanUpButton);

  _cleanUpScenes = new Slider(Slider::SliderStyle::IncDecButtons, Slider::TextEntryBoxPosition::TextBoxLeft);
  _cleanUpScenes->setName("Clean Up Scenes");
  _cleanUpScenes->setRange(1, 500, 1);
  _cleanUpScenes->setValue(10, dontSendNotification);
  addAndMakeVisible(_cleanUpScenes);

  // Add the sort methods to the combo box
  _sort = new ComboBox("sort mode");
  _sort->addListener(this);
  _sort->setEditableText(false);
  _sort->addItem("Attribute Default", 1);
  _sort->addItem("Average Hue", 2);
  //_sort->addItem("Key Hue", 3);
  _sort->addItem("Average Intensity", 3);
  //_sort->addItem("Key Intensity", 5);
  //_sort->addItem("Key Azimuth Angle", 6);
  _sort->setSelectedId(1);
  addAndMakeVisible(_sort);
}

AttributeControls::~AttributeControls()
{
  delete _container;
  _componentView->setViewedComponent(nullptr);
  delete _componentView;
  delete _search;
  delete _sort;
  delete _sortButton;
  delete _cleanUpButton;
  delete _cleanUpScenes;
}

void AttributeControls::paint (Graphics& g)
{
  g.fillAll(Colour(0xff333333));
  auto lbounds = getLocalBounds();
  lbounds.removeFromBottom(30);
  auto botRow2 = lbounds.removeFromBottom(30);
  botRow2.removeFromRight(150);

  g.setColour(Colours::white);
  g.drawFittedText("Scenes After Clean Up", botRow2.reduced(5), Justification::centredRight, 1);
}

void AttributeControls::resized()
{
  auto lbounds = getLocalBounds();

  auto botBounds = lbounds.removeFromBottom(30);
  _search->setBounds(botBounds.removeFromRight(80).reduced(5));
  _sortButton->setBounds(botBounds.removeFromRight(80).reduced(5));
  _cleanUpButton->setBounds(botBounds.removeFromRight(80).reduced(5));
  _sort->setBounds(botBounds.reduced(5));

  auto botRow2 = lbounds.removeFromBottom(30);
  _cleanUpScenes->setBounds(botRow2.removeFromRight(150).reduced(5));

  _componentView->setBounds(lbounds);
  _container->setWidth(_componentView->getMaximumVisibleWidth());
}

void AttributeControls::refresh()
{
}

void AttributeControls::reload()
{
  // Delete everything and reload attributes
  _container->removeAllControllers();
  initAttributes();

  _container->runPreprocess();
}

void AttributeControls::buttonClicked(Button * b)
{
  if (b->getName() == "Search") {
    // perform a search action
    getApplicationCommandManager()->invokeDirectly(SEARCH, true);
  }
  else if (b->getName() == "Sort") {
    String id = _sort->getItemText(_sort->getSelectedId() - 1);
    getGlobalSettings()->_currentSortMode = id.toStdString();

    // do the re-sort
    MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

    if (mc != nullptr) {
      mc->sortCluster();
    }
  }
  else if (b->getName() == "Clean Up") {
    int numResults = (int)_cleanUpScenes->getValue();

    // do the clean up
    MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

    if (mc != nullptr) {
      mc->cleanUpResults(numResults);
    }
  }
}

void AttributeControls::comboBoxChanged(ComboBox * b)
{
  // actions only taken when button is pressed
}

map<string, AttributeControllerBase*> AttributeControls::getActiveAttributes()
{
  return _container->getActiveAttribues();
}

void AttributeControls::initAttributes()
{
  // Add saturation
  _container->addAttributeController(new SaturationAttribute(50, 100, 100));

  // Tint
  //_container->addAttributeController(new TintAttribute());

  // noire
  _container->addAttributeController(new NoireAttribute());

  // Histogram brightness
  _container->addAttributeController(new HistogramBrightness("Brightness - Hist", 50, 100, 100));

  // Histogram contrast
  _container->addAttributeController(new HistogramContrast("Contrast - Hist", 255, 100, 100));

  // Orange Blue
  _container->addAttributeController(new OrangeBlueAttribute(100, 100));

  // moonlight
  _container->addAttributeController(new MoonlightAttribute(100, 100));

  //_container->addAttributeController(new BacklitAttribute());
  //_container->addAttributeController(new SoftAttribute());
  //_container->addAttributeController(new HighAngleAttribute());
  //_container->addAttributeController(new SVRAttribute("C:/Users/falindrith/Documents/GitHub/pairwise-collector/server/romantic_p2g.svm", "Romantic"));
}
