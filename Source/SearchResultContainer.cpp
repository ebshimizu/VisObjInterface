/*
  ==============================================================================

    SearchResultContainer.cpp
    Created: 7 Jan 2016 4:59:12pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SearchResultContainer.h"
#include "MainComponent.h"


//==============================================================================
SearchResultBlender::SearchResultBlender(SearchResult * s) : _target(s)
{
  Snapshot* b = new Snapshot(getRig());
  _base = snapshotToVector(b);
  delete b;

  _blender.setRange(0, 1, 0.01);
  _blender.setSliderStyle(Slider::LinearBar);
  _blender.setValue(0, dontSendNotification);
  _blender.addListener(this);
	_blender.setChangeNotificationOnlyOnRelease(true);
  addAndMakeVisible(_blender);
}

SearchResultBlender::~SearchResultBlender()
{
}

void SearchResultBlender::paint(Graphics & g)
{
  auto lbounds = getLocalBounds();
  g.setColour(Colours::white);
  g.setFont(12);
  g.drawFittedText("Blend", lbounds.removeFromTop(20), Justification::centred, 1);
}

void SearchResultBlender::resized()
{
  auto lbounds = getLocalBounds();
  lbounds.removeFromTop(20);
  _blender.setBounds(lbounds);
}

void SearchResultBlender::sliderValueChanged(Slider * s)
{
  float a = (float) s->getValue();

  // lerp values
  Eigen::VectorXd interp = (1 - a) * _base + a * _target->_scene;

  // Update rig
  Snapshot* newScene = vectorToSnapshot(interp);
  newScene->loadRig(getRig());
  delete newScene;

  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

  if (mc != nullptr) {
    mc->arnoldRenderNoPopup();
    mc->refreshParams();
    mc->refreshAttr();
    getRecorder()->log(ACTION, "User interpolated scene (" + String(a).toStdString() + "%)");
  }
}


//==============================================================================
SearchResultContainer::SearchResultContainer(shared_ptr<SearchResult> result, bool isHistoryItem) :
  _result(result), _isHistoryItem(isHistoryItem)
{
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
  regenToolTip();
  _isHovered = false;

  // magic number alert
  _clusterContents = new SearchResultList();
  _clusterContents->setWidth(820);
  _clusterContents->setCols(3);
  _wasSelected = false;
  _selectTime = 0;
  _isMostRecent = false;
}

SearchResultContainer::~SearchResultContainer()
{
  delete _clusterContents;
}

void SearchResultContainer::regenToolTip()
{
  String tt = "";

	bool first = true;
  for (const auto& t : _result->_editHistory) {
    if (!first) {
      tt = tt + " -> " + t->_name;
    }
    else {
      tt = tt + t->_name;
      first = false;
    }
  }

  //tt = tt + "(" + String(-_result->_objFuncVal) + ")";

	for (auto& m : _result->_extraFuncs) {
		tt = tt + m.first + ": " + String(m.second) + "\n";
	}

  setTooltip(tt);
}

void SearchResultContainer::paint (Graphics& g)
{
  if (_isHovered) {
    g.fillAll(Colours::lightyellow);
  }
  else if (_isMostRecent) {
    g.fillAll(Colours::lightgrey);
  }
  else {
    g.fillAll(Colour(0xff333333));
  }

  auto lbounds = getLocalBounds();
  lbounds.reduce(2, 2);
  g.drawImageWithin(_render, lbounds.getX(), lbounds.getY(), lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);

  auto textArea = lbounds.removeFromTop(25);
  textArea.removeFromLeft(10);
  g.setColour(Colours::black);
  g.setFont(12);
  g.drawText(String(_result->_sampleNo), textArea, Justification::centredLeft, false);
  g.drawText(String(_result->_sampleNo), textArea.translated(-1, -1), Justification::centredLeft, false);
  g.drawText(String(_result->_sampleNo), textArea.translated(1, 2), Justification::centredLeft, false);
  g.drawText(String(_result->_sampleNo), textArea.translated(1, 0), Justification::centredLeft, false);
  g.drawText(String(_result->_sampleNo), textArea.translated(-1, 0), Justification::centredLeft, false);
  g.drawText(String(_result->_sampleNo), textArea.translated(0, -1), Justification::centredLeft, false);
  g.drawText(String(_result->_sampleNo), textArea.translated(0, 1), Justification::centredLeft, false);

  g.setColour(Colours::white);
  g.setFont(12);
  g.drawText(String(_result->_sampleNo), textArea, Justification::centredLeft, false);
}

void SearchResultContainer::resized()
{
}

void SearchResultContainer::setImage(Image img)
{
  _render = img;

  if (getGlobalSettings()->_grayscaleMode)
    _render.desaturate();

  // create feature vector
  // consists of Lab image (0-2) and per-channel Lab gradient direction (3-5)
	Image scaled = _render.rescaled(100, 100);
  _features.resize(100 * 100 * 6);

	updateMask();

  for (int y = 0; y < scaled.getHeight(); y++) {
    for (int x = 0; x < scaled.getWidth(); x++) {
      int idx = (y * scaled.getWidth() + x) * 6;
      auto px = scaled.getPixelAt(x, y);
      Eigen::Vector3d Lab = rgbToLab(px.getRed() / 255.0, px.getGreen() / 255.0, px.getBlue() / 255.0);

      _features[idx] = Lab[0];
      _features[idx + 1] = Lab[1];
      _features[idx + 2] = Lab[2];
    }
  }

	computeGradient();
}

void SearchResultContainer::computeGradient()
{
  // magic number warning wow
	int dim = 100;

	// compute luminance gradient (x, y, magnitude)
	for (int y = 0; y < dim; y++) {
		for (int x = 0; x < dim; x++) {
			int idx = (y * dim + x) * 6;

			// we'll set borders to 0
			if (y == 0 || y == dim - 1 || x == 0 || x == dim - 1) {
				_features[idx + 3] = 0;
				_features[idx + 4] = 0;
				_features[idx + 5] = 0;
				continue;
			}

			// compute per-pixel gradient direction
			// *100 for scaling to match Lab range
			Eigen::Vector2d grad(_features[idx + dim * 6] - _features[idx - dim * 6], _features[idx + 6] - _features[idx - 6]);
			_features[idx + 3] = grad[0];
			_features[idx + 4] = grad[1];
			_features[idx + 5] = 0;// grad.norm();
		}
	}
}

void SearchResultContainer::clearSearchResult()
{
  _result = nullptr;
}

void SearchResultContainer::setSystem(DeviceSet selected)
{
  _selected = selected;
}

void SearchResultContainer::mouseDown(const MouseEvent & event)
{
  if (event.mods.isRightButtonDown()) {
    if (_selected.size() != 0) {
      PopupMenu m;

      m.addItem(1, "Transfer to Stage");
      m.addSeparator();
      m.addItem(2, "Pin Intensity and Color");
      m.addItem(3, "Pin Intensity");
      m.addItem(4, "Pin Color");
      int result = m.show();

      if (result == 0)
        return;

      Snapshot* s = vectorToSnapshot(_result->_scene);
      auto sd = s->getRigData();

      if (result == 1) {
        for (auto d : _selected.getDevices()) {
          getRig()->getDevice(d->getId())->setParam("intensity", sd[d->getId()]->getIntensity());

          if (getRig()->getDevice(d->getId())->paramExists("color")) {
            getRig()->getDevice(d->getId())->setParam("color", sd[d->getId()]->getColor());
          }
        }

        // leave other devices unaffected
        _selectTime = chrono::high_resolution_clock::now().time_since_epoch().count();
        _wasSelected = true;
      }
      else {
        bool pinIntens = (result == 2 || result == 3);
        bool pinColor = (result == 2 || result == 4);
        
        // get the system lights from the rig
        for (auto d : _selected.getDevices()) {
          if (pinIntens) {
            getRig()->getDevice(d->getId())->setParam("intensity", sd[d->getId()]->getIntensity());
            lockDeviceParam(d->getId(), "intensity");
          }

          if (pinColor) {
            getRig()->getDevice(d->getId())->setParam("color", sd[d->getId()]->getColor());
            lockDeviceParam(d->getId(), "color");
          }
        }
      }

      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

      if (mc != nullptr) {
        mc->arnoldRender(!_isHistoryItem);
        mc->refreshParams();
        mc->refreshAttr();
        mc->redrawResults();
        getRecorder()->log(ACTION, "User Transferred DeviceSet: " + _selected.info());
      }

      if (result != 1) {
        getApplicationCommandManager()->invokeDirectly(command::SEARCH, true);
      }
    }
    else {
      PopupMenu m;
      m.addItem(1, "Move to Stage");
      m.addItem(2, "Move Unpinned to Stage");
      m.addItem(3, "Move Selected to Stage");

      PopupMenu subTransferArea;

      // The id of each menu command corresponds to a lumiverse query that
      // selects the proper devices
      int startId = m.getNumItems() + 1;
      map<int, string> commands;

      for (auto area : getRig()->getMetadataValues("area")) {
        subTransferArea.addItem(startId, area);
        commands[startId] = "$area=" + area;
        startId++;
      }
      m.addSubMenu("Transfer Area to Stage", subTransferArea);

      PopupMenu subTransferSystem;
      for (auto system : getRig()->getMetadataValues("system")) {
        subTransferSystem.addItem(startId++, system);
        commands[startId] = "$system=" + system;
        startId++;
      }
      m.addSubMenu("Transfer System to Stage", subTransferSystem);

      const int result = m.show();

      if (result == 1) {
        Snapshot* s = vectorToSnapshot(_result->_scene);
        s->loadRig(getRig());
        delete s;

        // dont populate anything, let user select what to keep
        MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

        if (mc != nullptr) {
          mc->arnoldRender(!_isHistoryItem);
          mc->refreshParams();
          mc->refreshAttr();
          //mc->populateSystemViews();
          getRecorder()->log(ACTION, "User picked scene for stage: " + getTooltip().toStdString());
        }
      }
      else if (result == 2) {
        Snapshot* s = vectorToSnapshot(_result->_scene);

        MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

        if (mc != nullptr) {
          mc->transferSelected(s, getRig()->getAllDevices(), false);
          mc->refreshParams();
          mc->refreshAttr();
          mc->redrawResults();
        }

        delete s;
      }
      else if (result == 3) {
        Snapshot* s = vectorToSnapshot(_result->_scene);

        MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

        if (mc != nullptr) {
          mc->transferSelected(s);
          mc->refreshParams();
          mc->refreshAttr();
          mc->redrawResults();
        }

        delete s;
      }
      else {
        string query = commands[result];
        Snapshot* s = vectorToSnapshot(_result->_scene);

        MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

        if (mc != nullptr) {
          mc->transferSelected(s, getRig()->select(query));
          mc->refreshParams();
          mc->refreshAttr();
          mc->redrawResults();
        }

        delete s;
      }
    }
  }
  else if (event.mods.isLeftButtonDown() && event.mods.isCommandDown()) {
    // popup window to allow blending of current scene with search scene
    SearchResultBlender* popupComponent = new SearchResultBlender(_result.get());
    popupComponent->setSize(300, 45);
    CallOutBox::launchAsynchronously(popupComponent, getScreenBounds(), nullptr);
  }
  else if (event.mods.isLeftButtonDown()) {
    if (isClusterCenter()) {
      Viewport* vp = new Viewport();
      _clusterContents->setWidth(820);
      vp->setViewedComponent(_clusterContents, false);
      vp->setSize(_clusterContents->getWidth() + vp->getScrollBarThickness(), min(300, _clusterContents->getHeight()));

      CallOutBox::launchAsynchronously(vp, getScreenBounds(), nullptr);
    }
  }
}

void SearchResultContainer::mouseEnter(const MouseEvent & /* event */)
{
  _isHovered = true;
  getGlobalSettings()->_showThumbnailImg = true;
  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
  mc->setThumbImage(_render);
  getRecorder()->log(HOVER, vectorToString(_result->_scene).toStdString());
  mc->repaint();
  _clusterContents->repaint();
  getParentComponent()->repaint();
}

