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
  SliderPropertyComponent(id, min, max, step), _id(id), _other(nullptr)
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

    getGlobalSettings()->_stageRenderSamples = (int)newValue;
    p->setSamples(getGlobalSettings()->_stageRenderSamples);
  }
  else if (_id == "Thumbnail Render Samples")
    getGlobalSettings()->_thumbnailRenderSamples = (int)newValue;
  else if (_id == "Minimum Edit Distance")
    getGlobalSettings()->_minEditDist = newValue;
  else if (_id == "Scenes per Edit")
    getGlobalSettings()->_numEditScenes = (int)newValue;
  else if (_id == "Finite Difference Window")
    getGlobalSettings()->_searchDerivDelta = newValue;
  else if (_id == "GD - Tolerance")
    getGlobalSettings()->_searchGDTol = newValue;
  else if (_id == "GD - Gamma")
    getGlobalSettings()->_searchGDGamma = newValue;
  else if (_id == "Frame Width") {
    getGlobalSettings()->_renderWidth = (int)newValue;
    // Maintain aspect ratio
    getGlobalSettings()->_renderHeight = (int)(newValue * (9.0 / 16.0));
    _other->refresh();
  }
  else if (_id == "Frame Height") {
    getGlobalSettings()->_renderHeight = (int)newValue;
    // Maintain aspect ratio
    getGlobalSettings()->_renderWidth = (int)(newValue * (16.0 / 9.0));
    _other->refresh();
  }
  else if (_id == "Thumbnail Scale")
    getGlobalSettings()->_thumbnailRenderScale = newValue;
  else if (_id == "Edit Depth")
    getGlobalSettings()->_editDepth = (int)newValue;
}

double SettingsSlider::getValue() const
{
  if (_id == "Render Samples") {
    ArnoldAnimationPatch* p = getAnimationPatch();

    if (p == nullptr)
      return 0;

    return p->getSamples();
  }
  else if (_id == "Thumbnail Render Samples")
    return getGlobalSettings()->_thumbnailRenderSamples;
  else if (_id == "Minimum Edit Distance")
    return getGlobalSettings()->_minEditDist;
  else if (_id == "Scenes per Edit")
    return getGlobalSettings()->_numEditScenes;
  else if (_id == "Finite Difference Window")
    return getGlobalSettings()->_searchDerivDelta;
  else if (_id == "GD - Tolerance")
    return getGlobalSettings()->_searchGDTol;
  else if (_id == "GD - Gamma")
    return getGlobalSettings()->_searchGDGamma;
  else if (_id == "Frame Width")
    return getGlobalSettings()->_renderWidth;
  else if (_id == "Frame Height")
    return getGlobalSettings()->_renderHeight;
  else if (_id == "Thumbnail Scale")
    return getGlobalSettings()->_thumbnailRenderScale;
  else if (_id == "Edit Depth")
    return getGlobalSettings()->_editDepth;
}

void SettingsSlider::sliderValueChanged(Slider * s)
{
  SliderPropertyComponent::sliderValueChanged(s);
  
  getRecorder()->log(SYSTEM, "Setting " + _id + " set to " + String(s->getValue()).toStdString());
}


//==============================================================================
SettingsEditor::SettingsEditor()
{
  Array<PropertyComponent*> searchComponents;
  searchComponents.add(new SettingsSlider("Edit Depth", 1, 10, 1));
  searchComponents.add(new SettingsSlider("Minimum Edit Distance", 0, 100, 0.01));
  searchComponents.add(new SettingsSlider("Scenes per Edit", 1, 100, 1));
  searchComponents.add(new SettingsSlider("Finite Difference Window", 1e-7, 1, 1e-7));
  searchComponents.add(new SettingsSlider("GD - Tolerance", 0, 1e-2, 1e-7));
  searchComponents.add(new SettingsSlider("GD - Gamma", 0, 25, 1e-3));
  _settings.addSection("Search", searchComponents);

  Array<PropertyComponent*> renderComponents;
  _width = new SettingsSlider("Frame Width", 1, 3840, 1);
  _height = new SettingsSlider("Frame Height", 1, 2160, 1);
  _width->_other = _height;
  _height->_other = _width;

  renderComponents.add(new SettingsSlider("Render Samples", -3, 8, 1));
  renderComponents.add(new SettingsSlider("Thumbnail Render Samples", -3, 8, 1));
  renderComponents.add(_width);
  renderComponents.add(_height);
  renderComponents.add(new SettingsSlider("Thumbnail Scale", 0, 1, 0.01));

  _settings.addSection("Render", renderComponents);

  addAndMakeVisible(_settings);
}

SettingsEditor::~SettingsEditor()
{

}

void SettingsEditor::paint (Graphics& g)
{
  g.fillAll(Colour(0xff929292));
}

void SettingsEditor::resized()
{
  _settings.setBounds(getLocalBounds());
}

void SettingsEditor::updateDims()
{
  _width->refresh();
  _height->refresh();
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

void SettingsWindow::updateDims()
{
  _settingsEditor->updateDims();
}
