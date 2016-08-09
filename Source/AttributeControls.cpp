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

DeviceSelector::DeviceSelector(vector<string> initialSelectedIds, function<void(vector<string>)> update) :
  _updateFunc(update)
{
  auto deviceIds = getRig()->getAllDevices().getIds();

  for (auto& s : deviceIds) {
    _deviceIds.add(s);
  }

  _deviceIds.sort(false);

  addAndMakeVisible(_list);
  _list.setModel(this);
  _list.setMultipleSelectionEnabled(true);
  _list.setClickingTogglesRowSelection(true);

  // set selected rows
  for (auto s : initialSelectedIds) {
    _list.selectRow(_deviceIds.indexOf(String(s)), false, false);
  }
}

DeviceSelector::~DeviceSelector()
{
}

int DeviceSelector::getNumRows()
{
  return _deviceIds.size();
}

void DeviceSelector::paintListBoxItem(int rowNumber, Graphics & g, int width, int height, bool rowIsSelected)
{
  if (rowIsSelected)
    g.fillAll(Colours::lightblue);
  else
    g.fillAll(Colour(0xffa3a3a3));

  g.setColour(Colours::black);
  g.setFont(14);
  g.drawText(_deviceIds[rowNumber], 2, 0, width - 4, height, Justification::centredLeft, true);
}

void DeviceSelector::selectedRowsChanged(int /* lastRowSelected */)
{
  vector<string> selectedIds;
  int selectedRows = _list.getNumSelectedRows();

  for (int i = 0; i < selectedRows; i++) {
    selectedIds.push_back(_deviceIds[_list.getSelectedRow(i)].toStdString());
  }

  _updateFunc(selectedIds);
}

void DeviceSelector::resized()
{
  _list.setBounds(getLocalBounds());
}

int DeviceSelector::getListHeight()
{
  // returns the total height of the list
  return _list.getRowHeight() * getNumRows();
}

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
  int numComponents = (int)_controls.size();
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

  _height = (int) (_controls.size() * _componentHeight);
  setBounds(0, 0, _width, _height);
}

void AttributeControlsList::removeAttributeController(string name)
{
  if (_controls.count(name) > 0)
    delete _controls[name];

  _controls.erase(name);
  _height = (int)(_controls.size() * _componentHeight);
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

  _setKeyButton = new TextButton("Key Lights", "Sets Key Lights used for Clustering");
  _setKeyButton->addListener(this);
  addAndMakeVisible(_setKeyButton);

  _clusterButton = new TextButton("Cluster", "Clusters the currently returned search results");
  _clusterButton->addListener(this);
  addAndMakeVisible(_clusterButton);

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
  delete _setKeyButton;
  delete _clusterButton;
}

void AttributeControls::paint (Graphics& g)
{
  g.fillAll(Colour(0xff333333));
  auto lbounds = getLocalBounds();
  lbounds.removeFromBottom(30);
  auto botRow2 = lbounds.removeFromBottom(30);

  g.setColour(Colours::white);
  g.drawFittedText("Sort Mode: ", botRow2.removeFromLeft(80).reduced(5), Justification::right, 1);
}

void AttributeControls::resized()
{
  auto lbounds = getLocalBounds();

  auto botBounds = lbounds.removeFromBottom(30);
  _search->setBounds(botBounds.removeFromRight(80).reduced(5));
  _sortButton->setBounds(botBounds.removeFromRight(80).reduced(5));
  _clusterButton->setBounds(botBounds.removeFromRight(80).reduced(5));
  _setKeyButton->setBounds(botBounds.removeFromRight(80).reduced(5));

  auto botRow2 = lbounds.removeFromBottom(30);
  botRow2.removeFromLeft(80);
  _sort->setBounds(botRow2.reduced(5));

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
  else if (b->getName() == "Key Lights") {
    function<void(vector<string>)> update = [](vector<string> selected) {
      getGlobalSettings()->_keyIds = selected;
    };

    Viewport* vp = new Viewport();
    DeviceSelector* cds = new DeviceSelector(getGlobalSettings()->_keyIds, update);
    cds->setSize(200, min(cds->getListHeight(), 300));
    vp->setViewedComponent(cds, true);
    vp->setSize(cds->getWidth(), cds->getHeight());

    CallOutBox::launchAsynchronously(vp, _setKeyButton->getScreenBounds(), nullptr);
  }
  else if (b->getName() == "Cluster") {
    getApplicationCommandManager()->invokeDirectly(RECLUSTER, true);
  }
}

void AttributeControls::comboBoxChanged(ComboBox * /* b */)
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
  _container->addAttributeController(new SaturationAttribute(50));

  // Tint
  _container->addAttributeController(new TintAttribute());

  // noire
  _container->addAttributeController(new NoireAttribute());

  // Histogram brightness
  _container->addAttributeController(new HistogramBrightness("Brightness", 50));

  // Histogram contrast
  _container->addAttributeController(new HistogramContrast("Contrast", 255));

  // Orange Blue
  _container->addAttributeController(new OrangeBlueAttribute());

  // moonlight
  _container->addAttributeController(new MoonlightAttribute());

  // Image similarity
  // load from local folder
  File imageDir = File::getCurrentWorkingDirectory().getChildFile("image_attributes");
  Array<File> imagesToLoad;
  int numImage = imageDir.findChildFiles(imagesToLoad, 2, false, "*.png");

  //LabxyHistogram gen(5, 5, 5, 3, 3, { 0, 100, -70, 70, -70, 70, 0, 1, 0, 1 }, 100);
  //getGlobalSettings()->_metric = gen.getGroundDistances();

  for (int i = 0; i < numImage; i++) {
    String name = imagesToLoad[i].getFileNameWithoutExtension();
    _container->addAttributeController(new ImageAttribute(name.toStdString(), imagesToLoad[i].getFullPathName().toStdString(), 5));
  }

  //_container->addAttributeController(new BacklitAttribute());
  //_container->addAttributeController(new SoftAttribute());
  //_container->addAttributeController(new HighAngleAttribute());
  //_container->addAttributeController(new SVRAttribute("C:/Users/falindrith/Documents/GitHub/pairwise-collector/server/romantic_p2g.svm", "Romantic"));
}