void SearchResultContainer::mouseExit(const MouseEvent & /* event */)
{
  _isHovered = false;
  getGlobalSettings()->_showThumbnailImg = false;
  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
  mc->repaint();
  _clusterContents->repaint();
  getParentComponent()->repaint();
}

int SearchResultContainer::compareElements(SearchResultContainer * first, SearchResultContainer * second)
{
  double firstScore = first->getSearchResult()->_objFuncVal;
  double secondScore = second->getSearchResult()->_objFuncVal;

  if (firstScore < secondScore)
    return -1;
  if (firstScore > secondScore)
    return 1;
  else
    return 0;
}

Eigen::VectorXd SearchResultContainer::getFeatures()
{
  return _features;
}

void SearchResultContainer::setFeatures(Eigen::VectorXd features)
{
  _features = features;
}

bool SearchResultContainer::isClusterCenter()
{
  return _clusterContents->size() > 0;
}

int SearchResultContainer::numResults()
{
  if (!isClusterCenter())
    return 1;

  else {
    return _clusterContents->numElements();
  }
}

void SearchResultContainer::addToCluster(shared_ptr<SearchResultContainer> elem)
{
  _clusterContents->addResult(elem);
}

Array<shared_ptr<SearchResultContainer> > SearchResultContainer::getResults()
{
  if (isClusterCenter()) {
    return _clusterContents->getAllResults();
  }
  else {
    return Array<shared_ptr<SearchResultContainer> >();
  }
}

