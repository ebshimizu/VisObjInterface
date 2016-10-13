/*
  ==============================================================================

    AttributeControllerBase.cpp
    Created: 4 Jan 2016 1:50:03pm
    Author:  Evan

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

//==============================================================================
AttributeControllerBase::AttributeControllerBase(String name, int w, int h) : _name(name),
  _canonicalHeight(w), _canonicalWidth(h)
{
  // set juce component name
  setName(_name);

  _buttons.add(new TextButton("Ignore", "Do not use this attribute in the lighting search."));
  _buttons.add(new TextButton("Less", "Reduce the amount of this attribute when searching."));
  _buttons.add(new TextButton("Same", "Keep the amount of this attribute roughtly the same when searching."));
  _buttons.add(new TextButton("More", "Increase the amount of this attribute when searching"));
  //_buttons.add(new TextButton("Explore", "Show a spread of options from this attribute."));

  for (int i = 0; i < 4; i++) {
    _buttons[i]->setRadioGroupId(1);
    _buttons[i]->setClickingTogglesState(true);
    _buttons[i]->setColour(TextButton::buttonColourId, Colours::grey);
    _buttons[i]->setColour(TextButton::buttonOnColourId, Colours::deepskyblue);
    _buttons[i]->addListener(this);
    addAndMakeVisible(_buttons[i]);
  }

  _buttons[0]->setToggleState(true, dontSendNotification);
  _status = A_IGNORE;
}

AttributeControllerBase::~AttributeControllerBase()
{
  for (TextButton* b : _buttons) {
    delete b;
  }
}

void AttributeControllerBase::paint (Graphics& g)
{
  auto lbounds = getLocalBounds();
  auto top = lbounds.removeFromTop(24);
  top.reduce(2, 2);

  g.setColour(Colours::white);
  g.drawFittedText(_name, top, Justification::left, 1);

  if (!getGlobalSettings()->_cacheUpdated) {
    getGlobalSettings()->updateCache();
  }

  Image img = getGlobalSettings()->_renderCache.rescaled(_canonicalWidth, _canonicalHeight);

  Snapshot* s = new Snapshot(getRig());
  double attrVal = evaluateScene(s, img);
  delete s;
  
  g.drawFittedText(String(attrVal), top, Justification::right, 1);
}

void AttributeControllerBase::resized()
{
  auto lbounds = getLocalBounds();

  // top area for drawing
  lbounds.removeFromTop(24);
  
  int buttonWidth = lbounds.getWidth() / _buttons.size();
  for (int i = 0; i < _buttons.size(); i++) {
    _buttons[i]->setBounds(lbounds.removeFromLeft(buttonWidth).reduced(2));
  }

}

AttributeConstraint AttributeControllerBase::getStatus()
{
  return _status;
}

void AttributeControllerBase::setStatus(AttributeConstraint c)
{
  _status = c;
}

void AttributeControllerBase::buttonClicked(Button * b)
{
  if (!b->getToggleState())
    return;

  String buttonName = b->getName();
  getRecorder()->log(ACTION, "Attribute " + _name.toStdString() + " set to " + buttonName.toStdString());

  if (buttonName == "Ignore") {
    _status = A_IGNORE;
  }
  if (buttonName == "Less") {
    _status = A_LESS;
  }
  if (buttonName == "Same") {
    _status = A_EQUAL;
  }
  if (buttonName == "More") {
    _status = A_MORE;
  }
  if (buttonName == "Explore") {
    _status = A_EXPLORE;
  }
}

Image AttributeControllerBase::generateImage(Snapshot * s)
{
  auto devices = s->getDevices();
  auto p = getAnimationPatch();

  if (p == nullptr) {
    return Image(Image::ARGB, _canonicalWidth, _canonicalHeight, true);
  }

  // with caching we can render at full and then scale down
  Image highRes = Image(Image::ARGB, _canonicalWidth, _canonicalHeight, true);
  uint8* bufptr = Image::BitmapData(highRes, Image::BitmapData::readWrite).getPixelPointer(0, 0);
  p->setDims(_canonicalWidth, _canonicalHeight);
  p->setSamples(getGlobalSettings()->_thumbnailRenderSamples);

  getAnimationPatch()->renderSingleFrameToBuffer(devices, bufptr, _canonicalWidth, _canonicalHeight);

  // bit obsolete now
  // if the focus region has a non-zero width, pull out the proper section of the image
  //auto rect = getGlobalSettings()->_focusBounds;
  //if (rect.getWidth() > 0) {
    // pull subsection
  //  auto topLeft = rect.getTopLeft();
  //  auto botRight = rect.getBottomRight();
  //  Image clipped = highRes.getClippedImage(Rectangle<int>::leftTopRightBottom(
  //    (int)(topLeft.x * _canonicalWidth * 2), (int)(topLeft.y * _canonicalHeight * 2),
  //    (int)(botRight.x * _canonicalWidth * 2), (int)(botRight.y * _canonicalHeight * 2)
  //  ));

  //  canonical = clipped.rescaled(_canonicalWidth, _canonicalHeight);
  //}
  //else {
  // canonical = highRes.rescaled(_canonicalWidth, _canonicalHeight);
  //}

  return highRes;
}

Image AttributeControllerBase::generateImage(Snapshot * s, int w, int h)
{
  auto devices = s->getDevices();
  auto p = getAnimationPatch();

  if (p == nullptr) {
    return Image(Image::ARGB, w, h, true);
  }

  Image highRes = Image(Image::ARGB, w, h, true);
  uint8* bufptr = Image::BitmapData(highRes, Image::BitmapData::readWrite).getPixelPointer(0, 0);
  p->setDims(w, h);
  p->setSamples(getGlobalSettings()->_thumbnailRenderSamples);

  getAnimationPatch()->renderSingleFrameToBuffer(devices, bufptr, w, h);

  return highRes;
}
