/*
  ==============================================================================

    ParamControls.h
    Created: 15 Dec 2015 5:07:12pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef PARAMCONTROLS_H_INCLUDED
#define PARAMCONTROLS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "globals.h"
#include "ColoredTextButton.h"

class FloatPropertySlider : public SliderPropertyComponent
{
public:
  FloatPropertySlider(string id, string param, LumiverseFloat* val);
  ~FloatPropertySlider();

  void paint(Graphics& g) override;
  void mouseDown(const MouseEvent& event) override;

  virtual void setValue(double newValue) override;
  virtual double getValue() const override;
  void sliderDragStarted(Slider* s) override;
  void sliderDragEnded(Slider* s) override;

private:
  string _id;
  string _param;
};

class OrientationPropertySlider : public SliderPropertyComponent
{
public:
  OrientationPropertySlider(string id, string param, LumiverseOrientation* val);
  ~OrientationPropertySlider();

  void paint(Graphics& g) override;
  void mouseDown(const MouseEvent& event) override;

  virtual void setValue(double newValue) override;
  virtual double getValue() const override;
  void sliderDragStarted(Slider* s) override;
  void sliderDragEnded(Slider* s) override;

private:
  string _id;
  string _param;
};

class ColorPickerButton : public Component, public ChangeListener, public Button::Listener
{
public:
  ColorPickerButton(string id, string param, LumiverseColor* val);
  ~ColorPickerButton();

  void paint(Graphics& g) override;
  virtual void changeListenerCallback(ChangeBroadcaster* source) override;
  void changeId(string newId);
  void resized() override;
  
  virtual void buttonClicked(Button* b) override;
	virtual void refresh();

private:
  string _id;
  string _param;
  LumiverseColor* _val;
	ColoredTextButton* _button;
};


//==============================================================================
/*
*/
class ParamControls    : public Component, public TableListBoxModel, public Slider::Listener,
  public ChangeListener, public Button::Listener
{
public:
  ParamControls();
  ~ParamControls();

  void paint (Graphics&);
  void resized();

  // Initalizes the property panel used by this component
  void initProperties();

  // Refreshes the components in the property panel
  void refreshParams();

  // Return the number of rows in the table, which is the number of devices in the rig
  virtual int getNumRows();

  virtual void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected);
  virtual void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected);
  virtual Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate);
  virtual void selectedRowsChanged(int lastRowSelected);
  virtual void cellClicked(int rowNumber, int columnId, const MouseEvent& e);

  virtual void sliderValueChanged(Slider* s) override;
  virtual void buttonClicked(Button* b) override;
  virtual void changeListenerCallback(ChangeBroadcaster* source) override;

  void lockSelected(vector<string> params);
  void unlockSelected(vector<string> params);
  StringArray getSelectedIds();

private:
  PropertyPanel _properties;
  StringArray _ids;
  TableListBox _table;
  StringArray _selected;

  Slider _groupIntens;
  ColoredTextButton _groupColor;

  TextButton _qsArea;
  TextButton _qsSystem;
  TextButton _qsNone;
  TextButton _qsAll;
  TextButton _invert;
  TextButton _render;

  // stores the most recently clicked on device's color
  Colour _recentColor;
  float _recentIntens;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamControls)
};


#endif  // PARAMCONTROLS_H_INCLUDED
