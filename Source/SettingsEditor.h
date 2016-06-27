/*
  ==============================================================================

    SettingsEditor.h
    Created: 31 Dec 2015 12:50:03pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef SETTINGSEDITOR_H_INCLUDED
#define SETTINGSEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "globals.h"

class SettingsSlider : public SliderPropertyComponent
{
public:
  SettingsSlider(string id, double min, double max, double step);
  ~SettingsSlider();

  virtual void setValue(double newValue) override;
  virtual double getValue() const override;
  void sliderValueChanged(Slider* s) override;
  
  // mostly used for the height and width properties so they can update each other
  SettingsSlider* _other;
private:
  string _id;
};

class SettingsBoolButton : public BooleanPropertyComponent
{
public:
  SettingsBoolButton(string id);
  ~SettingsBoolButton();

  virtual void setState(bool newState) override;
  virtual bool getState() const override;
private:
  string _id;
};

class SettingsChoice : public ChoicePropertyComponent
{
public:
  SettingsChoice(string name, StringArray choices);
  ~SettingsChoice();

  virtual int getIndex() const override;
  virtual void setIndex(int newIndex) override;
};

//==============================================================================
/*
*/
class SettingsEditor : public Component
{
public:
  SettingsEditor();
  ~SettingsEditor();

  void paint (Graphics&);
  void resized();

  void updateDims();
	void refresh();

private:
  PropertyPanel _settings;
  SettingsSlider* _width;
  SettingsSlider* _height;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsEditor)
};

class SettingsWindow : public DocumentWindow
{
public:
  SettingsWindow();
  ~SettingsWindow();

  void resized() override;

  void closeButtonPressed() override;
  void updateDims();
	void refresh();

private:
  ScopedPointer<SettingsEditor> _settingsEditor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsWindow);
};

#endif  // SETTINGSEDITOR_H_INCLUDED
