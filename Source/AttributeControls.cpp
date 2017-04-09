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

LoggingTabbedComponent::LoggingTabbedComponent(TabbedButtonBar::Orientation orientation) :
  TabbedComponent(orientation)
{
}

void LoggingTabbedComponent::currentTabChanged(int newCurrentTabIndex, const String & newCurrentTabName)
{
  // inject log message about which tab is visible now
  getRecorder()->log(ACTION, "Current visible tab: " + newCurrentTabName.toStdString());

  TabbedComponent::currentTabChanged(newCurrentTabIndex, newCurrentTabName);
}


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
  _paramControls = new ParamControls();
  _history = new HistoryPanel();
  _historyViewer = new Viewport();
  _historyViewer->setViewedComponent(_history, true);

  _settings = new SettingsEditor();

  _vr = new PaletteControls();
  _ic = new IdeaControls();

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
  _sort->setSelectedId(0);
  addAndMakeVisible(_sort);

  // tab setup
  addAndMakeVisible(_tabs);
  //_tabs.addTab("Attributes", Colour(0xff333333), _componentView, true);
  _tabs.addTab("Lights", Colour(0xff333333), _paramControls, false);
  _tabs.addTab("Concepts", Colour(0xff333333), _ic, false);
  _tabs.addTab("Visual Research", Colour(0xff333333), _vr, false);
  _tabs.addTab("History", Colour(0xff333333), _historyViewer, false);
  _tabs.addTab("Settings", Colour(0xff333333), _settings, false);
  _tabs.setCurrentTabIndex(0);

  _reset.addListener(this);
  _reset.setButtonText("Reset");
  _reset.setName("reset");
  addAndMakeVisible(_reset);

  initPallets();
}

AttributeControls::~AttributeControls()
{
  delete _search;
  delete _sort;
  delete _sortButton;
  delete _setKeyButton;
  delete _clusterButton;
  delete _ic;
  delete _vr;

  delete _paramControls;
  delete _historyViewer;
  delete _settings;
}

void AttributeControls::paint (Graphics& g)
{
  g.fillAll(Colour(0xff333333));
  //auto lbounds = getLocalBounds();
  //lbounds.removeFromBottom(30);
  //auto botRow2 = lbounds.removeFromBottom(30);

  //g.setColour(Colours::white);
  //g.drawFittedText("Sort Mode: ", botRow2.removeFromLeft(80).reduced(5), Justification::right, 1);
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
  _history->setWidth(_historyViewer->getMaximumVisibleWidth());

}

void AttributeControls::refresh()
{
  _paramControls->refreshParams();
}

void AttributeControls::reload()
{
  // Delete everything and reload attributes
  initPallets();
}

void AttributeControls::buttonClicked(Button * b)
{
  if (b->getName() == "Search") {
    // perform a search action
    getApplicationCommandManager()->invokeDirectly(SEARCH, true);
  }
  else if (b->getName() == "Sort") {
    String id = _sort->getItemText(_sort->getSelectedItemIndex());
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
  return map<string, AttributeControllerBase*>();
}

void AttributeControls::deleteAllAttributes()
{
}

void AttributeControls::addAttributeController(AttributeControllerBase * controller)
{
}

void AttributeControls::lockAttributeModes()
{
}

void AttributeControls::unlockAttributeModes()
{
}

void AttributeControls::initPallets()
{
  _vr->_palettes->clearPallets();
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

      _vr->_palettes->addPallet(new GibbsPalette(name, originalImg));
      
      getRecorder()->log(SYSTEM, "Loaded image " + name.toStdString());
    }
    else {
      getRecorder()->log(SYSTEM, "Failed to load image " + name.toStdString());
    }
  }

  resized();
}

