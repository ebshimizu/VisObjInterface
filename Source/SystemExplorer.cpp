/*
  ==============================================================================

    SystemExplorer.cpp
    Created: 11 Nov 2016 10:05:09am
    Author:  falindrith

  ==============================================================================
*/

#include "SystemExplorer.h"
#include "MainComponent.h"

//==============================================================================

SystemExplorer::SystemExplorer(string name, string system) : _name(name), _blackout("B"), _solo("S"), _pin("P")
{
  setName(name);
  _selected = getRig()->select("$system=" + system);

  init();
}

SystemExplorer::SystemExplorer(string name) : _name(name)
{
  setName(name);
  init();
}

SystemExplorer::SystemExplorer(Array<shared_ptr<SearchResultContainer>> results, string name)
{
  setName(name);
  init();

  for (auto r : results)
    addNewResult(r);
}

SystemExplorer::SystemExplorer(Array<shared_ptr<SearchResultContainer>> results, string name, DeviceSet selected) :
  _name(name)
{
  setName(name);
  _selected = selected;
  init();

  for (auto r : results)
    addNewResult(r);
}

SystemExplorer::~SystemExplorer()
{
  delete _rowElems;
  delete _currentState;
  delete _rigState;
  delete _temp;
}

void SystemExplorer::addNewResult(shared_ptr<SearchResultContainer> result)
{
  auto nr = shared_ptr<SearchResultContainer>(new SearchResultContainer(shared_ptr<SearchResult>(result->getSearchResult())));
  nr->setSystem(_selected);

  // check similarity before adding, if too similar, reject
  // we check similarity against L2 parameter norm filtered by active devices
  for (auto r : _container->_results) {
    if (filteredDist(r, nr) < _distThreshold)
      return;
  }

  _container->addAndMakeVisible(nr.get());
  updateSingleImage(nr);
  _container->_results.add(nr);
  resized();
}

void SystemExplorer::updateAllImages(Snapshot* rigState)
{
  delete _rigState;
  _rigState = new Snapshot(*rigState);

  long long recent = 0;
  SearchResultContainer* recentElem = nullptr;
  for (auto r : _container->_results) {
    updateSingleImage(r);
    r->setSystem(_selected);
    r->setMostRecent(false);

    if (r->wasSelected()) {
      if (r->getTime() > recent) {
        recent = r->getTime();
        recentElem = r.get();
      }
    }
  }

  if (recentElem != nullptr)
    recentElem->setMostRecent(true);

  // also update current state scene
  if (!_isBlackout) {
    // update current scene if we're not in blackout mode
    delete _currentState;
    _currentState = new Snapshot(getRig());
    auto data = _currentState->getRigData();

    // isolate selected devices
    for (auto d : _currentState->getDevices()) {
      if (!_selected.contains(d->getId())) {
        d->setIntensity(0);
      }
    }

    _currentImg = renderImage(_currentState, getGlobalSettings()->_renderWidth * getGlobalSettings()->_thumbnailRenderScale,
      getGlobalSettings()->_renderHeight * getGlobalSettings()->_thumbnailRenderScale);
  }
}

void SystemExplorer::paint(Graphics & g)
{
  g.setColour(Colours::white);
  auto lbounds = getLocalBounds();

  lbounds.removeFromLeft(24);
  auto left = lbounds.removeFromLeft(200);
  left.removeFromBottom(24);
  g.drawFittedText(getName() + ": " + _name, left.removeFromTop(24), Justification::centredLeft, 2);

  auto line = lbounds.removeFromLeft(5);
  g.setColour(Colours::white);
  line.removeFromLeft(2);
  line.removeFromRight(2);
  g.fillRect(line);

  g.drawImageWithin(_currentImg, left.getX(), left.getY(), left.getWidth(), left.getHeight(), RectanglePlacement::centred);
}

