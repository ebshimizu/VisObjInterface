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
private:
  string _id;
};

//==============================================================================
/*
*/
class SettingsEditor    : public Component
{
public:
  SettingsEditor();
  ~SettingsEditor();

  void paint (Graphics&);
  void resized();

private:
  PropertyPanel _settings;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsEditor)
};

class SettingsWindow : public DocumentWindow
{
public:
  SettingsWindow();
  ~SettingsWindow();

  void resized() override;

  void closeButtonPressed() override;
private:
  ScopedPointer<SettingsEditor> _settingsEditor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsWindow);
};

#endif  // SETTINGSEDITOR_H_INCLUDED
