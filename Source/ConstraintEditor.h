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

private:
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

private:
  void updateButtonText();

  TextButton _deviceSelector;
  ComboBox _mode;

  bool _turnOff;
  DeviceSet _affected;
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
  map<ConstraintType, vector<ConstraintComponent*> > getConstraintData();

  void buttonClicked(Button* b) override;

private:
  int _id;
  Viewport* _vp;
  ConstraintContainer* _cc;
  
  TextButton _addButton;
};

#endif  // CONSTRAINTEDITOR_H_INCLUDED