double SearchResultContainer::dist(SearchResultContainer * y, DistanceMetric metric, bool overrideMask, bool invert)
{
  switch (metric) {
  case PPAVGLAB:
    return avgPixDist(y, overrideMask, invert);
  case PPMAXLAB:
    return maxPixDist(y, overrideMask, invert);
  case PP90LAB:
    return pctPixDist(y, overrideMask, invert);
  case L2LAB:
    return l2dist(y, overrideMask, invert);
  case L2PARAM:
    return l2paramDist(y);
  case L2SOFTMAXPARAM:
    return l2paramDistSoftmax(y);
  case L2LUMINANCE:
    return l2LuminanceDist(y, overrideMask, invert);
  case ATTRDIST:
    return attrDist(y);
	case DIRPPAVGLAB:
		return directedAvgPixDist(y, overrideMask, invert);
	case DIRPPAVG:
		return directedOnlyAvgPixDist(y, overrideMask, invert);
  case KEYPARAM:
    return l2SelectedParamDist(y);
  case L2GRAYPARAM:
    return l2GrayParamDist(y);
  default:
    return avgPixDist(y, overrideMask, invert);
  }
}

double SearchResultContainer::avgPixDist(SearchResultContainer * y, bool overrideMask, bool invert)
{
  double sum = 0;
  int count = 0;
  Eigen::VectorXd fy = y->getFeatures();

  // iterate through pixels in groups of 6
  for (int i = 0; i < _features.size() / 6; i++) {
    int idx = i * 6;

    double maskFactor = 1;
    if (!overrideMask && getGlobalSettings()->_useFGMask) {
      maskFactor = _mask.getPixelAt(i % 100, (int)(i / 100)).getBrightness();

      if (invert)
        maskFactor = 1 - maskFactor;

      if (maskFactor > 0)
        count++;
    }
    else {
      count++;
    }

    sum += maskFactor * sqrt(pow(fy[idx] - _features[idx], 2) +
      pow(fy[idx + 1] - _features[idx + 1], 2) +
      pow(fy[idx + 2] - _features[idx + 2], 2));
  }

  return sum / count;
}

