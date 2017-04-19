/*
  ==============================================================================

    ConstraintEditor.h
    Created: 20 Jul 2016 5:26:24pm
    Author:  eshimizu

  ==============================================================================
*/

#ifndef CONSTRAINTEDITOR_H_INCLUDED
#define CONSTRAINTEDITOR_H_INCLUDED

#include "globals.h"
#include "ColoredTextButton.h"

// Constraint Components provide an interface to edit relative constraints for the given search
class ConstraintComponent : public Component, public ButtonListener
{
public:
  ConstraintComponent(ConstraintType t, int id, Component* parent);
  ~ConstraintComponent();

  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  ConstraintType getType();
  int getId();

  virtual void buttonClicked(Button* b) override;

protected:
  PopupMenu getSelectorMenu(map<int, string>& cmdOut);
  void showDeviceSelectMenu(Button* b, DeviceSet& d);

  String deviceSetToString(DeviceSet& d);

  ConstraintType _t;
  int _id;

  TextButton _deleteButton;
  Component* _parent;
};

class KeyConstraint : public ConstraintComponent, public ButtonListener
{
public:
  KeyConstraint(int id, Component* parent);
  ~KeyConstraint();

  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  virtual void buttonClicked(Button* b) override;

  void updateButtonText();

  TextButton _deviceSelector;
  ToggleButton _exclusiveButton;

  DeviceSet _affected;
  bool _exclusive;
};

class ExcludeConstraint : public ConstraintComponent, public ButtonListener, public ComboBoxListener {
public:
  ExcludeConstraint(int id, Component* parent);
  ~ExcludeConstraint();

  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  virtual void buttonClicked(Button* b) override;
  void comboBoxChanged(ComboBox* b) override;

  void updateButtonText();

  TextButton _deviceSelector;
  ComboBox _mode;

  bool _turnOff;
  DeviceSet _affected;
};

class RelativeConstraint : public ConstraintComponent, public ButtonListener, public SliderListener {
public:
  RelativeConstraint(int id, Component* parent);
  ~RelativeConstraint();

  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  virtual void buttonClicked(Button* b) override;
  void sliderValueChanged(Slider* s) override;

  void updateButtonText();

  TextButton _sourceSelector;
  TextButton _targetSelector;
  Slider _ratioSlider;

  DeviceSet _source;
  DeviceSet _target;
  float _ratio;
};

class SaturationConstraint : public ConstraintComponent, public SliderListener, public ButtonListener {
public:
  SaturationConstraint(int id, Component* parent);
  ~SaturationConstraint();

  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  virtual void buttonClicked(Button* b) override;
  void sliderValueChanged(Slider* s) override;

  void updateButtonText();

  TextButton _deviceSelector;
  Slider _satSlider;

  DeviceSet _affected;
  float _min;
  float _max;
};

class ConstraintContainer : public Component
{
public:
  ConstraintContainer();
  ~ConstraintContainer();

  void resized() override;
  void paint(Graphics& g) override;

  void addConstraint(ConstraintComponent* c);
  void deleteConstraint(int id);
  void deleteAllConstraints();

  void setWidth(int width);
  ConstraintData getConstraintData();

  Array<ConstraintComponent*> _constraints;
};

class ConstraintEditor : public Component, public ButtonListener
{
public:
  ConstraintEditor();
  ~ConstraintEditor();

  void paint(Graphics& g) override;
  void resized() override;

  void addConstraint(ConstraintType t);
  void deleteConstraint(int id);
  void deleteAllConstraints();
  ConstraintData getConstraintData();

  void buttonClicked(Button* b) override;

private:
  int _id;
  Viewport* _vp;
  ConstraintContainer* _cc;
  
  TextButton _addButton;
  TextButton _deleteAllButton;
};

#endif  // CONSTRAINTEDITOR_H_INCLUDED
