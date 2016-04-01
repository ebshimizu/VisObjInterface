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
  else if (_id == "Number of Scenes Between Layers")
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
  else if (_id == "Cluster Distance Threshold")
    getGlobalSettings()->_clusterDistThreshold = newValue;
  else if (_id == "Result Difference Threshold")
    getGlobalSettings()->_clusterDiffThreshold = newValue;
  else if (_id == "Max Iterations")
    getGlobalSettings()->_maxEditIters = (int)newValue;
  else if (_id == "GB - Momentum")
    getGlobalSettings()->_searchMomentum = newValue;
  else if (_id == "MCMC Step Size")
    getGlobalSettings()->_editStepSize = newValue;
  else if (_id == "MCMC Max Iterations")
    getGlobalSettings()->_maxMCMCIters = (int)newValue;
  else if (_id == "Number of Clusters")
    getGlobalSettings()->_numDisplayClusters = (int)newValue;
  else if (_id == "JND Threshold")
    getGlobalSettings()->_jndThreshold = newValue;
  else if (_id == "Thumbnails Per Row")
    getGlobalSettings()->_clusterElemsPerRow = (int)newValue;
  else if (_id == "Diffuse Depth")
    getAnimationPatch()->getArnoldInterface()->setOptionParameter("GI_diffuse_depth", (int)newValue);
  else if (_id == "Glossy Depth")
    getAnimationPatch()->getArnoldInterface()->setOptionParameter("GI_glossy_depth", (int)newValue);
  else if (_id == "Reflection Depth")
    getAnimationPatch()->getArnoldInterface()->setOptionParameter("GI_reflection_depth", (int)newValue);
  else if (_id == "Refraction Depth")
    getAnimationPatch()->getArnoldInterface()->setOptionParameter("GI_refraction_depth", (int)newValue);
  else if (_id == "Volume Depth")
    getAnimationPatch()->getArnoldInterface()->setOptionParameter("GI_volume_depth", (int)newValue);
  else if (_id == "Total Depth")
    getAnimationPatch()->getArnoldInterface()->setOptionParameter("GI_total_depth", (int)newValue);
  else if (_id == "Diffuse Samples")
    getAnimationPatch()->getArnoldInterface()->setOptionParameter("GI_diffuse_samples", (int)newValue);
  else if (_id == "Glossy Samples")
    getAnimationPatch()->getArnoldInterface()->setOptionParameter("GI_glossy_samples", (int)newValue);
  else if (_id == "Refraction Samples")
    getAnimationPatch()->getArnoldInterface()->setOptionParameter("GI_refraction_samples", (int)newValue);
  else if (_id == "Reflection Samples")
    getAnimationPatch()->getArnoldInterface()->setOptionParameter("GI_reflection_samples", (int)newValue);
  else if (_id == "SSS Samples")
    getAnimationPatch()->getArnoldInterface()->setOptionParameter("GI_sss_samples", (int)newValue);
  else if (_id == "Volume Samples")
    getAnimationPatch()->getArnoldInterface()->setOptionParameter("GI_volume_samples", (int)newValue);
  else if (_id == "Accept Bandwidth")
    getGlobalSettings()->_accceptBandwidth = newValue;
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
  else if (_id == "Number of Scenes Between Layers")
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
  else if (_id == "Cluster Distance Threshold")
    return getGlobalSettings()->_clusterDistThreshold;
  else if (_id == "Result Difference Threshold")
    return getGlobalSettings()->_clusterDiffThreshold;
  else if (_id == "Max Iterations")
    return getGlobalSettings()->_maxEditIters;
  else if (_id == "GD - Momentum")
    return getGlobalSettings()->_searchMomentum;
  else if (_id == "MCMC Step Size")
    return getGlobalSettings()->_editStepSize;
  else if (_id == "MCMC Max Iterations")
    return getGlobalSettings()->_maxMCMCIters;
  else if (_id == "Number of Clusters")
    return getGlobalSettings()->_numDisplayClusters;
  else if (_id == "JND Threshold")
    return getGlobalSettings()->_jndThreshold;
  else if (_id == "Thumbnails Per Row")
    return getGlobalSettings()->_clusterElemsPerRow;
  else if (_id == "Diffuse Depth")
    return getAnimationPatch()->getArnoldInterface()->getOptionParameter("GI_diffuse_depth");
  else if (_id == "Glossy Depth")
    return getAnimationPatch()->getArnoldInterface()->getOptionParameter("GI_glossy_depth");
  else if (_id == "Reflection Depth")
    return getAnimationPatch()->getArnoldInterface()->getOptionParameter("GI_reflection_depth");
  else if (_id == "Refraction Depth")
    return getAnimationPatch()->getArnoldInterface()->getOptionParameter("GI_refraction_depth");
  else if (_id == "Volume Depth")
    return getAnimationPatch()->getArnoldInterface()->getOptionParameter("GI_volume_depth");
  else if (_id == "Total Depth")
    return getAnimationPatch()->getArnoldInterface()->getOptionParameter("GI_total_depth");
  else if (_id == "Diffuse Samples")
    return getAnimationPatch()->getArnoldInterface()->getOptionParameter("GI_diffuse_samples");
  else if (_id == "Glossy Samples")
    return getAnimationPatch()->getArnoldInterface()->getOptionParameter("GI_glossy_samples");
  else if (_id == "Refraction Samples")
    return getAnimationPatch()->getArnoldInterface()->getOptionParameter("GI_refraction_samples");
  else if (_id == "Reflection Samples")
    return getAnimationPatch()->getArnoldInterface()->getOptionParameter("GI_reflection_samples");
  else if (_id == "SSS Samples")
    return getAnimationPatch()->getArnoldInterface()->getOptionParameter("GI_sss_samples");
  else if (_id == "Volume Samples")
    return getAnimationPatch()->getArnoldInterface()->getOptionParameter("GI_volume_samples");
  else if (_id == "Accept Bandwidth")
    return getGlobalSettings()->_accceptBandwidth;
}

