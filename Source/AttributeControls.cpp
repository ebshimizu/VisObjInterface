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

void AttributeControlsList::lockImageAttrs()
{
  for (auto a : _controls) {
    if (ImageAttribute* ia = dynamic_cast<ImageAttribute*>(a.second)) {
      ia->lockMode();
    }
  }
}

void AttributeControlsList::unlockImageAttrs()
{
  for (auto a : _controls) {
    if (ImageAttribute* ia = dynamic_cast<ImageAttribute*>(a.second)) {
      ia->unlockMode();
    }
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
AttributeControls::AttributeControls() : _tabs(TabbedButtonBar::Orientation::TabsAtRight)
{
  // container init
  _container = new AttributeControlsList();
  _container->setName("attribute list");
  //addAndMakeVisible(_container);

  _paletteControls = new PaletteControls();
  _paletteControls->setName("Palette Controls");

  _componentView = new Viewport();
  _componentView->setViewedComponent(_container, true);
  //addAndMakeVisible(_componentView);

  _paramControls = new ParamControls();
  _history = new HistoryPanel();
  _historyViewer = new Viewport();
  _historyViewer->setViewedComponent(_history, true);

  _settings = new SettingsEditor();

  // button controls
  _search = new TextButton("Search", "Perform a search with the current attribute constraints");
  _search->addListener(this);
  addAndMakeVisible(_search);

  _sortButton = new TextButton("Sort", "Sort the search results according to the selected sort method");
  _sortButton->addListener(this);
  addAndMakeVisible(_sortButton);

  _setKeyButton = new TextButton("Key Lights", "Sets Key Lights used for Clustering");
  _setKeyButton->addListener(this);
  //addAndMakeVisible(_setKeyButton);

  _clusterButton = new TextButton("Cluster", "Clusters the currently returned search results");
  _clusterButton->addListener(this);
  //addAndMakeVisible(_clusterButton);

  // Add the sort methods to the combo box
  _sort = new ComboBox("sort mode");
  _sort->addListener(this);
  _sort->setEditableText(false);
  _sort->addItem("Attribute Default", 1);
  _sort->addItem("Average Hue", 2);
  //_sort->addItem("Key Hue", 3);
  _sort->addItem("Average Intensity", 3);
  _sort->addItem("Side Light Style", 4);
  //_sort->addItem("Key Intensity", 5);
  //_sort->addItem("Key Azimuth Angle", 6);
  _sort->setSelectedId(1);
  addAndMakeVisible(_sort);

  // tab setup
  addAndMakeVisible(_tabs);
  //_tabs.addTab("Attributes", Colour(0xff333333), _componentView, true);
  _tabs.addTab("Lights", Colour(0xff333333), _paramControls, true);
  _tabs.addTab("Palettes", Colour(0xff333333), _paletteControls, true);
  _tabs.addTab("History", Colour(0xff333333), _historyViewer, true);
  _tabs.addTab("Settings", Colour(0xff333333), _settings, true);
  _tabs.setCurrentTabIndex(0);

  _reset.addListener(this);
  _reset.setButtonText("Reset");
  _reset.setName("reset");
  addAndMakeVisible(_reset);

  initAttributes();
  initPallets();
}

AttributeControls::~AttributeControls()
{
  delete _componentView;
  delete _search;
  delete _sort;
  delete _sortButton;
  delete _setKeyButton;
  delete _clusterButton;
  delete _paletteControls;
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
  _reset.setBounds(botBounds.removeFromRight(80).reduced(5));
  //_clusterButton->setBounds(botBounds.removeFromRight(80).reduced(5));
  //_setKeyButton->setBounds(botBounds.removeFromRight(80).reduced(5));

  auto botRow2 = lbounds.removeFromBottom(30);
  botRow2.removeFromLeft(80);
  _sort->setBounds(botRow2.reduced(5));

  _tabs.setBounds(lbounds);
  _container->setWidth(_componentView->getMaximumVisibleWidth());
  _history->setWidth(_historyViewer->getMaximumVisibleWidth());
}

void AttributeControls::refresh()
{
}

void AttributeControls::reload()
{
  // Delete everything and reload attributes
  _container->removeAllControllers();
  initAttributes();
  initPallets();
  _paletteControls->_tempConstraints->updateBounds();

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
  else if (b->getName() == "reset") {
    getApplicationCommandManager()->invokeDirectly(RESET_ALL, false);
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

void AttributeControls::deleteAllAttributes()
{
  _container->removeAllControllers();
}

void AttributeControls::addAttributeController(AttributeControllerBase * controller)
{
  controller->preProcess();
  _container->addAttributeController(controller);
}

void AttributeControls::lockAttributeModes()
{
  _container->lockImageAttrs();
}

void AttributeControls::unlockAttributeModes()
{
  _container->unlockImageAttrs();
}

void AttributeControls::initPallets()
{
  _paletteControls->_palettes->clearPallets();
  File imageDir = getGlobalSettings()->_imageAttrLoc;
  Array<File> imagesToLoad;
  int numImage = imageDir.findChildFiles(imagesToLoad, 2, false, "*.png");

  for (int i = 0; i < numImage; i++) {
    String name = imagesToLoad[i].getFileNameWithoutExtension();
    String filepath = imagesToLoad[i].getFullPathName();

    File img(filepath);
    FileInputStream in(img);

    if (in.openedOk()) {
      // load image
      PNGImageFormat pngReader;
      Image originalImg = pngReader.decodeImage(in);

      _paletteControls->_palettes->addPallet(new GibbsPalette(name, originalImg));
      
      getRecorder()->log(SYSTEM, "Loaded image for pallet " + name.toStdString());
    }
    else {
      getRecorder()->log(SYSTEM, "Failed to load image for pallet " + name.toStdString());
    }
  }

  resized();
}

vector<pair<GibbsScheduleConstraint, GibbsSchedule*>> AttributeControls::getGibbsSchedule()
{
  auto colorConstraints = _paletteControls->_tempConstraints->getColorDists();
  auto intensConstraints = _paletteControls->_tempConstraints->getIntensDist();
  
  double avg, max;
  int k;

  GibbsScheduleConstraint ci;
  ci._followConventions = true;
  ci._param = GINTENSITY;
  ci._targets = getRig()->getAllDevices();

  GibbsScheduleConstraint cc;
  cc._param = GCOLOR;
  cc._followConventions = true;
  cc._targets = getRig()->getAllDevices();

  GibbsSchedule* sched = new GibbsSchedule();
  vector<normal_distribution<float>> idists;
  idists.push_back(intensConstraints);
  sched->setIntensDist(idists);
  _paletteControls->_tempConstraints->getIntensDistProps(sched->_avgIntens, sched->_maxIntens, sched->_numBrightLights);
  sched->_useSystems = _paletteControls->_tempConstraints->useSystems();

  sched->setColorDists(colorConstraints[0], colorConstraints[1], colorConstraints[2]);
  vector<pair<GibbsScheduleConstraint, GibbsSchedule*>> state;
  state.push_back(pair<GibbsScheduleConstraint, GibbsSchedule*>(ci, sched));
  state.push_back(pair<GibbsScheduleConstraint, GibbsSchedule*>(cc, sched));

  return state;
}

ParamControls * AttributeControls::getParamController()
{
  return _paramControls;
}

HistoryPanel* AttributeControls::getHistory()
{
  return _history;
}

void AttributeControls::setColors(vector<Eigen::VectorXd> colors, double intens)
{
  _paletteControls->_tempConstraints->addColors(colors, intens);
  _paletteControls->resized();
}

void AttributeControls::refreshSettings()
{
  _settings->refresh();
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
  //_container->addAttributeController(new MoonlightAttribute());

  // Image similarity
  // load from local folder
  File imageDir = getGlobalSettings()->_imageAttrLoc;
  Array<File> imagesToLoad;
  int numImage = imageDir.findChildFiles(imagesToLoad, 2, false, "*.png");

  //LabxyHistogram gen(5, 5, 5, 3, 3, { 0, 100, -70, 70, -70, 70, 0, 1, 0, 1 }, 100);
  //getGlobalSettings()->_metric = gen.getGroundDistances();

  // Directional test for diversity
  DirectionalTestAttribute* dt = new DirectionalTestAttribute("Directional", Image(), 0);
  _container->addAttributeController(dt);

  for (int i = 0; i < numImage; i++) {
    String name = imagesToLoad[i].getFileNameWithoutExtension();
    _container->addAttributeController(new ImageAttribute(name.toStdString(), imagesToLoad[i].getFullPathName().toStdString(), 50));

    // TEST
    ImageAttribute* mod = new ImageAttribute(name.toStdString() + "_directed", imagesToLoad[i].getFullPathName().toStdString(), 50);
    mod->setStyle(Style::DIRECTIONAL);
    _container->addAttributeController(mod);
  }

  getStatusBar()->setStatusMessage("Loaded " + String(numImage) + " images from " + imageDir.getFullPathName());
}

AttributeControls::PaletteControls::PaletteControls()
{
  _palettes = new GibbsPalletContainer();
  _palettes->setName("available palettes");
  //addAndMakeVisible(_pallets);

  _paletteViewer = new Viewport();
  _paletteViewer->setViewedComponent(_palettes, true);
  addAndMakeVisible(_paletteViewer);

  _tempConstraints = new GibbsConstraintContainer();
  addAndMakeVisible(_tempConstraints);
}

AttributeControls::PaletteControls::~PaletteControls()
{
  delete _paletteViewer;
  delete _tempConstraints;
}

void AttributeControls::PaletteControls::resized()
{
  auto lbounds = getLocalBounds();
  _tempConstraints->setSize(lbounds.getWidth(), lbounds.getHeight());
  _tempConstraints->setBounds(lbounds.removeFromTop(_tempConstraints->getHeight()));

  _paletteViewer->setBounds(lbounds);
  _palettes->setSize(_paletteViewer->getMaximumVisibleWidth(), 0);
}
