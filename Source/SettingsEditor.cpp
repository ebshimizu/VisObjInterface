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
  else if (_id == "Finite Difference Window")
    getGlobalSettings()->_searchDerivDelta = newValue;
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
  else if (_id == "Initial Edit Depth")
    getGlobalSettings()->_editDepth = (int)newValue;
  else if (_id == "Cluster Distance Threshold")
    getGlobalSettings()->_clusterDistThreshold = newValue;
  else if (_id == "MCMC Step Size")
    getGlobalSettings()->_editStepSize = newValue;
  else if (_id == "MCMC Max Iterations")
    getGlobalSettings()->_maxMCMCIters = (int)newValue;
  else if (_id == "JND Threshold")
    getGlobalSettings()->_jndThreshold = newValue;
  else if (_id == "Thumbnails Per Row")
    getGlobalSettings()->_clusterElemsPerRow = (int)newValue;
#ifdef USE_ARNOLD
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
#endif
  else if (_id == "Max Results")
    getGlobalSettings()->_maxReturnedScenes = (int)newValue;
  else if (_id == "Temperature")
    getGlobalSettings()->_T = newValue;
  else if (_id == "Mean Shift Bandwidth")
    getGlobalSettings()->_meanShiftBandwidth = newValue;
  else if (_id == "Mean Shift Epsilon")
    getGlobalSettings()->_meanShiftEps = newValue;
  else if (_id == "Search Threads")
    getGlobalSettings()->_searchThreads = (int)newValue;
  else if (_id == "Number of Clusters")
    getGlobalSettings()->_numClusters = (int)newValue;
  else if (_id == "Spectral Bandwidth")
    getGlobalSettings()->_spectralBandwidth = newValue;
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
  else if (_id == "Finite Difference Window")
    return getGlobalSettings()->_searchDerivDelta;
  else if (_id == "Frame Width")
    return getGlobalSettings()->_renderWidth;
  else if (_id == "Frame Height")
    return getGlobalSettings()->_renderHeight;
  else if (_id == "Thumbnail Scale")
    return getGlobalSettings()->_thumbnailRenderScale;
  else if (_id == "Initial Edit Depth")
    return getGlobalSettings()->_editDepth;
  else if (_id == "Cluster Distance Threshold")
    return getGlobalSettings()->_clusterDistThreshold;
  else if (_id == "MCMC Step Size")
    return getGlobalSettings()->_editStepSize;
  else if (_id == "MCMC Max Iterations")
    return getGlobalSettings()->_maxMCMCIters;
  else if (_id == "JND Threshold")
    return getGlobalSettings()->_jndThreshold;
  else if (_id == "Thumbnails Per Row")
    return getGlobalSettings()->_clusterElemsPerRow;
#ifdef USE_ARNOLD
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
#endif
  else if (_id == "Max Results")
    return getGlobalSettings()->_maxReturnedScenes;
  else if (_id == "Temperature")
    return getGlobalSettings()->_T;
  else if (_id == "Mean Shift Bandwidth")
    return getGlobalSettings()->_meanShiftBandwidth;
  else if (_id == "Mean Shift Epsilon")
    return getGlobalSettings()->_meanShiftEps;
  else if (_id == "Search Threads")
    return getGlobalSettings()->_searchThreads;
  else if (_id == "Number of Clusters")
    return getGlobalSettings()->_numClusters;
  else if (_id == "Spectral Bandwidth")
    return getGlobalSettings()->_spectralBandwidth;

  return 0;
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
  if (_id == "Exhaustive Search")
    getGlobalSettings()->_standardMCMC = newState;
  else if (_id == "Export Traces")
    getGlobalSettings()->_exportTraces = newState;
  else if (_id == "Grayscale Mode")
    getGlobalSettings()->_grayscaleMode = newState;
  else if (_id == "Generate Graph")
    getGlobalSettings()->_autoRunTraceGraph = newState;
  else if (_id == "Use Mask") {
    if (getGlobalSettings()->_fgMask.getWidth() != 0) {
      getGlobalSettings()->_useFGMask = newState;
    }
  }

  refresh();
}

