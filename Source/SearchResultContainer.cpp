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
  float a = s->getValue();

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
SearchResultContainer::SearchResultContainer(SearchResult* result) : _result(result)
{
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
  regenToolTip();
  _isHovered = false;

  // magic number alert
  _clusterContents = new SearchResultList();
  _clusterContents->setWidth(820);
}

SearchResultContainer::~SearchResultContainer()
{
  if (_result != nullptr)
    delete _result;

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

  tt = tt + " (" + String(-_result->_objFuncVal) + ")";
  setTooltip(tt);
}

void SearchResultContainer::paint (Graphics& g)
{
  if (_isHovered) {
    g.fillAll(Colours::yellow);
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
  g.setFont(14);
  g.drawText(String(_result->_sampleNo), textArea, Justification::centredLeft, false);

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
  Image scaled = _render.rescaled(64, 64);
  _features.resize(64 * 64 * 3);

  bool useMask = getGlobalSettings()->_useFGMask;

  if (useMask) {
    _mask = getGlobalSettings()->_fgMask.rescaled(64, 64);
  }

  for (int y = 0; y < scaled.getHeight(); y++) {
    for (int x = 0; x < scaled.getWidth(); x++) {
      int idx = (y * scaled.getWidth() + x) * 3;
      auto px = scaled.getPixelAt(x, y);
      Eigen::Vector3d Lab = rgbToLab(px.getRed() / 255.0, px.getGreen() / 255.0, px.getBlue() / 255.0);

      _features[idx] = Lab[0];
      _features[idx + 1] = Lab[1];
      _features[idx + 2] = Lab[2];
    }
  }
}

void SearchResultContainer::clearSearchResult()
{
  if (_result != nullptr)
    delete _result;

  _result = nullptr;
}

void SearchResultContainer::mouseDown(const MouseEvent & event)
{
  if (event.mods.isRightButtonDown()) {
    PopupMenu m;
    m.addItem(1, "Move to Stage");
    m.addItem(2, "Repeat Search with Selected");

    const int result = m.show();

    if (result == 1) {
      Snapshot* s = vectorToSnapshot(_result->_scene);
      s->loadRig(getRig());
      delete s;

      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

      if (mc != nullptr) {
        mc->arnoldRender();
        mc->refreshParams();
        mc->refreshAttr();
        getRecorder()->log(ACTION, "User picked scene for stage: " + getTooltip().toStdString());
      }
    }
    else if (result == 2) {
      Snapshot* s = vectorToSnapshot(_result->_scene);
      s->loadRig(getRig());
      delete s;
      MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

      if (mc != nullptr) {
        mc->refreshParams();
        mc->refreshAttr();
        mc->search();
      }
    }
  }
  else if (event.mods.isLeftButtonDown() && event.mods.isCommandDown()) {
    // popup window to allow blending of current scene with search scene
    SearchResultBlender* popupComponent = new SearchResultBlender(_result);
    popupComponent->setSize(300, 45);
    CallOutBox& cb = CallOutBox::launchAsynchronously(popupComponent, getScreenBounds(), nullptr);
  }
  else if (event.mods.isLeftButtonDown()) {
    //if (isClusterCenter()) {
    //  Viewport* vp = new Viewport();
    //  vp->setViewedComponent(_clusterContents, false);
    //  vp->setSize(_clusterContents->getWidth() + vp->getScrollBarThickness(), 300);

    //  CallOutBox& cb = CallOutBox::launchAsynchronously(vp, getScreenBounds(), nullptr);
    //}
  }
}

void SearchResultContainer::mouseEnter(const MouseEvent & event)
{
  _isHovered = true;
  getGlobalSettings()->_showThumbnailImg = true;
  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());
  mc->setThumbImage(_render);
  mc->repaint();
  _clusterContents->repaint();
  getParentComponent()->repaint();
}

void SearchResultContainer::mouseExit(const MouseEvent & event)
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
    return _clusterContents->size();
  }
}

void SearchResultContainer::addToCluster(shared_ptr<SearchResultContainer> elem)
{
  _clusterContents->addResult(elem);

}

Eigen::Vector3d SearchResultContainer::rgbToLab(double r, double g, double b)
{
  Eigen::Vector3d xyz = ColorUtils::convRGBtoXYZ(r, g, b, sRGB);
  return ColorUtils::convXYZtoLab(xyz, refWhites[D65] / 100.0);
}

double SearchResultContainer::dist(SearchResultContainer * y)
{
  string metric = getGlobalSettings()->_distMetric;
  if (metric == "Per-Pixel Average Lab Difference") {
    return avgPixDist(y);
  }
  else if (metric == "Per-Pixel Maximum Lab Difference") {
    return maxPixDist(y);
  }
  else if (metric == "Per-Pixel 90th Percentile Difference") {
    return pctPixDist(y);
  }
  else if (metric == "Lab L2 Norm") {
    return l2dist(y);
  }
  else if (metric == "Parameter L2 Norm") {
    return l2paramDist(y);
  }
  else if (metric == "Softmax Parameter L2 Norm") {
    return l2paramDistSoftmax(y);
  }
  else if (metric == "Luminance L2 Norm") {
    return l2LuminanceDist(y);
  }
  else if (metric == "Attribute Function Distance") {
    return attrDist(y);
  }
  else {
    // default to Per-Pixel average if no mode specified.
    return avgPixDist(y);
  }
}