double SearchResultContainer::l2dist(SearchResultContainer * y, bool overrideMask, bool invert)
{
  double sum = 0;
  Eigen::VectorXd fy = y->getFeatures();

  // iterate through pixels in groups of 6
  for (int i = 0; i < _features.size() / 6; i++) {
    int idx = i * 6;

    double maskFactor = 1;
    if (!overrideMask && getGlobalSettings()->_useFGMask) {
      maskFactor = _mask.getPixelAt(i % 100, (int)(i / 100)).getBrightness();

      if (invert)
        maskFactor = 1 - maskFactor;
    }

    sum += maskFactor * (pow(fy[idx] - _features[idx], 2) +
      pow(fy[idx + 1] - _features[idx + 1], 2) +
      pow(fy[idx + 2] - _features[idx + 2], 2));
  }

  return sqrt(sum);
}

double SearchResultContainer::maxPixDist(SearchResultContainer * y, bool overrideMask, bool invert)
{
  double maxDist = 0;
  Eigen::VectorXd fy = y->getFeatures();

  // iterate through pixels in groups of 6
  for (int i = 0; i < _features.size() / 6; i++) {
    int idx = i * 6;

    double maskFactor = 1;
    if (!overrideMask && getGlobalSettings()->_useFGMask) {
      maskFactor = _mask.getPixelAt(i % 100, (int)(i / 100)).getBrightness();
      
      if (invert)
        maskFactor = 1 - maskFactor;
    }

    double dist = maskFactor * sqrt(pow(fy[idx] - _features[idx], 2) +
      pow(fy[idx + 1] - _features[idx + 1], 2) +
      pow(fy[idx + 2] - _features[idx + 2], 2));

    if (dist > maxDist)
      maxDist = dist;
  }

  return maxDist;
}