GibbsSchedule* AttributeControls::getGibbsSchedule()
{
  GibbsSchedule* sched = new GibbsSchedule();
  
  // gather pinned devices
  DeviceSet pins;
  set<string> ip, cp, pp;
  map<string, vector<string>> angles; // this is discarded for now

  // gather regions of pinned devices
  for (auto p : getGlobalSettings()->_pinnedRegions) {
    pins = pins.add(computeAffectedDevices(p, angles));
  }

  for (auto id : pins.getIds()) {
    ip.insert(id);
    cp.insert(id);
    pp.insert(id);
  }

  // manual pins
  for (auto id : getRig()->getAllDevices().getIds()) {
    if (isDeviceParamLocked(id, "intensity"))
      ip.insert(id);

    if (isDeviceParamLocked(id, "color"))
      cp.insert(id);

    if (isDeviceParamLocked(id, "pan"))
      pp.insert(id);
  }

  // gather all the samplers from the ideas
  for (auto i : _ic->_ideas->getIdeas()) {
    // focus palettes
    map<string, vector<string>> fp;

    // not every idea has a corresponding region on the stage
    if (getGlobalSettings()->_ideaMap.count(i) == 0)
      continue;

    // determine affected devices
    // get selected region
    auto r = getGlobalSettings()->_ideaMap[i];

    // compute affected devices
    DeviceSet affected = computeAffectedDevices(i, fp);
    
    // don't do anything with things that have a zero-size selection,
    // can cause problems with evaluating scores n stuff
    if (affected.size() == 0)
      continue;

    // remove filtered devices
    auto filter = i->getFilter();
    auto devices = affected.getDevices();
    for (auto data : devices) {
      if (filter.count(data->getMetadata("system")) == 0) {
        affected = affected.remove(data->getId());
      }
    }

    // create a sampler
    // the only sampler in this system that actually cares about the moving light position
    // is the intensity sampler.
    if (i->getType() == COLOR_PALETTE) {
      ColorSampler* colorSampler = new ColorSampler(affected, r, ip, cp, i->getColors(), i->getWeights());
      colorSampler->_name = i->getName().toStdString();
      colorSampler->setColorHistogram(i->_color);
      sched->addSampler((Sampler*)colorSampler);
    }
    else if (i->getType() == THEATRICAL) {
      TheatricalSampler* theatricalSampler = new TheatricalSampler(affected, r, ip, cp, i->getColors(), i->getWeights());
      theatricalSampler->_name = i->getName().toStdString();
      theatricalSampler->setColorHistogram(i->_color);
      sched->addSampler((Sampler*)theatricalSampler);
    }
    else if (i->getType() == MONOCHROME) {
      auto target = i->getColors()[0];
      Colour c((float)target[0], (float)target[1], (float)target[2], 1.0f);
      MonochromeSampler* monochromeSampler = new MonochromeSampler(affected, r, ip, cp, c);
      monochromeSampler->_name = i->getName().toStdString();
      sched->addSampler((Sampler*)monochromeSampler);
    }
    else if (i->getType() == INTENS_DIST) {
      IntensitySampler* intensSampler = new IntensitySampler(affected, r, ip, cp, i->_k, i->_meanBright, i->_mean);
      intensSampler->setBrightnessHistogram(i->_brightness);
      intensSampler->_name = i->getName().toStdString();
      intensSampler->_concept = i->getImage();
      intensSampler->setFocusPalettes(fp, pp);
      
      sched->addSampler((Sampler*)intensSampler);
    }
  }

  // create the pin sampler
  // note that here the region size is irrelevant and that the pins DeviceSet is also mostly irrelevant
  if (!getGlobalSettings()->_iterativeSystemSelect) {
    Sampler* pinSampler = (Sampler*)(new PinSampler(pins, juce::Rectangle<float>(), ip, cp));
    pinSampler->_name = "Pinned";
    sched->addSampler(pinSampler);
  }

  // move intensity to top of stack
  sched->moveIntensityUp();

  getGlobalSettings()->_intensityPins = ip;
  getGlobalSettings()->_colorPins = cp;

  // log info about the entire sampler
  sched->log();

  return sched;
}

ParamControls * AttributeControls::getParamController()
{
  return _paramControls;
}

HistoryPanel* AttributeControls::getHistory()
{
  return _history;
}

void AttributeControls::setColors(vector<Eigen::VectorXd> colors, double intens, vector<float> weights)
{
}

void AttributeControls::refreshSettings()
{
  _settings->refresh();
}

void AttributeControls::addIdea(Image i, String name, IdeaType type)
{
  _ic->_ideas->addIdea(i, name, type);
  _ic->_ideas->resized();
}

void AttributeControls::saveIdeas(File destFolder)
{
  _ic->_ideas->saveIdeas(destFolder);
}