void SystemExplorer::resized()
{
  auto lbounds = getLocalBounds();

  auto buttons = lbounds.removeFromLeft(24);
  buttons.removeFromTop(24);
  _blackout.setBounds(buttons.removeFromTop(24).reduced(2));
  _solo.setBounds(buttons.removeFromTop(24).reduced(2));
  _pin.setBounds(buttons.removeFromTop(24).reduced(2));

  auto left = lbounds.removeFromLeft(200);

  _intens.setBounds(left.removeFromBottom(24));

  lbounds.removeFromLeft(5);
  _rowElems->setBounds(lbounds);
  _container->setSize(lbounds.getWidth(), _rowElems->getMaximumVisibleHeight());
}

DeviceSet SystemExplorer::getViewedDevices()
{
  return _selected;
}

void SystemExplorer::comboBoxChanged(ComboBox * c)
{
  // assumes for now that each element in the combo box is a system
  String system = c->getText();

  if (system == "")
    return;

  _selected = getRig()->select("$system=" + system.toStdString());

  //setName(system);
  updateAllImages(new Snapshot(getRig()));
}

void SystemExplorer::sort(string method)
{
  CacheSorter sorter = CacheSorter(method);

  _container->_results.sort(sorter);
  resized();
}

void SystemExplorer::init()
{
  // create viewport and container
  _rowElems = new Viewport();
  _container = new SystemExplorerResults();
  addAndMakeVisible(_container);
  _rowElems->setViewedComponent(_container, true);
  addAndMakeVisible(_rowElems);

  // create the image and current state
  _currentState = new Snapshot(getRig());
  _rigState = new Snapshot(*_currentState);
  auto data = _currentState->getRigData();

  // isolate selected devices
  float avgIntens = 0;
  for (auto d : _currentState->getDevices()) {
    if (!_selected.contains(d->getId())) {
      d->setIntensity(0);
    }
    else {
      avgIntens += d->getIntensity()->asPercent();
    }
  }

  _distThreshold = getGlobalSettings()->_jndThreshold;

  // set intensity slider
  _intens.setRange(0, 100, 0.01);
  _intens.setValue(100.0 * avgIntens / _selected.size(), dontSendNotification);
  _intens.addListener(this);
  _intens.setSliderStyle(Slider::SliderStyle::LinearBar);
  addAndMakeVisible(_intens);

  _currentImg = renderImage(_currentState, getGlobalSettings()->_renderWidth * getGlobalSettings()->_thumbnailRenderScale,
    getGlobalSettings()->_renderHeight * getGlobalSettings()->_thumbnailRenderScale);
  _isBlackout = false;
  _isSolo = false;
  _temp = new Snapshot(*_currentState);

  _pin.setButtonText("P");
  _solo.setButtonText("S");
  _blackout.setButtonText("B");
  _pin.setName("P");
  _solo.setName("S");
  _blackout.setName("B");

  _pin.setColour(TextButton::buttonOnColourId, Colours::darkred);
  _solo.setColour(TextButton::buttonOnColourId, Colours::darkgoldenrod);

  _pin.setToggleState(_isPinned, dontSendNotification);
  _solo.setToggleState(_isSolo, dontSendNotification);
  _blackout.setToggleState(_isBlackout, dontSendNotification);

  _pin.addListener(this);
  _solo.addListener(this);
  _blackout.addListener(this);
  addAndMakeVisible(_pin);
  addAndMakeVisible(_solo);
  addAndMakeVisible(_blackout);

  updatePinState();
}

void SystemExplorer::buttonClicked(Button * b)
{
  if (b->getName() == "B") {
    if (_isBlackout) {
      unBlackout();
    }
    else {
      blackout();
    }
  }
  else if (b->getName() == "S") {
    if (_isSolo) {
      unSolo();
    }
    else {
      solo();
    }
  }
  else if (b->getName() == "P") {
    togglePin();
  }
}