double SearchResultContainer::pctPixDist(SearchResultContainer * y, bool overrideMask, bool invert)
{
  Array<double> dists;
  Eigen::VectorXd fy = y->getFeatures();

  // iterate through pixels in groups of 6
  for (int i = 0; i < _features.size() / 6; i++) {
    int idx = i * 6;

    double maskFactor = 1;
    if (!overrideMask && getGlobalSettings()->_useFGMask) {
      maskFactor = _mask.getPixelAt(i % 100, (int)(i / 100)).getBrightness();

      if (invert)
        maskFactor = 1 - maskFactor;
    }

    double dist = maskFactor * sqrt(pow(fy[idx] - _features[idx], 2) +
      pow(fy[idx + 1] - _features[idx + 1], 2) +
      pow(fy[idx + 2] - _features[idx + 2], 2));

    dists.add(dist);
  }

  dists.sort();

  // pick point around 90% mark
  int pct = (int) (.9 * dists.size());

  return dists[pct];
}

double SearchResultContainer::l2paramDist(SearchResultContainer * y)
{
  // the scene is stored as a vector in the SearchResult of each container.
  Snapshot* ys = vectorToSnapshot(y->getSearchResult()->_scene);
  Snapshot* xs = vectorToSnapshot(_result->_scene);

  auto xdevices = xs->getRigData();
  auto ydevices = ys->getRigData();

  double sum = 0;

  for (auto& d : xdevices) {
    // get relevant parameters (color and intensity)
    Device* dy = ydevices[d.first];

    float xintens = d.second->getIntensity()->asPercent();
    float yintens = dy->getIntensity()->asPercent();

    LumiverseColor* xcolor = d.second->getColor();
    LumiverseColor* ycolor = dy->getColor();

    Eigen::Vector3d xc(1, 1, 1);
    Eigen::Vector3d yc(1, 1, 1);    

    if (xcolor != nullptr)
      xc = xcolor->getRGB() * xintens;

    if (ycolor != nullptr)
      yc = ycolor->getRGB() * yintens;

    sum += (xc - yc).norm();
  }

  delete ys;
  delete xs;
  return sqrt(sum);
}