double SearchResultContainer::avgPixDist(SearchResultContainer * y, bool overrideMask)
{
  double sum = 0;
  Eigen::VectorXd fy = y->getFeatures();

  // iterate through pixels in groups of 3
  for (int i = 0; i < _features.size() / 3; i++) {
    int idx = i * 3;

    double maskFactor = 1;
    if (!overrideMask && getGlobalSettings()->_useFGMask) {
      maskFactor = _mask.getPixelAt(i % 64, (int)(i / 64)).getBrightness();
    }

    sum += maskFactor * sqrt(pow(fy[idx] - _features[idx], 2) +
      pow(fy[idx + 1] - _features[idx + 1], 2) +
      pow(fy[idx + 2] - _features[idx + 2], 2));
  }

  return sum / (_features.size() / 3);
}

double SearchResultContainer::l2dist(SearchResultContainer * y)
{
  double sum = 0;
  Eigen::VectorXd fy = y->getFeatures();

  // iterate through pixels in groups of 3
  for (int i = 0; i < _features.size() / 3; i++) {
    int idx = i * 3;

    double maskFactor = 1;
    if (getGlobalSettings()->_useFGMask) {
      maskFactor = _mask.getPixelAt(i % 64, (int)(i / 64)).getBrightness();
    }

    sum += maskFactor * (pow(fy[idx] - _features[idx], 2) +
      pow(fy[idx + 1] - _features[idx + 1], 2) +
      pow(fy[idx + 2] - _features[idx + 2], 2));
  }

  return sqrt(sum);
}

double SearchResultContainer::maxPixDist(SearchResultContainer * y)
{
  double maxDist = 0;
  Eigen::VectorXd fy = y->getFeatures();

  // iterate through pixels in groups of 3
  for (int i = 0; i < _features.size() / 3; i++) {
    int idx = i * 3;

    double maskFactor = 1;
    if (getGlobalSettings()->_useFGMask) {
      maskFactor = _mask.getPixelAt(i % 64, (int)(i / 64)).getBrightness();
    }

    double dist = maskFactor * sqrt(pow(fy[idx] - _features[idx], 2) +
      pow(fy[idx + 1] - _features[idx + 1], 2) +
      pow(fy[idx + 2] - _features[idx + 2], 2));

    if (dist > maxDist)
      maxDist = dist;
  }

  return maxDist;
}

double SearchResultContainer::pctPixDist(SearchResultContainer * y)
{
  Array<double> dists;
  Eigen::VectorXd fy = y->getFeatures();

  // iterate through pixels in groups of 3
  for (int i = 0; i < _features.size() / 3; i++) {
    int idx = i * 3;

    double maskFactor = 1;
    if (getGlobalSettings()->_useFGMask) {
      maskFactor = _mask.getPixelAt(i % 64, (int)(i / 64)).getBrightness();
    }

    double dist = maskFactor * sqrt(pow(fy[idx] - _features[idx], 2) +
      pow(fy[idx + 1] - _features[idx + 1], 2) +
      pow(fy[idx + 2] - _features[idx + 2], 2));

    dists.add(dist);
  }

  dists.sort();

  // pick point around 90% mark
  int pct = .9 * dists.size();

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

    Eigen::Vector3d xc = xcolor->getRGB() * xintens;
    Eigen::Vector3d yc = ycolor->getRGB() * yintens;

    sum += (xc - yc).norm();
  }

  int count = xdevices.size();

  delete ys;
  delete xs;
  return sum / count;
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
  for (int i = 0; i < xparams.size(); i++) {
    xparams[i] = pow(xparams[i], xparams.size());
    yparams[i] = pow(yparams[i], yparams.size());
  }

  xparams /= xparams.sum();
  yparams /= yparams.sum();

  delete ys;
  delete xs;

  return (xparams - yparams).norm();
}

double SearchResultContainer::l2LuminanceDist(SearchResultContainer * y)
{
  // just pull the luminance component of each image and take the L2 norm
  double sum = 0;
  Eigen::VectorXd yf = y->getFeatures();

  for (int i = 0; i < _features.size() / 3; i++) {
    double maskFactor = 1;
    if (getGlobalSettings()->_useFGMask) {
      maskFactor = _mask.getPixelAt(i % 64, (int)(i / 64)).getBrightness();
    }

    sum += maskFactor * pow(_features[i * 3] - yf[i * 3], 2);
  }

  return sqrt(sum);
}

double SearchResultContainer::attrDist(SearchResultContainer * y)
{
  return abs(_result->_objFuncVal - y->getSearchResult()->_objFuncVal);
}
