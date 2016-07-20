/*
  ==============================================================================

    ConstraintEditor.cpp
    Created: 20 Jul 2016 5:26:24pm
    Author:  eshimizu

  ==============================================================================
*/

#include "ConstraintEditor.h"

ConstraintEditor::ConstraintEditor() : _font(14.0)
{
  addAndMakeVisible(_table);
  _table.setModel(this);

  _table.getHeader().addColumn("ID", 1, 150);
  _table.getHeader().addColumn("Scope", 2, 100);
  _table.getHeader().addColumn("Params", 3, 150);
  _table.getHeader().addColumn("Devices", 4, 200);
  _table.getHeader().addColumn("Delete", 5, 100);

  _table.setMultipleSelectionEnabled(false);

  for (auto& c : getGlobalSettings()->_constraints) {
    _ids.add(c.first);
  }
}

ConstraintEditor::~ConstraintEditor()
{
}

int ConstraintEditor::getNumRows()
{
  return getGlobalSettings()->_constraints.size();
}

void ConstraintEditor::paintRowBackground(Graphics & g, int rowNumber, int width, int height, bool rowIsSelected)
{
  if (rowIsSelected)
    g.fillAll(Colours::lightblue);
  else if (rowNumber % 2)
    g.fillAll(Colour(0xffeeeeee));
}

void ConstraintEditor::paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
  g.setColour(Colours::black);
  g.setFont(_font);

  if (columnId == 1) {
    String text(_ids[rowNumber]);
    g.drawText(text, 2, 0, width - 4, height, Justification::centredLeft, true);
  }

  g.setColour(Colours::black.withAlpha(0.2f));
  g.fillRect(width - 1, 0, 1, height);
}

void ConstraintEditor::sortOrderChanged(int newSortColumnId, bool isForwards)
{
}

Component * ConstraintEditor::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component * existingComponentToUpdate)
{
  if (columnId == 2) {
    ConstraintScopeComponent* scopeBox = static_cast<ConstraintScopeComponent*>(existingComponentToUpdate);

    if (scopeBox == nullptr)
      scopeBox = new ConstraintScopeComponent();

    scopeBox->setTargetConstraint(_ids[rowNumber].toStdString());
    return scopeBox;
  }
  else {
    return nullptr;
  }
}

void ConstraintEditor::resized()
{
  _table.setBoundsInset(BorderSize<int>(8));
}

ConstraintWindow::ConstraintWindow() :
  DocumentWindow("Constraints", Colour(0xff333333), TitleBarButtons::closeButton, true)
{
  setContentOwned(_constraintEditor = new ConstraintEditor(), false);
}

ConstraintWindow::~ConstraintWindow()
{
  _constraintEditor = nullptr;
}

void ConstraintWindow::resized()
{
  _constraintEditor->setBounds(getLocalBounds());
}

void ConstraintWindow::closeButtonPressed()
{
  delete this;
}

ConstraintEditor::ConstraintScopeComponent::ConstraintScopeComponent()
{
  addAndMakeVisible(_box);
  _box.addItem("Local", LOCAL + 1);
  _box.addItem("Global", GLOBAL + 1);

  _box.addListener(this);
}

ConstraintEditor::ConstraintScopeComponent::~ConstraintScopeComponent()
{
}

void ConstraintEditor::ConstraintScopeComponent::resized()
{
  _box.setBoundsInset(BorderSize<int>(2));
}

void ConstraintEditor::ConstraintScopeComponent::setTargetConstraint(string id)
{
  _id = id;
  _box.setSelectedId(getGlobalSettings()->_constraints[_id]._scope + 1, dontSendNotification);
}

void ConstraintEditor::ConstraintScopeComponent::comboBoxChanged(ComboBox * b)
{
  getGlobalSettings()->_constraints[_id]._scope = (ConsistencyScope) (b->getSelectedId() - 1);
}