double SearchResultContainer::l2paramDistSoftmax(SearchResultContainer * y)
{
  // the scene is stored as a vector in the SearchResult of each container.
  Snapshot* ys = vectorToSnapshot(y->getSearchResult()->_scene);
  Snapshot* xs = vectorToSnapshot(_result->_scene);

  auto xdevices = xs->getRigData();
  auto ydevices = ys->getRigData();

  // vector of intensity scaled RGB colors
  Eigen::VectorXd xparams, yparams;
  xparams.resize(xdevices.size() * 3);
  yparams.resize(ydevices.size() * 3);
  xparams.setZero();
  yparams.setZero();

  int i = 0;
  for (auto& d : xdevices) {
    // get relevant parameters (color and intensity)
    Device* dy = ydevices[d.first];

    float xintens = d.second->getIntensity()->getVal();
    float yintens = dy->getIntensity()->getVal();

    LumiverseColor* xcolor = d.second->getColor();
    LumiverseColor* ycolor = dy->getColor();

    Eigen::Vector3d xc = xcolor->getRGB() * xintens;
    Eigen::Vector3d yc = ycolor->getRGB() * yintens;

    int idx = i * 3;
    xparams[idx] = xc[0];
    xparams[idx + 1] = xc[1];
    xparams[idx + 2] = xc[2];

    yparams[idx] = yc[0];
    yparams[idx + 1] = yc[1];
    yparams[idx + 2] = yc[2];

    i++;
  }

  // softmax normalization
  for (i = 0; i < xparams.size(); i++) {
    xparams[i] = pow(xparams[i], xparams.size());
    yparams[i] = pow(yparams[i], yparams.size());
  }

  xparams /= xparams.sum();
  yparams /= yparams.sum();

  delete ys;
  delete xs;

  return (xparams - yparams).norm();
}

double SearchResultContainer::l2LuminanceDist(SearchResultContainer * y, bool overrideMask, bool invert)
{
  // just pull the luminance component of each image and take the L2 norm
  double sum = 0;
  Eigen::VectorXd yf = y->getFeatures();

  for (int i = 0; i < _features.size() / 6; i++) {
    double maskFactor = 1;
    if (!overrideMask && getGlobalSettings()->_useFGMask) {
      maskFactor = _mask.getPixelAt(i % 100, (int)(i / 100)).getBrightness();

      if (invert)
        maskFactor = 1 - maskFactor;
    }

    sum += maskFactor * pow(_features[i * 6] - yf[i * 6], 2);
  }

  return sqrt(sum);
}

double SearchResultContainer::attrDist(SearchResultContainer * y)
{
  return abs(_result->_objFuncVal - y->getSearchResult()->_objFuncVal);
}

double SearchResultContainer::directedAvgPixDist(SearchResultContainer * y, bool overrideMask, bool invert)
{
	double sum = 0;
	int count = 0;
	Eigen::VectorXd fy = y->getFeatures();

	// iterate through pixels in groups of 6
	for (int i = 0; i < _features.size() / 6; i++) {
		int idx = i * 6;

		double maskFactor = 1;
		if (!overrideMask && getGlobalSettings()->_useFGMask) {
			maskFactor = _mask.getPixelAt(i % 100, (int)(i / 100)).getBrightness();

			if (invert)
				maskFactor = 1 - maskFactor;

			if (maskFactor > 0)
				count++;
		}
		else {
			count++;
		}

		sum += maskFactor * sqrt(pow(fy[idx] - _features[idx], 2) +
			pow(fy[idx + 1] - _features[idx + 1], 2) +
			pow(fy[idx + 2] - _features[idx + 2], 2) +
			pow(fy[idx + 3] - _features[idx + 3], 2) +
			pow(fy[idx + 4] - _features[idx + 4], 2) +
			pow(fy[idx + 5] - _features[idx + 5], 2));
	}

	return sum / count;
}

double SearchResultContainer::directedOnlyAvgPixDist(SearchResultContainer * y, bool overrideMask, bool invert)
{
	double sum = 0;
	int count = 0;
	Eigen::VectorXd fy = y->getFeatures();

	// iterate through pixels in groups of 6
	for (int i = 0; i < _features.size() / 6; i++) {
		int idx = i * 6;

		double maskFactor = 1;
		if (!overrideMask && getGlobalSettings()->_useFGMask) {
			maskFactor = _mask.getPixelAt(i % 100, (int)(i / 100)).getBrightness();

			if (invert)
				maskFactor = 1 - maskFactor;

			if (maskFactor > 0)
				count++;
		}
		else {
			count++;
		}

		sum += maskFactor * sqrt(pow(fy[idx + 3] - _features[idx + 3], 2) +
			pow(fy[idx + 4] - _features[idx + 4], 2) +
			pow(fy[idx + 5] - _features[idx + 5], 2));
	}

	return sum / count;
}