void AttributeControls::loadIdeas(File srcFolder)
{
  _ic->_ideas->loadIdeas(srcFolder);
  _ic->_ideas->resized();
}

void AttributeControls::deleteIdeas()
{
  _ic->_ideas->deleteAllIdeas();
}

void AttributeControls::updateSortMenu()
{
  // each idea generates two different scores: a masked
  // and unmasked version. The sort keys are the names of the
  // ideas
  _sort->clear(dontSendNotification);
  auto ideas = _ic->_ideas->getIdeas();

  _sort->addItem("Sample ID", 1);
  for (int i = 1; i < ideas.size() + 1; i++)  {
    _sort->addItem(ideas[i - 1]->getName(), i * 2 + 1);
    _sort->addItem(ideas[i - 1]->getName() + " - foreground", i * 2 + 2);
  }

  _sort->setSelectedId(1);
}

void AttributeControls::initAttributes()
{
}

DeviceSet AttributeControls::computeAffectedDevices(juce::Rectangle<float> region, double threshold)
{
  // for each device, check if the cropped sensitivity image is above a threshold
  DeviceSet affected(getRig());

  int width = getGlobalSettings()->_sensitivityCache.begin()->second.i100.getWidth();
  int height = getGlobalSettings()->_sensitivityCache.begin()->second.i100.getHeight();

  juce::Rectangle<int> scaledRegion = juce::Rectangle<int>((int)(region.getX() * width), (int)(region.getY() * height),
    (int)(region.getWidth() * width), (int)(region.getHeight() * height));

  for (auto id : getRig()->getAllDevices().getIds()) {
    // compute sensitivity from cache cropped from bounding box
    sensCache cache = getGlobalSettings()->_sensitivityCache[id];
    Image i100Crop = cache.i100.getClippedImage(scaledRegion);

    // iterate through, count number of pixels that are at or above the 50th percentile
    int numBright = 0;

    int num85 = 0;
    int num95 = 0;

    for (int y = 0; y < i100Crop.getHeight(); y++) {
      for (int x = 0; x < i100Crop.getWidth(); x++) {
        if (i100Crop.getPixelAt(x, y).getBrightness() >= cache.avgVal) {
          numBright++;
        }
        if (i100Crop.getPixelAt(x, y).getBrightness() >= cache.data["85pct"]) {
          num85++;
        }
        if (i100Crop.getPixelAt(x, y).getBrightness() >= cache.data["95pct"]) {
          num95++;
        }
      }
    }

    // does the light cover the bbox?
    float coverageRatio = (float)(num85) / (i100Crop.getHeight() * i100Crop.getWidth());

    // is the light contained in the bbox?
    float contentsRatio = (float)(num85) / cache.data["85pct_ct"];

    // is there a particularly bright spot in the bbox?
    float highlight = num95 / cache.data["95pct_ct"];

    if (coverageRatio > 0.25 || contentsRatio > 0.50 || highlight > 0.05) {
      affected = affected.add(id);
    }
  }

  return affected;
}


