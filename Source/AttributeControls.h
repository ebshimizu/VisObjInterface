/*
  ==============================================================================

    AttributeControls.h
    Created: 15 Dec 2015 5:07:22pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef ATTRIBUTECONTROLS_H_INCLUDED
#define ATTRIBUTECONTROLS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "globals.h"
#include "SettingsEditor.h"
#include "ParamControls.h"
#include "HistoryPanel.h"
#include "AttributeControllerBase.h"
#include "GibbsComponents.h"
#include "Idea.h"

class DeviceSelector : public Component, public ListBoxModel
{
public:
  DeviceSelector(vector<string> initialSelectedIds, function<void(vector<string>)> update);
  ~DeviceSelector();

  virtual int getNumRows() override;
  virtual void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
  virtual void selectedRowsChanged(int lastRowSelected) override;
  virtual void resized() override;
  int getListHeight();

private:
  ListBox _list;
  StringArray _deviceIds;
  function<void(vector<string>)> _updateFunc;
};

class AttributeControlsList : public Component
{
public:
  AttributeControlsList();
  ~AttributeControlsList();

  void setWidth(int width);

  void paint(Graphics& g);
  void resized();

  void addAttributeController(AttributeControllerBase* control);
  void removeAttributeController(string name);
  void removeAllControllers();
  void runPreprocess();

  void lockImageAttrs();
  void unlockImageAttrs();

  map<string, AttributeControllerBase*> getActiveAttribues();

private:
  int _width;
  int _height;

  int _componentHeight = 50;

  map<string, AttributeControllerBase*> _controls;
};

//==============================================================================
/*
*/
class AttributeControls : public Component, public Button::Listener, public ComboBox::Listener
{
public:
  AttributeControls();
  ~AttributeControls();

  void paint (Graphics&);
  void resized();
  void refresh();

  // Used for when a new rig gets loaded into the interface
  void reload();
  
  virtual void buttonClicked(Button* b) override;
  virtual void comboBoxChanged(ComboBox* b) override;

  map<string, AttributeControllerBase*> getActiveAttributes();

  // Deletes all the attributes in the container
  void deleteAllAttributes();
  
  // Adds a new controller to the container.
  void addAttributeController(AttributeControllerBase* controller);
  
  // When the search is running, the image attribute mode controls should be disabled
  void lockAttributeModes();
  void unlockAttributeModes();

  // initializes gibbs pallets
  void initPallets();

  GibbsSchedule* getGibbsSchedule();

  // Gets the parameter controller
  ParamControls* getParamController();

  // Gets the history controller
  HistoryPanel* getHistory();

  // sets colors in the palettes
  void setColors(vector<Eigen::VectorXd> colors, double intens, vector<float> weights);

  void refreshSettings();

  void addIdea(Image i, String name);
  void saveIdeas(File destFolder);
  void loadIdeas(File srcFolder);

  // updates the sort menu with the set of currently available sort methods
  void updateSortMenu();

  // returns a set of devices that affect the idea's selected region
  DeviceSet computeAffectedDevices(Rectangle<float> region, double threshold = 0.01);
  DeviceSet computeAffectedDevices(shared_ptr<Idea> idea, double threshold = 0.01);
private:
  void initAttributes();

  // returns a set of devices that do not contain any devices in the intens or color sets
  void filterPinnedDevices(DeviceSet& target, set<string> intens, set<string> color);

  class PaletteControls : public Component {
  public:
    PaletteControls();
    ~PaletteControls();

    virtual void resized();

    GibbsPalletContainer* _palettes;
    Viewport* _paletteViewer;
  };

  class IdeaControls : public Component {
  public:
    IdeaControls();
    ~IdeaControls();

    virtual void resized();

    IdeaList* _ideas;
    Viewport* _ideaViewer;
  };

  PaletteControls* _vr;
  IdeaControls* _ic;

  GibbsConstraintContainer* _tempConstraints;

  TabbedComponent _tabs;

  AttributeControlsList* _container;
  ParamControls* _paramControls;
  HistoryPanel* _history;
  SettingsEditor* _settings;

  Viewport* _componentView;
  Viewport* _historyViewer;
  
  TextButton* _search;
  TextButton* _sortButton;
  TextButton* _setKeyButton;
  TextButton* _clusterButton;
  TextButton _reset;
  ComboBox* _sort;
  Slider* _numClusters;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeControls)
};


#endif  // ATTRIBUTECONTROLS_H_INCLUDED