double SearchResultContainer::l2SelectedParamDist(SearchResultContainer * y)
{
  vector<string> keys = getGlobalSettings()->_keyIds;
  Snapshot* xs = vectorToSnapshot(_result->_scene);
  Snapshot* ys = vectorToSnapshot(y->getSearchResult()->_scene);

  auto& xdevices = xs->getRigData();
  auto& ydevices = ys->getRigData();

  // select the key lights
  // vector format: intensity-adjusted RGB (3 channels)
  Eigen::VectorXd xfeats, yfeats;
  xfeats.resize(keys.size() * 3);
  yfeats.resizeLike(xfeats);

  for (int i = 0; i < keys.size(); i++) {
    string key = keys[i];
    auto xrgb = xdevices[key]->getColor()->getRGB() * xdevices[key]->getIntensity()->asPercent();
    auto yrgb = ydevices[key]->getColor()->getRGB() * ydevices[key]->getIntensity()->asPercent();

    xfeats[i * 3] = xrgb[0];
    xfeats[i * 3 + 1] = xrgb[1];
    xfeats[i * 3 + 2] = xrgb[2];

    yfeats[i * 3] = yrgb[0];
    yfeats[i * 3 + 1] = yrgb[1];
    yfeats[i * 3 + 2] = yrgb[2];
  }

  delete xs;
  delete ys;

  return (xfeats - yfeats).norm();
}

double SearchResultContainer::l2GrayParamDist(SearchResultContainer * y)
{
  // the scene is stored as a vector in the SearchResult of each container.
  Snapshot* ys = vectorToSnapshot(y->getSearchResult()->_scene);
  Snapshot* xs = vectorToSnapshot(_result->_scene);

  auto xdevices = xs->getRigData();
  auto ydevices = ys->getRigData();

  double sum = 0;

  for (auto& d : xdevices) {
    // get relevant parameters (color and intensity)
    Device* dy = ydevices[d.first];

    float xintens = d.second->getIntensity()->asPercent();
    float yintens = dy->getIntensity()->asPercent();

    LumiverseColor* xcolor = d.second->getColor();
    LumiverseColor* ycolor = dy->getColor();

    Eigen::Vector3d xc(1, 1, 1);
    Eigen::Vector3d yc(1, 1, 1);

    if (xcolor != nullptr)
      xc = xcolor->getRGB() * xintens;

    if (ycolor != nullptr)
      yc = ycolor->getRGB() * yintens;

    xintens = (float)xc.mean() * xintens;
    yintens = (float)yc.mean() * yintens;

    float diff = xintens - yintens;

    sum += diff * diff;
  }

  delete ys;
  delete xs;
  return sqrt(sum);
}

void SearchResultContainer::sort(AttributeSorter * s)
{
  if (_clusterContents->size() > 1) {
    _clusterContents->sort(s);
  }
}

void SearchResultContainer::updateMask()
{
	if (getGlobalSettings()->_useFGMask) {
		_mask = getGlobalSettings()->_fgMask.rescaled(100, 100);
	}
}

long long SearchResultContainer::getTime()
{
  return _selectTime;
}

void SearchResultContainer::setMostRecent(bool isRecent)
{
  _isMostRecent = isRecent;
}

bool SearchResultContainer::wasSelected()
{
  return _wasSelected;
}

// ======================================================================
// i need this in multiple places so i'll just leave it here
double repulsionTerm(SearchResultContainer * s, Array<shared_ptr<SearchResultContainer>>& pts, double c, double r, DistanceMetric metric)
{
  double sum = 0;

  for (auto p : pts) {
    if (p == nullptr)
      continue;

    double dist = p->dist(s, metric, false, false);

    if (dist > r)
      continue;

    double ratio = dist / r;
    sum += (1 - ratio);
  }

  return sum * c;
}
