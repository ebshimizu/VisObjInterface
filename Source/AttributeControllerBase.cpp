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
AttributeControllerBase::AttributeControllerBase(String name) : _name(name)
{
  // set juce component name
  setName(_name);

  _buttons.add(new TextButton("Ignore", "Do not use this attribute in the lighting search."));
  _buttons.add(new TextButton("Less", "Reduce the amount of this attribute when searching."));
  _buttons.add(new TextButton("Same", "Keep the amount of this attribute roughtly the same when searching."));
  _buttons.add(new TextButton("More", "Increase the amount of this attribute when searching"));

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
  auto bounds = getLocalBounds();
  auto top = bounds.removeFromTop(24);
  top.reduce(2, 2);

  g.setColour(Colours::white);
  g.drawFittedText(_name, top, Justification::left, 1);

  double attrVal = evaluateScene(getRig()->getDeviceRaw());
  g.drawFittedText(String(attrVal), top, Justification::right, 1);
}

void AttributeControllerBase::resized()
{
  auto bounds = getLocalBounds();

  // top area for drawing
  bounds.removeFromTop(24);
  
  int buttonWidth = bounds.getWidth() / 4;
  for (int i = 0; i < 4; i++) {
    _buttons[i]->setBounds(bounds.removeFromLeft(buttonWidth).reduced(2));
  }

}

AttributeConstraint AttributeControllerBase::getStatus()
{
  return _status;
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
}
