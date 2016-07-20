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
