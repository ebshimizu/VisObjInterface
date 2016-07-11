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

class ColorPropertyPicker : public PropertyComponent, public ChangeListener, public Button::Listener
{
public:
  ColorPropertyPicker(string id, string param, LumiverseColor* val);
  ~ColorPropertyPicker();

  void paint(Graphics& g) override;
  void mouseDown(const MouseEvent& event) override;
  virtual void changeListenerCallback(ChangeBroadcaster* source) override;
  
  virtual void buttonClicked(Button* b) override;
	virtual void refresh() override;

private:
  string _id;
  string _param;
  LumiverseColor* _val;
	ColoredTextButton* _button;
};


//==============================================================================
/*
*/
class ParamControls    : public Component
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

private:
  PropertyPanel _properties;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamControls)
};


#endif  // PARAMCONTROLS_H_INCLUDED