DeviceSet AttributeControls::computeAffectedDevices(juce::Rectangle<float> region, map<string, vector<string>>& activeMLPalettes, double threshold)
{
  // for each device, check if the cropped sensitivity image is above a threshold
  DeviceSet affected(getRig());
  activeMLPalettes.clear();

  int width, height;
  if (getGlobalSettings()->_sensitivityCache.size() == 0) {
    width = getGlobalSettings()->_mlSensCache.begin()->second.begin()->second.i100.getWidth();
    height = getGlobalSettings()->_mlSensCache.begin()->second.begin()->second.i100.getHeight();
  }
  else {
    width = getGlobalSettings()->_sensitivityCache.begin()->second.i100.getWidth();
    height = getGlobalSettings()->_sensitivityCache.begin()->second.i100.getHeight();
  }

  juce::Rectangle<int> scaledRegion = juce::Rectangle<int>((int)(region.getX() * width), (int)(region.getY() * height),
    (int)(region.getWidth() * width), (int)(region.getHeight() * height));

  for (auto id : getRig()->getAllDevices().getIds()) {
    auto fp = getRig()->getDevice(id)->getFocusPaletteNames();
    if (fp.size() > 0) {
      // for focus palettes, we want to figure out which positions of the light can be active
      // in this next sampling step
      for (auto p : fp) {
        sensCache cache = getGlobalSettings()->_mlSensCache[id][p];
        Image i100Crop = cache.i100.getClippedImage(scaledRegion);

        // iterate through, count number of pixels that are at or above the 50th percentile
        int numBright = 0;

        int num85 = 0;
        int num95 = 0;

        for (int y = 0; y < i100Crop.getHeight(); y++) {
          for (int x = 0; x < i100Crop.getWidth(); x++) {
            if (i100Crop.getPixelAt(x, y).getBrightness() >= cache.avgVal) {
              numBright++;
            }
            if (i100Crop.getPixelAt(x, y).getBrightness() >= cache.data["85pct"]) {
              num85++;
            }
            if (i100Crop.getPixelAt(x, y).getBrightness() >= cache.data["95pct"]) {
              num95++;
            }
          }
        }

        // does the light cover the bbox?
        float coverageRatio = (float)(num85) / (i100Crop.getHeight() * i100Crop.getWidth());

        // is the light contained in the bbox?
        float contentsRatio = (float)(num85) / cache.data["85pct_ct"];

        // is there a particularly bright spot in the bbox?
        float highlight = num95 / cache.data["95pct_ct"];

        if (coverageRatio > 0.25 || contentsRatio > 0.50 || highlight > 0.05) {
          affected = affected.add(id);
          activeMLPalettes[id].push_back(p);
        }
      }
    }
    else {
      // compute sensitivity from cache cropped from bounding box
      sensCache cache = getGlobalSettings()->_sensitivityCache[id];
      Image i100Crop = cache.i100.getClippedImage(scaledRegion);

      // iterate through, count number of pixels that are at or above the 50th percentile
      int numBright = 0;

      int num85 = 0;
      int num95 = 0;

      for (int y = 0; y < i100Crop.getHeight(); y++) {
        for (int x = 0; x < i100Crop.getWidth(); x++) {
          if (i100Crop.getPixelAt(x, y).getBrightness() >= cache.avgVal) {
            numBright++;
          }
          if (i100Crop.getPixelAt(x, y).getBrightness() >= cache.data["85pct"]) {
            num85++;
          }
          if (i100Crop.getPixelAt(x, y).getBrightness() >= cache.data["95pct"]) {
            num95++;
          }
        }
      }

      // does the light cover the bbox?
      float coverageRatio = (float)(num85) / (i100Crop.getHeight() * i100Crop.getWidth());

      // is the light contained in the bbox?
      float contentsRatio = (float)(num85) / cache.data["85pct_ct"];

      // is there a particularly bright spot in the bbox?
      float highlight = num95 / cache.data["95pct_ct"];

      if (coverageRatio > 0.25 || contentsRatio > 0.50 || highlight > 0.05) {
        affected = affected.add(id);
      }
    }
  }

  return affected;
}
DeviceSet AttributeControls::computeAffectedDevices(juce::Rectangle<float> region, map<string, double>& debugInfo, double threshold)
{
  // for each device, check if the cropped sensitivity image is above a threshold
  DeviceSet affected(getRig());

  int width = getGlobalSettings()->_sensitivityCache.begin()->second.i100.getWidth();
  int height = getGlobalSettings()->_sensitivityCache.begin()->second.i100.getHeight();

  juce::Rectangle<int> scaledRegion = juce::Rectangle<int>((int)(region.getX() * width), (int)(region.getY() * height),
    (int)(region.getWidth() * width), (int)(region.getHeight() * height));

  for (auto id : getRig()->getAllDevices().getIds()) {
    // compute sensitivity from cache cropped from bounding box
    sensCache cache = getGlobalSettings()->_sensitivityCache[id];
    Image i100Crop = cache.i100.getClippedImage(scaledRegion);

    // iterate through, count number of pixels that are at or above the 50th percentile
    int numBright = 0;

    int num85 = 0;
    int num95 = 0;

    for (int y = 0; y < i100Crop.getHeight(); y++) {
      for (int x = 0; x < i100Crop.getWidth(); x++) {
        if (i100Crop.getPixelAt(x, y).getBrightness() >= cache.avgVal) {
          numBright++;
        }
        if (i100Crop.getPixelAt(x, y).getBrightness() >= cache.data["85pct"]) {
          num85++;
        }
        if (i100Crop.getPixelAt(x, y).getBrightness() >= cache.data["95pct"]) {
          num95++;
        }
      }
    }

    // does the light cover the bbox?
    float coverageRatio = (float)(num85) / (i100Crop.getHeight() * i100Crop.getWidth());

    // is the light contained in the bbox?
    float contentsRatio = (float)(num85) / cache.data["85pct_ct"];

    // is there a particularly bright spot in the bbox?
    float highlight = num95 / cache.data["95pct_ct"];

    debugInfo[id + " coverage"] = coverageRatio;
    debugInfo[id + " contents"] = contentsRatio;
    debugInfo[id + " highlight"] = highlight;

    if (coverageRatio > 0.25 || contentsRatio > 0.50 || highlight > 0.05) {
      affected = affected.add(id);
    }
  }

  return affected;
}