void SystemExplorer::blackout()
{
  _isBlackout = true;

  // set the current state of the devices in the rig to black, update
  for (auto d : _selected.getDevices()) {
    getRig()->getDevice(d->getId())->setIntensity(0);
    lockDeviceParam(d->getId(), "intensity");
    lockDeviceParam(d->getId(), "color");
  }

  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

  if (mc != nullptr) {
    mc->arnoldRender(false);
    mc->refreshParams();
    mc->refreshAttr();
    mc->redrawResults();
  }

  Snapshot* tmp = new Snapshot(getRig());

  // isolate selected devices
  for (auto d : tmp->getDevices()) {
    if (!_selected.contains(d->getId())) {
      d->setIntensity(0);
    }
  }

  _currentImg = renderImage(tmp, getGlobalSettings()->_renderWidth * getGlobalSettings()->_thumbnailRenderScale,
    getGlobalSettings()->_renderHeight * getGlobalSettings()->_thumbnailRenderScale);
  delete tmp;

  _blackout.setToggleState(_isBlackout, dontSendNotification);
}

void SystemExplorer::unBlackout()
{
  _isBlackout = false;

  // restore the current state
  auto& status = _currentState->getRigData();
  for (auto d : _selected.getDevices()) {
    getRig()->getDevice(d->getId())->setIntensity(status[d->getId()]->getIntensity()->getVal());
    getRig()->getDevice(d->getId())->setParam("color", LumiverseTypeUtils::copy(status[d->getId()]->getColor()));
  }

  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

  if (mc != nullptr) {
    mc->arnoldRender(false);
    mc->refreshParams();
    mc->refreshAttr();
    mc->redrawResults();
  }

  // isolate selected devices
  for (auto d : _currentState->getDevices()) {
    if (!_selected.contains(d->getId())) {
      d->setIntensity(0);
    }
  }

  _currentImg = renderImage(_currentState, getGlobalSettings()->_renderWidth * getGlobalSettings()->_thumbnailRenderScale,
    getGlobalSettings()->_renderHeight * getGlobalSettings()->_thumbnailRenderScale);
  _blackout.setToggleState(_isBlackout, dontSendNotification);
}

void SystemExplorer::solo()
{
  _isSolo = true;
  _solo.setToggleState(_isSolo, dontSendNotification);

  Snapshot* temp = new Snapshot(getRig());
  updateAllImages(temp);
  delete temp;

  repaint();
  _container->repaint();
}

void SystemExplorer::unSolo()
{
  _isSolo = false;
  _solo.setToggleState(_isSolo, dontSendNotification);
  
  Snapshot* temp = new Snapshot(getRig());
  updateAllImages(temp);
  delete temp;

  repaint();
  _container->repaint();
}

void SystemExplorer::togglePin()
{
  updatePinState();

  if (_isPinned) {
    // unlock
    for (auto d : _selected.getIds()) {
      unlockDeviceParam(d, "intensity");
      unlockDeviceParam(d, "color");
    }
    _isPinned = false;
  }
  else {
    // lock
    for (auto d : _selected.getIds()) {
      lockDeviceParam(d, "intensity");
      lockDeviceParam(d, "color");
    }
    _isPinned = true;
  }

  _pin.setToggleState(_isPinned, dontSendNotification);
  getApplicationCommandManager()->invokeDirectly(command::REFRESH_PARAMS, true);
}

void SystemExplorer::clear()
{
  _container->_results.clear();
  _container->resized();
}

void SystemExplorer::updatePinState()
{
  // pinned if all devices are locked, otherwise consider everything unlocked.
  bool allLocked = true;
  for (auto d : _selected.getIds()) {
    allLocked &= isDeviceParamLocked(d, "intensity");
  }

  _isPinned = allLocked;
  _pin.setToggleState(_isPinned, dontSendNotification);
}

void SystemExplorer::updateIntensSlider()
{
  // compute avg intens
  float avgIntens = 0;
  for (auto d : _currentState->getDevices()) {
    if (_selected.contains(d->getId())) {
      avgIntens += d->getIntensity()->asPercent();
    }
  }

  _intens.setValue(avgIntens * 100 / _selected.size(), dontSendNotification);
}

void SystemExplorer::sliderDragEnded(Slider * s)
{
  // Trigger re-render
  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

  if (mc != nullptr) {
    mc->refreshParams();
    mc->refreshAttr();
    mc->redrawResults();
  }

  getApplicationCommandManager()->invokeDirectly(command::ARNOLD_RENDER, true);
}