void SettingsSlider::sliderValueChanged(Slider * s)
{
  SliderPropertyComponent::sliderValueChanged(s);
  
  getRecorder()->log(SYSTEM, _id + " set to " + String(s->getValue()).toStdString());
}

SettingsBoolButton::SettingsBoolButton(string id) : 
  BooleanPropertyComponent(id, "On", "Off"), _id(id)
{
}

SettingsBoolButton::~SettingsBoolButton()
{
}

void SettingsBoolButton::setState(bool newState)
{
  if (_id == "Random Mode")
    getGlobalSettings()->_randomMode = newState;

  refresh();
}

bool SettingsBoolButton::getState() const
{
  if (_id == "Random Mode")
    return getGlobalSettings()->_randomMode;

  return false;
}

//==============================================================================
SettingsEditor::SettingsEditor()
{
  Array<PropertyComponent*> searchComponents;
  searchComponents.add(new SettingsSlider("Number of Scenes Between Layers", 1, 100, 1));
  searchComponents.add(new SettingsSlider("Edit Depth", 1, 25, 1));
  searchComponents.add(new SettingsSlider("Minimum Edit Distance", 0, 100, 0.01));
  searchComponents.add(new SettingsSlider("JND Threshold", 0.01, 5, 0.01));
  searchComponents.add(new SettingsSlider("Scenes per Edit", -1, 100, 1));
  searchComponents.add(new SettingsSlider("Accept Bandwidth", 0.001, 0.5, 0.001));
  searchComponents.add(new SettingsSlider("Finite Difference Window", 1e-7, 1e-3, 1e-7));
  searchComponents.add(new SettingsSlider("Cluster Distance Threshold", 1e-3, 5, 1e-3));
  searchComponents.add(new SettingsSlider("Result Difference Threshold", 1e-3, 5, 1e-3));
  searchComponents.add(new SettingsBoolButton("Random Mode"));
  _settings.addSection("Search Shared", searchComponents);

  Array<PropertyComponent*> mcmcComponents;
  mcmcComponents.add(new SettingsSlider("MCMC Step Size", 0, 0.1, 0.001));
  mcmcComponents.add(new SettingsSlider("MCMC Max Iterations", 1, 10000, 1));
  _settings.addSection("MCMC Search", mcmcComponents);

  //Array<PropertyComponent*> gdSearchComponents;
  //gdSearchComponents.add(new SettingsSlider("GD - Tolerance", 0, 1e-2, 1e-7));
  //gdSearchComponents.add(new SettingsSlider("GD - Gamma", 0, 25, 1e-3));  
  //gdSearchComponents.add(new SettingsSlider("GD - Momentum", 1e-3, 1, 1e-3));
  //gdSearchComponents.add(new SettingsSlider("Max Iterations", 1, 10000, 1));
  //_settings.addSection("Gradient Descent Search", gdSearchComponents);

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
  renderComponents.add(new SettingsSlider("Thumbnails Per Row", 1, 20, 1));
  renderComponents.add(new SettingsSlider("Total Depth", 1, 32, 1));
  renderComponents.add(new SettingsSlider("Diffuse Depth", 1, 16, 1));
  renderComponents.add(new SettingsSlider("Glossy Depth", 1, 16, 1));
  renderComponents.add(new SettingsSlider("Reflection Depth", 1, 16, 1));
  renderComponents.add(new SettingsSlider("Refraction Depth", 1, 16, 1));
  renderComponents.add(new SettingsSlider("Volume Depth", 1, 16, 1));
  renderComponents.add(new SettingsSlider("Diffuse Samples", 1, 16, 1));
  renderComponents.add(new SettingsSlider("Glossy Samples", 1, 16, 1));
  renderComponents.add(new SettingsSlider("Refraction Samples", 1, 16, 1));
  renderComponents.add(new SettingsSlider("Reflection Samples", 1, 16, 1));
  renderComponents.add(new SettingsSlider("SSS Samples", 1, 16, 1));
  renderComponents.add(new SettingsSlider("Volume Samples", 1, 16, 1));

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