DeviceSet AttributeControls::computeAffectedDevices(shared_ptr<Idea> idea, double threshold)
{
  return computeAffectedDevices(getGlobalSettings()->_ideaMap[idea], threshold);
}

DeviceSet AttributeControls::computeAffectedDevices(shared_ptr<Idea> idea, map<string, vector<string>>& activeMLPalettes, double threshold)
{
  return computeAffectedDevices(getGlobalSettings()->_ideaMap[idea], activeMLPalettes, threshold);
}

void AttributeControls::toggleOldInterface()
{
  _tabs.clearTabs();
  _tabs.addTab("Lights", Colour(0xff333333), _paramControls, false);
  _tabs.addTab("History", Colour(0xff333333), _historyViewer, false);
  _tabs.setCurrentTabIndex(0);
}

void AttributeControls::toggleNewInterface()
{
  _tabs.clearTabs();
  _tabs.addTab("Concepts", Colour(0xff333333), _ic, false);
  _tabs.addTab("Visual Research", Colour(0xff333333), _vr, false);
  _tabs.addTab("History", Colour(0xff333333), _historyViewer, false);
  _tabs.setCurrentTabIndex(0);
}

void AttributeControls::toggleAllInterface()
{
  _tabs.clearTabs();
  _tabs.addTab("Lights", Colour(0xff333333), _paramControls, false);
  _tabs.addTab("Concepts", Colour(0xff333333), _ic, false);
  _tabs.addTab("Visual Research", Colour(0xff333333), _vr, false);
  _tabs.addTab("History", Colour(0xff333333), _historyViewer, false);
  _tabs.addTab("Settings", Colour(0xff333333), _settings, false);
  _tabs.setCurrentTabIndex(0);
}

void AttributeControls::filterPinnedDevices(DeviceSet& target, set<string> intens, set<string> color)
{
  for (auto id : intens) {
    target = target.remove(id);
  }

  for (auto id : color) {
    target = target.remove(id);
  }
}

AttributeControls::PaletteControls::PaletteControls()
{
  _palettes = new GibbsPalletContainer();
  _palettes->setName("available palettes");
  //addAndMakeVisible(_pallets);

  _paletteViewer = new Viewport();
  _paletteViewer->setViewedComponent(_palettes, true);
  addAndMakeVisible(_paletteViewer);
}

AttributeControls::PaletteControls::~PaletteControls()
{
  delete _paletteViewer;
}

void AttributeControls::PaletteControls::resized()
{
  auto lbounds = getLocalBounds();

  _paletteViewer->setBounds(lbounds);
  _palettes->setSize(_paletteViewer->getMaximumVisibleWidth() - _paletteViewer->getScrollBarThickness(), 0);
}

AttributeControls::IdeaControls::IdeaControls()
{
  _ideas = new IdeaList();
  _ideas->setName("ideas");
  //addAndMakeVisible(_pallets);

  _ideaViewer = new Viewport();
  _ideaViewer->setViewedComponent(_ideas, true);
  addAndMakeVisible(_ideaViewer);
}

AttributeControls::IdeaControls::~IdeaControls()
{
  delete _ideaViewer;
}

void AttributeControls::IdeaControls::resized()
{
  auto lbounds = getLocalBounds();

  _ideaViewer->setBounds(lbounds);
  _ideas->setSize(_ideaViewer->getMaximumVisibleWidth() - _ideaViewer->getScrollBarThickness(), 0);
}

