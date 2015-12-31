/*
  ==============================================================================

    SettingsEditor.cpp
    Created: 31 Dec 2015 12:50:03pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SettingsEditor.h"


SettingsSlider::SettingsSlider(string id, double min, double max, double step) :
  SliderPropertyComponent(id, min, max, step), _id(id)
{
  slider.setName(id);
  slider.setValue(getValue());
}

SettingsSlider::~SettingsSlider()
{
}

void SettingsSlider::setValue(double newValue)
{
  if (_id == "Render Samples") {
    ArnoldAnimationPatch* p = getAnimationPatch();

    if (p == nullptr)
      return;

    p->setSamples((int)newValue);
  }
  else if (_id == "Thumbnail Render Samples") {
    getGlobalSettings()->_thumbnailRenderSamples = (int)newValue;
  }
}

double SettingsSlider::getValue() const
{
  if (_id == "Render Samples") {
    ArnoldAnimationPatch* p = getAnimationPatch();

    if (p == nullptr)
      return 0;

    return p->getSamples();
  }
  else if (_id == "Thumbnail Render Samples") {
    return getGlobalSettings()->_thumbnailRenderSamples;
  }
}

void SettingsSlider::sliderValueChanged(Slider * s)
{
  SliderPropertyComponent::sliderValueChanged(s);

  getRecorder()->log(SYSTEM, "Setting " + _id + " set to " + String(s->getValue()).toStdString());
}


//==============================================================================
SettingsEditor::SettingsEditor()
{
  Array<PropertyComponent*> renderComponents;
  renderComponents.add(new SettingsSlider("Render Samples", -3, 8, 1));
  renderComponents.add(new SettingsSlider("Thumbnail Render Samples", -3, 8, 1));

  _settings.addSection("Render", renderComponents);

  addAndMakeVisible(_settings);
}

SettingsEditor::~SettingsEditor()
{

}

void SettingsEditor::paint (Graphics& g)
{

}

void SettingsEditor::resized()
{
  _settings.setBounds(getLocalBounds());
}

SettingsWindow::SettingsWindow() :
  DocumentWindow("Settings", Colour(0xff333333), TitleBarButtons::closeButton, true)
{
  setContentOwned(_settingsEditor = new SettingsEditor(), false);
}

SettingsWindow::~SettingsWindow()
{
  _settingsEditor = nullptr;
}

void SettingsWindow::resized()
{
  _settingsEditor->setBounds(getLocalBounds());
}

void SettingsWindow::closeButtonPressed()
{
  delete this;
}
