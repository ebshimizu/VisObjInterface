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

class ConstraintDeviceSelector : public Component, public ListBoxModel
{
public:
  ConstraintDeviceSelector(string id, Button* b);
  ~ConstraintDeviceSelector();

  virtual int getNumRows() override;
  virtual void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
  virtual void selectedRowsChanged(int lastRowSelected) override;
  virtual void resized() override;
  int getListHeight();

private:
  string _id;
  ListBox _list;
  StringArray _deviceIds;
  Button* _b;
};

class ConstraintParameterSelector : public Component, public ListBoxModel
{
public:
  ConstraintParameterSelector(string id, Button* b);
  ~ConstraintParameterSelector();

  virtual int getNumRows() override;
  virtual void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
  virtual void selectedRowsChanged(int lastRowSelected) override;
  virtual void resized() override;
  int getListHeight();

private:
  string _id;
  ListBox _list;
  Button* _b;
};

class ConstraintEditor : public Component, public TableListBoxModel
{
public:
  ConstraintEditor();
  ~ConstraintEditor();

  int getNumRows() override;
  void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
  void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
  void sortOrderChanged(int newSortColumnId, bool isForwards) override;
  Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;
  void reload();

  void resized() override;

  class ConstraintScopeComponent : public Component, private ComboBoxListener
  {
  public:
    ConstraintScopeComponent();
    ~ConstraintScopeComponent();

    void resized() override;
    void setTargetConstraint(string id);
    void comboBoxChanged(ComboBox* b) override;

  private:
    ComboBox _box;
    string _id;
  };

  class ConstraintDevicesComponent : public Component, private ButtonListener
  {
  public:
    ConstraintDevicesComponent();
    ~ConstraintDevicesComponent();

    void resized() override;
    void setTargetConstraint(string id);
    void buttonClicked(Button* b) override;
    void updateButtonText();

  private:
    string _id;
    TextButton _button;
  };

  class ConstraintParamsComponent : public Component, private ButtonListener
  {
  public:
    ConstraintParamsComponent();
    ~ConstraintParamsComponent();

    void resized() override;
    void setTargetConstraint(string id);
    void buttonClicked(Button* b) override;
    void updateButtonText();

  private:
    string _id;
    TextButton _button;
  };

  class ConstraintDeleteComponent : public Component, private ButtonListener
  {
  public:
    ConstraintDeleteComponent(ConstraintEditor* parent);
    ~ConstraintDeleteComponent();

    void resized() override;
    void setTargetConstraint(string id);
    void buttonClicked(Button* b) override;
    
  private:
    string _id;
    ColoredTextButton _button;
    ConstraintEditor* _parent;
  };

private:
  TableListBox _table;
  Font _font;

  StringArray _ids;
};

class ConstraintWindow : public DocumentWindow
{
public:
  ConstraintWindow();
  ~ConstraintWindow();

  void resized() override;

  void closeButtonPressed() override;

private:
  ScopedPointer<ConstraintEditor> _constraintEditor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConstraintWindow);
};


#endif  // CONSTRAINTEDITOR_H_INCLUDED