void SystemExplorer::sliderValueChanged(Slider * s)
{
  // due to this happening fairly frequently while drawing, we update the rig
  // by itself here
  for (auto d : _selected.getIds()) {
    getRig()->getDevice(d)->getIntensity()->setValAsPercent(s->getValue() / 100.0);
  }
}

void SystemExplorer::updateSingleImage(shared_ptr<SearchResultContainer> result)
{
  // grab the actual snapshot this corresponds to
  shared_ptr<SearchResult> sr = result->getSearchResult();
  vectorToExistingSnapshot(sr->_scene, *_temp);

  // we also collect some data to help sort this thing
  Eigen::Vector3d hsv(0, 0, 0);
  float intens = 0;
  int count = 0;

  // this could be threaded, so we access the cached copy of the rig state
  auto rigData = _rigState->getRigData();

  // set all unlocked intensity non-system devices to 0 if not in solo mode
  if (!_isSolo) {
    for (auto d : _temp->getRigData()) {
      if (!_selected.contains(d.first)) {
        if (isDeviceParamLocked(d.first, "intensity")) {
          // if locked, grab values and update the snapshot
          d.second->setIntensity(rigData[d.first]->getIntensity()->getVal());
          d.second->setParam("color", LumiverseTypeUtils::copy(rigData[d.first]->getParam("color")));
        }
        else {
          d.second->setIntensity(0);
        }
      }
      else {
        // sorting data
        if (d.second->paramExists("color")) {
          hsv += d.second->getColor()->getHSV();
        }
        intens += d.second->getIntensity()->asPercent();
        count++;
      }
    }
  }
  else {
    // only turn on the things that this set affects
    for (auto d : _temp->getRigData()) {
      if (!_selected.contains(d.first)) {
        d.second->setIntensity(0);
      }
      else {
        // sorting data
        if (d.second->paramExists("color")) {
          hsv += d.second->getColor()->getHSV();
        }
        intens += d.second->getIntensity()->asPercent();
        count++;
      }
    }
  }

  // now render the thing
  // we'll render at 25% of full res for now, to test speed
  Image preview = renderImage(_temp, getGlobalSettings()->_renderWidth * 0.25, getGlobalSettings()->_renderHeight * 0.25);
  result->setImage(preview);

  // search data
  hsv /= count;
  result->_sortVals["intens"] = intens / count;

  // hue only
  result->_sortVals["hue"] = hsv[0];

  // hsv combined
  float hsCombo = round(hsv[0]) + hsv[1];
  result->_sortVals["hs"] = hsCombo;
}

double SystemExplorer::filteredDist(shared_ptr<SearchResultContainer> r1, shared_ptr<SearchResultContainer> r2)
{
  double dist = 0;
  Snapshot* s1 = vectorToSnapshot(r1->getSearchResult()->_scene);
  Snapshot* s2 = vectorToSnapshot(r2->getSearchResult()->_scene);
  auto ids = _selected.getIds();

  auto rd1 = s1->getRigData();
  auto rd2 = s2->getRigData();

  for (auto id : ids) {
    if (rd1[id]->paramExists("color")) {
      auto hsv1 = rd1[id]->getColor()->getHSV();
      auto hsv2 = rd2[id]->getColor()->getHSV();
      hsv1[0] /= 360;
      hsv2[0] /= 360;

      hsv1 *= rd1[id]->getIntensity()->asPercent();
      hsv2 *= rd2[id]->getIntensity()->asPercent();

      dist += (hsv1 - hsv2).squaredNorm();
    }
    else {
      dist += pow(rd1[id]->getIntensity()->asPercent() - rd2[id]->getIntensity()->asPercent(), 2);
    }
  }
  
  return sqrt(dist) / _selected.size();
}

SystemExplorer::SystemExplorerResults::SystemExplorerResults()
{
}

void SystemExplorer::SystemExplorerResults::resized()
{
  // resize to fit all elements
  int imgWidth = getHeight();
  int totalWidth = _results.size() * imgWidth;
  setSize(totalWidth, getHeight());

  // now place them
  auto lbounds = getLocalBounds();

  for (auto r : _results) {
    r->setBounds(lbounds.removeFromLeft(imgWidth));
  }
}