bool SettingsBoolButton::getState() const
{
  if (_id == "Exhaustive Search")
    return getGlobalSettings()->_standardMCMC;
  else if (_id == "Export Traces")
    return getGlobalSettings()->_exportTraces;
  else if (_id == "Grayscale Mode")
    return getGlobalSettings()->_grayscaleMode;
  else if (_id == "Generate Graph")
    return getGlobalSettings()->_autoRunTraceGraph;
  else if (_id == "Use Mask") {
    return getGlobalSettings()->_useFGMask;
  }

  return false;
}

SettingsChoice::SettingsChoice(string name, StringArray choices) :
  ChoicePropertyComponent(name)
{
  for (auto& c : choices) {
    this->choices.add(c);
  }
}

SettingsChoice::~SettingsChoice()
{
}

int SettingsChoice::getIndex() const
{
  if (getName() == "Clustering Method") {
    string mode = getGlobalSettings()->_clusterMethodName;
    return choices.indexOf(String(mode));
  }
  else if (getName() == "Distance Metric") {
    string metric = getGlobalSettings()->_distMetric;
    return choices.indexOf(String(metric));
  }
}

void SettingsChoice::setIndex(int newIndex)
{
  if (getName() == "Clustering Method") {
    getGlobalSettings()->_clusterMethodName = choices[newIndex].toStdString();
  }
  else if (getName() == "Distance Metric") {
    getGlobalSettings()->_distMetric = choices[newIndex].toStdString();
  }
}


//==============================================================================
SettingsEditor::SettingsEditor()
{
  Array<PropertyComponent*> searchComponents;
  searchComponents.add(new SettingsSlider("Initial Edit Depth", 1, 25, 1));
  searchComponents.add(new SettingsSlider("JND Threshold", 0.01, 10, 0.01));
  searchComponents.add(new SettingsSlider("Max Results", 1, 500, 1));
  searchComponents.add(new SettingsSlider("MCMC Step Size", 0, 0.25, 0.001));
  searchComponents.add(new SettingsSlider("MCMC Max Iterations", 1, 100, 1));
  //searchComponents.add(new SettingsSlider("Mean Shift Epsilon", 0, 0.01, 1e-6));
  searchComponents.add(new SettingsSlider("Temperature", 0, 25, 0.01));
  //searchComponents.add(new SettingsSlider("Finite Difference Window", 1e-7, 1e-3, 1e-7));
  searchComponents.add(new SettingsBoolButton("Exhaustive Search"));
  searchComponents.add(new SettingsBoolButton("Export Traces"));
  searchComponents.add(new SettingsBoolButton("Generate Graph"));
  searchComponents.add(new SettingsSlider("Search Threads", 1, thread::hardware_concurrency(), 1));
  searchComponents.add(new SettingsBoolButton("Use Mask"));
  _settings.addSection("Search", searchComponents);


  Array<PropertyComponent*> clusterComponents;
  clusterComponents.add(new SettingsChoice("Clustering Method", { "K-Means", "Mean Shift", "Spectral Clustering" }));
  clusterComponents.add(new SettingsChoice("Distance Metric", { "Per-Pixel Average Lab Difference",
    "Per-Pixel Maximum Lab Difference", "Per-Pixel 90th Percentile Difference", "Lab L2 Norm", "Luminance L2 Norm",
    "Parameter L2 Norm", "Softmax Parameter L2 Norm" })); //, "Whitened Parameter L2 Norm"
  clusterComponents.add(new SettingsSlider("Number of Clusters", 1, 25, 1));
  clusterComponents.add(new SettingsSlider("Mean Shift Bandwidth", 0, 20, 0.01));
  clusterComponents.add(new SettingsSlider("Spectral Bandwidth", 1e-3, 5, 0.001));
  _settings.addSection("Clustering", clusterComponents);

  Array<PropertyComponent*> renderComponents;
  //_width = new SettingsSlider("Frame Width", 1, 3840, 1);
  //_height = new SettingsSlider("Frame Height", 1, 2160, 1);
  //_width->_other = _height;
  //_height->_other = _width;
  renderComponents.add(new SettingsBoolButton("Grayscale Mode"));

#ifdef USE_ARNOLD
  renderComponents.add(new SettingsSlider("Render Samples", -3, 8, 1));
  renderComponents.add(new SettingsSlider("Thumbnail Render Samples", -3, 8, 1));
  //renderComponents.add(_width);
  //renderComponents.add(_height);
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
#endif

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