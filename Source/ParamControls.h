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

class FloatPropertySlider : public SliderPropertyComponent
{
public:
  FloatPropertySlider(string id, string param, LumiverseFloat* val);
  ~FloatPropertySlider();

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

  virtual void setValue(double newValue) override;
  virtual double getValue() const override;
  void sliderDragStarted(Slider* s) override;
  void sliderDragEnded(Slider* s) override;

private:
  string _id;
  string _param;
};

class ColorPropertySlider : public SliderPropertyComponent
{
public:
  ColorPropertySlider(string id, string param, string channel, LumiverseColor* val);
  ~ColorPropertySlider();

  virtual void setValue(double newValue) override;
  virtual double getValue() const override;
  void sliderDragStarted(Slider* s) override;
  void sliderDragEnded(Slider* s) override;

private:
  string _id;
  string _param;
  string _channel;
};

class HSVColorPropertySlider : public SliderPropertyComponent
{
public:
  HSVColorPropertySlider(string id, string param, string channel, LumiverseColor* val);
  ~HSVColorPropertySlider();

  virtual void setValue(double newValue) override;
  virtual double getValue() const override;
  void sliderDragStarted(Slider* s) override;
  void sliderDragEnded(Slider* s) override;

private:
  string _id;
  string _param;
  string _channel;
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
