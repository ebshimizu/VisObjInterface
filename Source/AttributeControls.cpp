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

  _clusters = new Slider(Slider::IncDecButtons, Slider::TextEntryBoxPosition::TextBoxLeft);
  _clusters->addListener(this);
  _clusters->setRange(1, 100, 1);
  _clusters->setValue(getGlobalSettings()->_numDisplayClusters, dontSendNotification);
  _clusters->setName("clusters");
  addAndMakeVisible(_clusters);

  // Add the sort methods to the combo box
  _sort = new ComboBox("sort mode");
  _sort->addListener(this);
  _sort->setEditableText(false);
  _sort->addItem("Attribute Default", 1);
  _sort->addItem("Average Hue", 2);
  _sort->addItem("Key Hue", 3);
  _sort->addItem("Average Intensity", 4);
  _sort->addItem("Key Intensity", 5);
  _sort->addItem("Key Azimuth Angle", 6);
  _sort->setSelectedId(1);
  addAndMakeVisible(_sort);
}

AttributeControls::~AttributeControls()
{
  delete _container;
  _componentView->setViewedComponent(nullptr);
  delete _componentView;
  delete _search;
  delete _clusters;
  delete _sort;
}

void AttributeControls::paint (Graphics& g)
{
  g.fillAll(Colour(0xff333333));
}

void AttributeControls::resized()
{
  auto lbounds = getLocalBounds();

  auto botBounds = lbounds.removeFromBottom(30);
  _search->setBounds(botBounds.removeFromRight(80).reduced(5));
  _clusters->setBounds(botBounds.removeFromRight(140).reduced(5));
  _sort->setBounds(botBounds.reduced(5));

  _componentView->setBounds(lbounds);
  _container->setWidth(_componentView->getMaximumVisibleWidth());
}

void AttributeControls::refresh()
{
  _clusters->setValue(getGlobalSettings()->_numDisplayClusters, dontSendNotification);
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
}

void AttributeControls::sliderValueChanged(Slider * slider)
{
  if (slider->getName() == "clusters") {
    int newVal = (int)slider->getValue();
    if (newVal != getGlobalSettings()->_numDisplayClusters) {
      getGlobalSettings()->_numDisplayClusters = newVal;
      getApplicationCommandManager()->invokeDirectly(command::RECLUSTER, true);
    }
  }
}

void AttributeControls::comboBoxChanged(ComboBox * b)
{
  String id = b->getItemText(b->getSelectedId() - 1);
  getGlobalSettings()->_currentSortMode = id.toStdString();

  // do the re-sort
  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

  if (mc != nullptr) {
    mc->sortCluster();
  }
}

map<string, AttributeControllerBase*> AttributeControls::getActiveAttributes()
{
  return _container->getActiveAttribues();
}

void AttributeControls::initAttributes()
{
  _container->addAttributeController(new TestAttribute());
  _container->addAttributeController(new BrightAttribute());

  // add the contrast attributes
  auto& areas = getRig()->getMetadataValues("area");
  // however, skip them if there's just 1 area. contrast within one area
  // is a different attribute
  if (areas.size() > 1) {
    // for convenience we'll stick these keys in a vector
    vector<string> areaStrings;
    for (auto s : areas) {
      areaStrings.push_back(s);
    }

    for (int i = 0; i < areaStrings.size(); i++) {
      for (int j = i + 1; j < areaStrings.size(); j++) {
        _container->addAttributeController(new ContrastAttribute(areaStrings[i], areaStrings[j]));
      }
    }
  }

  //_container->addAttributeController(new BacklitAttribute());
  //_container->addAttributeController(new SoftAttribute());
  //_container->addAttributeController(new HighAngleAttribute());
  //_container->addAttributeController(new SVRAttribute("C:/Users/falindrith/Documents/GitHub/pairwise-collector/server/romantic_p2g.svm", "Romantic"));

}