//==============================================================================

SystemExplorerContainer::SystemExplorerContainer(SearchResultsContainer* rc) : _add("Add View"), _rc(rc)
{
  _add.setName("add");
  _add.addListener(this);
  addAndMakeVisible(_add);
  _counter = 0;
}

SystemExplorerContainer::~SystemExplorerContainer()
{
  for (auto e : _explorers)
    delete e;

  for (auto b : _deleteButtons)
    delete b;
}

void SystemExplorerContainer::paint(Graphics & g)
{
  g.fillAll(Colour(0xff333333));
  g.setColour(Colours::white);
  g.setFont(14);

  auto lbounds = getLocalBounds();
  for (int i = 0; i < _explorers.size(); i++) {
    //g.drawFittedText(_explorers[i]->getName(), lbounds.removeFromTop(24).reduced(2), Justification::left, 1);
    lbounds.removeFromTop(_rowHeight);
  }
}

void SystemExplorerContainer::resized()
{
  int height = _rowHeight * _explorers.size() + 24;
  setSize(getWidth(), height);

  auto lbounds = getLocalBounds();

  auto top = lbounds.removeFromTop(24);
  _add.setBounds(top.removeFromLeft(80));

  for (int i = 0; i < _explorers.size(); i++) {
    auto rowBounds = lbounds.removeFromTop(_rowHeight);

    _deleteButtons[i]->setBounds(rowBounds.getX() + 2, rowBounds.getY() + 2, 20, 20);
    _explorers[i]->setBounds(rowBounds);
  }
}

void SystemExplorerContainer::setHeight(int height)
{
  _rowHeight = height;
  resized();
}

void SystemExplorerContainer::sort(string method)
{
  for (auto e : _explorers) {
    e->sort(method);
  }
}

void SystemExplorerContainer::addContainer(string system)
{
  SystemExplorer* e = new SystemExplorer(system, system);
  addContainer(e);
}

void SystemExplorerContainer::addContainer()
{
  SystemExplorer* e = new SystemExplorer(_rc->getAllResults(), "new view");
  addContainer(e);
}

void SystemExplorerContainer::addResult(shared_ptr<SearchResultContainer> result)
{
  for (auto e : _explorers) {
    e->addNewResult(result);
  }
}

void SystemExplorerContainer::clear()
{
  for (auto e : _explorers)
    e->clear();
}

void SystemExplorerContainer::updateImages()
{
  ThreadPool renderers(max(1u, thread::hardware_concurrency() - 2));

  for (auto e : _explorers) {
    renderers.addJob(new UpdateImageJob(e, new Snapshot(getRig())), true);
  }

  // wait for completion
  while (renderers.getNumJobs() > 0)
  {
    this_thread::sleep_for(100ms);
  }

  for (auto e : _explorers) {
    e->updatePinState();
    e->updateIntensSlider();
    e->repaint();
  }
}

