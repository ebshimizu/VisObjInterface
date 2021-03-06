/*
  ==============================================================================

    SettingsEditor.cpp
    Created: 31 Dec 2015 12:50:03pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SettingsEditor.h"
#include "MainComponent.h"

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
    getGlobalSettings()->_startChainLength = (int)newValue;
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
  else if (_id == "Number of Primary Clusters")
    getGlobalSettings()->_numPrimaryClusters = (int)newValue;
  else if (_id == "Number of Secondary Clusters")
    getGlobalSettings()->_numSecondaryClusters = (int)newValue;
  else if (_id == "Spectral Bandwidth")
    getGlobalSettings()->_spectralBandwidth = newValue;
  else if (_id == "Primary Divisive Threshold")
    getGlobalSettings()->_primaryDivisiveThreshold = newValue;
  else if (_id == "Secondary Divisive Threshold")
    getGlobalSettings()->_secondaryDivisiveThreshold = newValue;
  else if (_id == "L-M Max Iterations")
    getGlobalSettings()->_maxGradIters = (int)newValue;
  else if (_id == "Expected Value Weight")
    getGlobalSettings()->_evWeight = (float)newValue;
  else if (_id == "Switching Time")
    getGlobalSettings()->_resampleTime = (int)newValue;
  else if (_id == "Movable Threads")
    getGlobalSettings()->_resampleThreads = (int)newValue;
  else if (_id == "Freeze Region Tolerance")
    getGlobalSettings()->_maskTolerance = newValue;
  else if (_id == "Repulsion Cone K")
    getGlobalSettings()->_repulsionConeK = newValue;
  else if (_id == "Repulsion Cost K")
    getGlobalSettings()->_repulsionCostK = newValue;
  else if (_id == "Repulsion Num Pairs")
    getGlobalSettings()->_numPairs = (int)newValue;
  else if (_id == "View JND Threshold")
    getGlobalSettings()->_viewJndThreshold = newValue;
  else if (_id == "JND Threshold Decay Rate")
    getGlobalSettings()->_thresholdDecayRate = newValue;
  else if (_id == "Big Color Bucket Weight")
    getGlobalSettings()->_bigBucketSize = newValue;
  else if (_id == "Exposure") {
    if (getAnimationPatch != nullptr) {
      CachingArnoldInterface* renderer = dynamic_cast<CachingArnoldInterface*>(getAnimationPatch()->getArnoldInterface());
      if (renderer)
        renderer->setExposure((float)newValue);
    }
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
    return getGlobalSettings()->_startChainLength;
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
  else if (_id == "Number of Primary Clusters")
    return getGlobalSettings()->_numPrimaryClusters;
  else if (_id == "Number of Secondary Clusters")
    return getGlobalSettings()->_numSecondaryClusters;
  else if (_id == "Spectral Bandwidth")
    return getGlobalSettings()->_spectralBandwidth;
  else if (_id == "Primary Divisive Threshold")
    return getGlobalSettings()->_primaryDivisiveThreshold;
  else if (_id == "Secondary Divisive Threshold")
    return getGlobalSettings()->_secondaryDivisiveThreshold;
  else if (_id == "L-M Max Iterations")
    return getGlobalSettings()->_maxGradIters;
  else if (_id == "Expected Value Weight")
    return getGlobalSettings()->_evWeight;
  else if (_id == "Switching Time")
    return getGlobalSettings()->_resampleTime;
  else if (_id == "Movable Threads")
    return getGlobalSettings()->_resampleThreads;
  else if (_id == "Freeze Region Tolerance")
    return getGlobalSettings()->_maskTolerance;
  else if (_id == "Repulsion Cone K")
    return getGlobalSettings()->_repulsionConeK;
  else if (_id == "Repulsion Cost K")
    return getGlobalSettings()->_repulsionCostK;
  else if (_id == "Repulsion Num Pairs")
    return getGlobalSettings()->_numPairs;
  else if (_id == "View JND Threshold")
    return getGlobalSettings()->_viewJndThreshold;
  else if (_id == "JND Threshold Decay Rate")
    return getGlobalSettings()->_thresholdDecayRate;
  else if (_id == "Big Color Bucket Weight")
    return getGlobalSettings()->_bigBucketSize;
  else if (_id == "Exposure") {
    if (getAnimationPatch() != nullptr) {
      CachingArnoldInterface* renderer = dynamic_cast<CachingArnoldInterface*>(getAnimationPatch()->getArnoldInterface());
      if (renderer) {
        return renderer->getExposure();
      }
      return 0;
    }
    else {
      return 0;
    }
  }

  return 0;
}

void SettingsSlider::sliderValueChanged(Slider * s)
{
  //Slider::Listener::sliderValueChanged(s);
  
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
  if (_id == "Export Traces")
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
  else if (_id == "Reduce Repeat Edits") {
    getGlobalSettings()->_reduceRepeatEdits = newState;
  }
  else if (_id == "Uniform Edit Weights") {
    getGlobalSettings()->_uniformEditWeights = newState;
  }
  else if (_id == "Randomize Starts") {
    getGlobalSettings()->_randomInit = newState;
  }
  else if (_id == "Continuous Sort") {
    getGlobalSettings()->_continuousSort = newState;
  }
  else if (_id == "Use Styles") {
    getGlobalSettings()->_useSearchStyles = newState;
  }
  else if (_id == "Recompute Color Weights") {
    getGlobalSettings()->_recalculateWeights = newState;
  }
  else if (_id == "Auto Cluster") {
    getGlobalSettings()->_autoCluster = newState;
  }
  else if (_id == "Unconstrained") {
    getGlobalSettings()->_unconstrained = newState;
  }
  else if (_id == "px intens dist") {
    getGlobalSettings()->_pxIntensDist = newState;
  }
  else if (_id == "Disable Pin Adjustment") {
    getGlobalSettings()->_noPinWiggle = newState;
  }

  refresh();
}

bool SettingsBoolButton::getState() const
{
  if (_id == "Export Traces")
    return getGlobalSettings()->_exportTraces;
  else if (_id == "Grayscale Mode")
    return getGlobalSettings()->_grayscaleMode;
  else if (_id == "Generate Graph")
    return getGlobalSettings()->_autoRunTraceGraph;
  else if (_id == "Use Mask")
    return getGlobalSettings()->_useFGMask;
  else if (_id == "Reduce Repeat Edits")
    return getGlobalSettings()->_reduceRepeatEdits;
  else if (_id == "Uniform Edit Weights")
    return getGlobalSettings()->_uniformEditWeights;
  else if (_id == "Randomize Starts")
    return getGlobalSettings()->_randomInit;
  else if (_id == "Continuous Sort")
    return getGlobalSettings()->_continuousSort;
  else if (_id == "Use Styles")
    return getGlobalSettings()->_useSearchStyles;
  else if (_id == "Recompute Color Weights")
    return getGlobalSettings()->_recalculateWeights;
  else if (_id == "Auto Cluster")
    return getGlobalSettings()->_autoCluster;
  else if (_id == "Unconstrained")
    return getGlobalSettings()->_unconstrained;
  else if (_id == "px intens dist")
    return getGlobalSettings()->_pxIntensDist;
  else if (_id == "Disable Pin Adjustment") {
    return getGlobalSettings()->_noPinWiggle;
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
  if (getName() == "Primary Clustering Method") {
    ClusterMethod mode = getGlobalSettings()->_primaryClusterMethod;
    return (int)mode;
  }
  else if (getName() == "Primary Distance Metric") {
    DistanceMetric metric = getGlobalSettings()->_primaryClusterMetric;
    return (int)metric;
  }
  else if (getName() == "Primary Focus Region") {
    FocusArea area = getGlobalSettings()->_primaryFocusArea;
    return (int)area;
  }
  else if (getName() == "Secondary Clustering Method") {
    ClusterMethod mode = getGlobalSettings()->_secondaryClusterMethod;
    return (int)mode;
  }
  else if (getName() == "Secondary Distance Metric") {
    DistanceMetric metric = getGlobalSettings()->_secondaryClusterMetric;
    return (int)metric;
  }
  else if (getName() == "Secondary Focus Region") {
    FocusArea area = getGlobalSettings()->_secondaryFocusArea;
    return (int)area;
  }
	else if (getName() == "Cluster Display Mode") {
		ClusterDisplayMode mode = getGlobalSettings()->_clusterDisplay;
		return (int)mode;
	}
	else if (getName() == "Search Mode") {
		SearchMode mode = getGlobalSettings()->_searchMode;
		return (int)mode;
	}
  else if (getName() == "Edit Selection Mode") {
    EditSelectMode mode = getGlobalSettings()->_editSelectMode;
    return (int)mode;
  }
  else if (getName() == "Search Distance Metric") {
    DistanceMetric metric = getGlobalSettings()->_searchDistMetric;
    return (int)metric;
  }
  else if (getName() == "Search Threshold Metric") {
    DistanceMetric metric = getGlobalSettings()->_searchDispMetric;
    return (int)metric;
  }

  return 0;
}

void SettingsChoice::setIndex(int newIndex)
{
  if (getName() == "Primary Clustering Method") {
    getGlobalSettings()->_primaryClusterMethod = (ClusterMethod)newIndex;

    if (newIndex == (int)STYLE) {
      getGlobalSettings()->_numPrimaryClusters = 4;
    }
  }
  else if (getName() == "Primary Distance Metric") {
    getGlobalSettings()->_primaryClusterMetric = (DistanceMetric)newIndex;
  }
  else if (getName() == "Primary Focus Region") {
    getGlobalSettings()->_primaryFocusArea = (FocusArea)newIndex;
  }
  else if (getName() == "Secondary Clustering Method") {
    getGlobalSettings()->_secondaryClusterMethod = (ClusterMethod)newIndex;

    if (newIndex == (int)STYLE) {
      getGlobalSettings()->_numSecondaryClusters = 4;
    }
  }
  else if (getName() == "Secondary Distance Metric") {
    getGlobalSettings()->_secondaryClusterMetric = (DistanceMetric)newIndex;
  }
  else if (getName() == "Secondary Focus Region") {
    getGlobalSettings()->_secondaryFocusArea = (FocusArea)newIndex;
  }
	else if (getName() == "Cluster Display Mode") {
		ClusterDisplayMode newMode = (ClusterDisplayMode)newIndex;
		if (newMode != getGlobalSettings()->_clusterDisplay) {
			getGlobalSettings()->_clusterDisplay = newMode;
			MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

			if (mc != nullptr) {
				mc->clearClusters();
				mc->resized();
			}
		}
	}
	else if (getName() == "Search Mode") {
		getGlobalSettings()->_searchMode = (SearchMode)newIndex;
	}
  else if (getName() == "Edit Selection Mode") {
    getGlobalSettings()->_editSelectMode = (EditSelectMode)newIndex;
  }
  else if (getName() == "Search Distance Metric") {
    getGlobalSettings()->_searchDistMetric = (DistanceMetric)newIndex;
  }
  else if (getName() == "Search Threshold Metrid") {
    getGlobalSettings()->_searchDispMetric = (DistanceMetric)newIndex;
  }
}


SettingsButton::SettingsButton(string name) : ButtonPropertyComponent(name, false)
{
}

SettingsButton::~SettingsButton()
{
}

void SettingsButton::buttonClicked()
{
	if (getName() == "Log Directory") {
		FileChooser fc("Select Log Directory", File::getCurrentWorkingDirectory(),
			"", true);

		if (fc.browseForDirectory()) {
			File selected = fc.getResult();
			String logPath = selected.getFullPathName();
			getGlobalSettings()->_logRootDir = logPath.toStdString();

			File clusterFolder;
			clusterFolder = clusterFolder.getCurrentWorkingDirectory().getChildFile(String(getGlobalSettings()->_logRootDir + "/clusters/"));
			if (!clusterFolder.exists()) {
				clusterFolder.createDirectory();
			}

			File logFolder;
			logFolder = logFolder.getCurrentWorkingDirectory().getChildFile(String(getGlobalSettings()->_logRootDir + "/traces/"));
			if (!logFolder.exists()) {
				logFolder.createDirectory();
			}

			refresh();
		}
	}
  if (getName() == "OSC Settings") {
    // pop up a dialog, settings shouldn't be edited live
    AlertWindow w("OSC Settings",
      "",
      AlertWindow::NoIcon);

    // gather osc settings, if they exist
    OscPatch* osc = nullptr;
    for (auto p : getRig()->getPatches()) {
      osc = dynamic_cast<OscPatch*>(p.second);
      if (osc != nullptr)
        break;
    }

    String ip = "[No OSC Patch Defined]";
    int port = 8000;
    int inPort = 9000;

    if (osc != nullptr) {
      ip = osc->getAddress();
      port = osc->getPort();
      inPort = osc->getInPort();
    }

    w.addTextEditor("IP Address", ip, "Eos IP Address");
    w.addTextEditor("TX Port", String(port), "TX Port");
    w.addTextEditor("RX Port", String(inPort), "RX Port");

    w.addButton("Update", 1, KeyPress(KeyPress::returnKey, 0, 0));
    w.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));

    if (w.runModalLoop() != 0)
    {
      ip = w.getTextEditor("IP Address")->getText();
      port = w.getTextEditor("TX Port")->getText().getIntValue();
      inPort = w.getTextEditor("RX Port")->getText().getIntValue();

      if (osc != nullptr) {
        // can't change this live
        osc->close();
        osc->changeAddress(ip.toStdString(), port);
        osc->changeInPort(inPort);
        osc->init();
        getStatusBar()->setStatusMessage("Updated OSC settings");
      }
      else {
        osc = new OscPatch(ip.toStdString(), port, ETC_EOS);
        osc->changeInPort(inPort);
        getRig()->addPatch("osc", (Patch*)osc);
        osc->init();
        getStatusBar()->setStatusMessage("Created new OSC output on " + ip + ":" + String(port));
      }

      refresh();
    }
  }
}

String SettingsButton::getButtonText() const
{
	if (getName() == "Log Directory") {
		return getGlobalSettings()->_logRootDir;
	}
  if (getName() == "OSC Settings") {
    if (getRig() == nullptr)
      return "No Rig Loaded";

    // find the osc patch if it exists
    for (auto p : getRig()->getPatches()) {
      OscPatch* osc = dynamic_cast<OscPatch*>(p.second);
      if (osc != nullptr) {
        return "TX:" + String(osc->getAddress()) + ":" + String(osc->getPort());
      }
    }

    return "No OSC Patch Found";
  }

	return "";
}

//==============================================================================
SettingsEditor::SettingsEditor()
{
  Array<PropertyComponent*> searchComponents;
  searchComponents.add(new SettingsChoice("Search Mode", { "MCMC with no inner loop", "MCMC with Random Starting Points",
    "K-Frontier Random Start", "K-Frontier MCMC", "CMA-ES", "K-Frontier MCMC with Repulsion", "Gibbs Sampling" }));
  searchComponents.add(new SettingsChoice("Search Distance Metric", { "Per-Pixel Average Lab Difference",
    "Per-Pixel Maximum Lab Difference", "Per-Pixel 90th Percentile Difference", "Lab L2 Norm", "Luminance L2 Norm",
    "Parameter L2 Norm", "Softmax Parameter L2 Norm", "Attribute Function Distance", "Directed Per-Pixel Average Lab Difference",
    "Per-Pixel Average Gradient Direction Difference", "Selected Key Light Parameter L2 Norm", "Grayscale Paramter L2" }));
  searchComponents.add(new SettingsChoice("Edit Selection Mode", { "Default", "Simple Bandit", "Uniform Random", "Adversarial Bandit",
    "Directed Gibbs Sampling Test" }));
  searchComponents.add(new SettingsSlider("Initial Edit Depth", 1, 250, 1));
  searchComponents.add(new SettingsSlider("JND Threshold", 0.01, 200, 0.01));
  searchComponents.add(new SettingsSlider("JND Threshold Decay Rate", 0.001, 2, 0.001));
  searchComponents.add(new SettingsSlider("View JND Threshold", 0.001, 1, 0.001));
  searchComponents.add(new SettingsChoice("Search Threshold Metric", { "Per-Pixel Average Lab Difference",
    "Per-Pixel Maximum Lab Difference", "Per-Pixel 90th Percentile Difference", "Lab L2 Norm", "Luminance L2 Norm",
    "Parameter L2 Norm", "Softmax Parameter L2 Norm", "Attribute Function Distance", "Directed Per-Pixel Average Lab Difference",
    "Per-Pixel Average Gradient Direction Difference", "Selected Key Light Parameter L2 Norm", "Grayscale Paramter L2" }));
  searchComponents.add(new SettingsSlider("Max Results", 1, 1000, 1));
  searchComponents.add(new SettingsSlider("MCMC Step Size", 0, 1, 0.001));
  searchComponents.add(new SettingsSlider("MCMC Max Iterations", 1, 100, 1));
	searchComponents.add(new SettingsSlider("L-M Max Iterations", 1, 1000, 1));
  //searchComponents.add(new SettingsSlider("Mean Shift Epsilon", 0, 0.01, 1e-6));
  searchComponents.add(new SettingsSlider("Temperature", 0, 25, 0.01));
  //searchComponents.add(new SettingsSlider("Finite Difference Window", 1e-7, 1e-3, 1e-7));
  searchComponents.add(new SettingsBoolButton("Export Traces"));
  searchComponents.add(new SettingsBoolButton("Generate Graph"));
  searchComponents.add(new SettingsSlider("Search Threads", 1, thread::hardware_concurrency(), 1));
  searchComponents.add(new SettingsBoolButton("Use Mask"));
  searchComponents.add(new SettingsSlider("Freeze Region Tolerance", 0, 25, 0.1));
  searchComponents.add(new SettingsBoolButton("Reduce Repeat Edits"));
  searchComponents.add(new SettingsSlider("Expected Value Weight", 0, 1, 0.01));
  searchComponents.add(new SettingsBoolButton("Uniform Edit Weights"));
  searchComponents.add(new SettingsBoolButton("Randomzie Starts"));
  searchComponents.add(new SettingsSlider("Switching Time", 1, 500, 1));
  searchComponents.add(new SettingsSlider("Movable Threads", 0, thread::hardware_concurrency(), 1));
  searchComponents.add(new SettingsBoolButton("Continuous Sort"));
  searchComponents.add(new SettingsBoolButton("Use Styles"));
  searchComponents.add(new SettingsSlider("Repulsion Cone K", 0, 2, 0.01));
  searchComponents.add(new SettingsSlider("Repulsion Cost K", 0, 5, 0.01));
  searchComponents.add(new SettingsSlider("Repulsion Num Pairs", 1, 200, 1));
  searchComponents.add(new SettingsSlider("Big Color Bucket Weight", 0, 1, 0.001));
  searchComponents.add(new SettingsBoolButton("Recompute Color Weights"));
  searchComponents.add(new SettingsBoolButton("Unconstrained"));
  searchComponents.add(new SettingsBoolButton("px intens dist"));
  searchComponents.add(new SettingsBoolButton("Iterative System Select"));
  searchComponents.add(new SettingsBoolButton("Disable Pin Adjustment"));
  _settings.addSection("Search", searchComponents);


  Array<PropertyComponent*> clusterComponents;
  clusterComponents.add(new SettingsBoolButton("Auto Cluster"));
  clusterComponents.add(new SettingsSlider("Thumbnails Per Row", 1, 20, 1));
	clusterComponents.add(new SettingsChoice("Cluster Display Mode", { "Columns", "Grid" }));
  clusterComponents.add(new SettingsChoice("Primary Clustering Method", { "K-Means", "Mean Shift", "Spectral Clustering",
    "Divisive K-Means", "Thresholded Devisive", "Styles" }));
  clusterComponents.add(new SettingsChoice("Primary Distance Metric", { "Per-Pixel Average Lab Difference",
    "Per-Pixel Maximum Lab Difference", "Per-Pixel 90th Percentile Difference", "Lab L2 Norm", "Luminance L2 Norm",
    "Parameter L2 Norm", "Softmax Parameter L2 Norm", "Attribute Function Distance", "Directed Per-Pixel Average Lab Difference",
		"Per-Pixel Average Gradient Direction Difference", "Selected Key Light Parameter L2 Norm", "Grayscale Paramter L2" })); //, "Whitened Parameter L2 Norm"
  clusterComponents.add(new SettingsSlider("Number of Primary Clusters", 1, 30, 1));
  clusterComponents.add(new SettingsSlider("Primary Divisive Threshold", 1e-3, 25, 0.001));
  clusterComponents.add(new SettingsChoice("Primary Focus Region", { "All", "Foreground", "Background" }));
  clusterComponents.add(new SettingsChoice("Secondary Clustering Method", { "K-Means", "Mean Shift", "Spectral Clustering",
    "Divisive K-Means", "Thresholded Devisive", "Styles" }));
  clusterComponents.add(new SettingsChoice("Secondary Distance Metric", { "Per-Pixel Average Lab Difference",
    "Per-Pixel Maximum Lab Difference", "Per-Pixel 90th Percentile Difference", "Lab L2 Norm", "Luminance L2 Norm",
    "Parameter L2 Norm", "Softmax Parameter L2 Norm", "Attribute Function Distance", "Directed Per-Pixel Average Lab Difference",
		"Per-Pixel Average Gradient Direction Difference", "Selected Key Light Parameter L2 Norm", "Grayscale Paramter L2" })); //, "Whitened Parameter L2 Norm"
  clusterComponents.add(new SettingsSlider("Number of Secondary Clusters", 1, 30, 1));
  clusterComponents.add(new SettingsSlider("Secondary Divisive Threshold", 1e-3, 25, 0.001));
  clusterComponents.add(new SettingsChoice("Secondary Focus Region", { "All", "Foreground", "Background" }));
  clusterComponents.add(new SettingsSlider("Mean Shift Bandwidth", 0, 20, 0.01));
  clusterComponents.add(new SettingsSlider("Spectral Bandwidth", 1e-3, 5, 0.001));
  //_settings.addSection("Clustering", clusterComponents);

  Array<PropertyComponent*> renderComponents;
  //_width = new SettingsSlider("Frame Width", 1, 3840, 1);
  //_height = new SettingsSlider("Frame Height", 1, 2160, 1);
  //_width->_other = _height;
  //_height->_other = _width;
  renderComponents.add(new SettingsSlider("Exposure", 0, 6, 0.01));
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

	Array<PropertyComponent*> otherComponents;
	otherComponents.add(new SettingsButton("Log Directory"));
  otherComponents.add(new SettingsButton("OSC Settings"));
	_settings.addSection("Other", otherComponents);

  addAndMakeVisible(_settings);
}

SettingsEditor::~SettingsEditor()
{
  _settings.clear();
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

void SettingsEditor::refresh()
{
	_settings.refreshAll();
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

void SettingsWindow::refresh()
{
	_settingsEditor->refresh();
}