void SystemExplorerContainer::buttonClicked(Button * b)
{
  if (b->getName() == "add") {
    // create popup menu
    PopupMenu main;
    PopupMenu sys;
    PopupMenu area;
    
    int i = 1;
    main.addItem(i, "Selected Lights");
    i++;

    main.addItem(i, "Custom Query");
    i++;

    map<int, string> cmdMap;

    // populate systems
    for (auto s : getRig()->getMetadataValues("system")) {
      sys.addItem(i, s);
      cmdMap[i] = "$system=" + s;
      i++;
    }

    // populate areas
    for (auto a : getRig()->getMetadataValues("area")) {
      area.addItem(i, a);
      cmdMap[i] = "$area=" + a;
      i++;
    }

    // add to menu
    main.addSubMenu("Systems", sys);
    main.addSubMenu("Areas", area);

    // get a response
    int result = main.show();

    // no selection
    if (result == 0)
      return;

    SystemExplorer* e;
    if (result == 1) {
      // get selected lights
      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
      auto ids = mc->getSelectedDeviceIds();

      if (ids.size() == 0) {
        getStatusBar()->setStatusMessage("Can't add view: no lights selected", true, false);
        return;
      }

      DeviceSet s(getRig());
      for (auto id : ids) {
        s = s.add(id.toStdString());
      }

      e = new SystemExplorer(_rc->getAllResults(), s.info(), s);
    }
    else if (result == 2) {
      AlertWindow w("Custom Lumiverse Query",
        "Enter a Lumiverse query to select relevant devices.",
        AlertWindow::QuestionIcon);

      w.addTextEditor("query", "", "Query:");

      w.addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
      w.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));

      if (w.runModalLoop() != 0) // is they picked 'ok'
      {
        // this is the text they entered..
        String cmd = w.getTextEditorContents("query");
        DeviceSet selected = getRig()->select(cmd.toStdString());

        if (selected.size() == 0) {
          getStatusBar()->setStatusMessage("Can't add view: no lights selected using query \"" + cmd + "\"", true, false);
          return;
        }

        getStatusBar()->setStatusMessage("Added new view using query \"" + cmd + "\"");
        e = new SystemExplorer(_rc->getAllResults(), cmd.toStdString(), selected);
      }
      else {
        getStatusBar()->setStatusMessage("Cancelled custom selection.");
        return;
      }
    }
    else {
      // selected command
      string cmd = cmdMap[result];

      getStatusBar()->setStatusMessage("Added new view using query \"" + cmd + "\"");

      // add container with selected devices
      e = new SystemExplorer(_rc->getAllResults(), cmd, getRig()->select(cmd));
    }

    addContainer(e);
  }
  else {
    // this is a delete button
    String toDelete = b->getName();

    SystemExplorer* toRemove;
    for (auto v : _explorers) {
      if (v->getName() == toDelete) {
        toRemove = v;
      }
    }

    _explorers.removeAllInstancesOf(toRemove);
    _deleteButtons.removeAllInstancesOf((TextButton*)b);
    delete b;
    delete toRemove;
    resized();
  }
}

void SystemExplorerContainer::addContainer(SystemExplorer * e)
{
  addAndMakeVisible(e);
  _explorers.add(e);

  // create delete button
  TextButton* del = new TextButton("x");
  del->setTooltip("Delete");
  del->setName(String(_counter));
  e->setName(String(_counter));
  del->addListener(this);
  addAndMakeVisible(del);
  _deleteButtons.add(del);

  _counter++;

  resized();
}

SystemExplorerContainer::UpdateImageJob::UpdateImageJob(SystemExplorer* e, Snapshot* state) :
  ThreadPoolJob("update images " + e->getName()), _e(e), _state(state)
{
}

SystemExplorerContainer::UpdateImageJob::~UpdateImageJob()
{
  delete _state;
}

ThreadPoolJob::JobStatus SystemExplorerContainer::UpdateImageJob::runJob()
{
  _e->updateAllImages(_state);
  return ThreadPoolJob::JobStatus::jobHasFinished;
}

// ============================================================================

ExplorerPanel::ExplorerPanel()
{
  // actual init happens a bit later
}

ExplorerPanel::~ExplorerPanel()
{
  delete _viewer;
}

void ExplorerPanel::initWithResults(SearchResultsContainer * rc)
{
  _exp = new SystemExplorerContainer(rc);
  _viewer = new Viewport();
  _viewer->setViewedComponent(_exp, true);
  addAndMakeVisible(_viewer);
  _exp->setHeight(200);
}

void ExplorerPanel::resized()
{
  auto lbounds = getLocalBounds();
  
  _viewer->setBounds(lbounds);
  _exp->setSize(_viewer->getMaximumVisibleWidth(), _viewer->getMaximumVisibleHeight());
}

void ExplorerPanel::paint(Graphics & g)
{
  g.fillAll(Colour(0xff333333));
}

void ExplorerPanel::buttonClicked(Button * b)
{
}

SystemExplorerContainer * ExplorerPanel::getContainer()
{
  return _exp;
}
