/*
  ==============================================================================

    ConstraintEditor.cpp
    Created: 20 Jul 2016 5:26:24pm
    Author:  eshimizu

  ==============================================================================
*/

#include "ConstraintEditor.h"

ConstraintDeviceSelector::ConstraintDeviceSelector(string id, Button* b) : _id(id), _b(b) {
  // get list of ids to display
  auto deviceIds = getRig()->getAllDevices().getIds();

  for (auto& s : deviceIds) {
    _deviceIds.add(s);
  }

  addAndMakeVisible(_list);
  _list.setModel(this);
  _list.setMultipleSelectionEnabled(true);
  _list.setClickingTogglesRowSelection(true);

  // set selected rows
  auto selectedIds = getGlobalSettings()->_constraints[_id]._affected.getIds();
  for (auto s : selectedIds) {
    _list.selectRow(_deviceIds.indexOf(String(s)), false, false);
  }
}

ConstraintDeviceSelector::~ConstraintDeviceSelector()
{
}

int ConstraintDeviceSelector::getNumRows()
{
  return _deviceIds.size();
}

void ConstraintDeviceSelector::paintListBoxItem(int rowNumber, Graphics & g, int width, int height, bool rowIsSelected)
{
  if (rowIsSelected)
    g.fillAll(Colours::lightblue);
  else
    g.fillAll(Colour(0xffa3a3a3));

  g.setColour(Colours::black);
  g.setFont(14);
  g.drawText(_deviceIds[rowNumber], 2, 0, width - 4, height, Justification::centredLeft, true);
}

void ConstraintDeviceSelector::selectedRowsChanged(int /* lastRowSelected */)
{
  // check selected things, update device set for relevant constraint
  DeviceSet selected;
  int selectedRows = _list.getNumSelectedRows();

  for (int i = 0; i < selectedRows; i++) {
    DeviceSet d = getRig()->select(_deviceIds[_list.getSelectedRow(i)].toStdString());
    selected = selected.add(d);
  }

  getGlobalSettings()->_constraints[_id]._affected = selected;

  auto ids = selected.getIds();
  String text;
  bool first = true;
  for (auto& id : ids) {
    if (first) {
      text += id;
      first = false;
    }
    else {
      text += ", " + id;
    }
  }

  _b->setButtonText(text);
}

void ConstraintDeviceSelector::resized()
{
  _list.setBounds(getLocalBounds());
}

int ConstraintDeviceSelector::getListHeight()
{
  // returns the total height of the list
  return _list.getRowHeight() * getNumRows();
}

ConstraintParameterSelector::ConstraintParameterSelector(string id, Button * b) : _id(id), _b(b)
{
  addAndMakeVisible(_list);
  _list.setModel(this);
  _list.setMultipleSelectionEnabled(true);
  _list.setClickingTogglesRowSelection(true);

  // set selected rows
  // a little right now weird since we're grouping all color restrictions as one constraint
  set<EditParam> params = getGlobalSettings()->_constraints[_id]._params;
  for (auto param : params) {
    if (param == INTENSITY)
      _list.selectRow(0, true, false);
    else
      _list.selectRow(1, true, false);
  }
}

ConstraintParameterSelector::~ConstraintParameterSelector()
{
}

int ConstraintParameterSelector::getNumRows()
{
  // just intensity and color
  return 2;
}

void ConstraintParameterSelector::paintListBoxItem(int rowNumber, Graphics & g, int width, int height, bool rowIsSelected)
{
  if (rowIsSelected)
    g.fillAll(Colours::lightblue);
  else
    g.fillAll(Colour(0xffa3a3a3));

  g.setColour(Colours::black);
  g.setFont(14);
  
  String text;
  if (rowNumber == 0)
    text = "Intensity";
  else if (rowNumber == 1)
    text = "Color";

  g.drawText(text, 2, 0, width - 4, height, Justification::centredLeft, true);
}

void ConstraintParameterSelector::selectedRowsChanged(int /* lastRowSelected */)
{
  set<EditParam> params;
  vector<String> paramStrings;

  int selectedRows = _list.getNumSelectedRows();

  for (int i = 0; i < selectedRows; i++) {
    if (_list.getSelectedRow(i) == 0) {
      params.insert(INTENSITY);
      paramStrings.push_back("Intensity");
    }
    else if (_list.getSelectedRow(i) == 1) {
      params.insert({ RED, GREEN, BLUE, HUE, SAT });
      paramStrings.push_back("Color");
    }
  }
  getGlobalSettings()->_constraints[_id]._params = params;
  
  String text;
  bool first = true;
  for (auto& p : paramStrings) {
    if (first) {
      text += p;
      first = false;
    }
    else {
      text += ", " + p;
    }
  }

  _b->setButtonText(text);
}

void ConstraintParameterSelector::resized()
{
  _list.setBounds(getLocalBounds());
}

int ConstraintParameterSelector::getListHeight()
{
  // returns the total height of the list
  return _list.getRowHeight() * getNumRows();
}

// ============================================================================

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

  addAndMakeVisible(_newConstraint);
  addAndMakeVisible(_clearConstraints);
  addAndMakeVisible(_resetConstraints);
  _newConstraint.addListener(this);
  _clearConstraints.addListener(this);
  _resetConstraints.addListener(this);

  _newConstraint.setName("New");
  _clearConstraints.setName("Clear");
  _resetConstraints.setName("Reset");

  _newConstraint.setButtonText("New");
  _clearConstraints.setButtonText("Clear");
  _resetConstraints.setButtonText("Reset");
}

ConstraintEditor::~ConstraintEditor()
{
}

int ConstraintEditor::getNumRows()
{
  _ids.clear();
  for (auto& c : getGlobalSettings()->_constraints) {
    _ids.add(c.first);
  }

  return (int) getGlobalSettings()->_constraints.size();
}

void ConstraintEditor::paintRowBackground(Graphics & g, int rowNumber, int /* width */, int /* height */, bool rowIsSelected)
{
  if (rowIsSelected)
    g.fillAll(Colours::lightblue);
  else if (rowNumber % 2)
    g.fillAll(Colour(0xffeeeeee));
}

void ConstraintEditor::paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool /* rowIsSelected */)
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

void ConstraintEditor::sortOrderChanged(int /* newSortColumnId */, bool /* isForwards */)
{
}

Component * ConstraintEditor::refreshComponentForCell(int rowNumber, int columnId, bool /* isRowSelected */, Component * existingComponentToUpdate)
{
  if (columnId == 2) {
    ConstraintScopeComponent* scopeBox = static_cast<ConstraintScopeComponent*>(existingComponentToUpdate);

    if (scopeBox == nullptr)
      scopeBox = new ConstraintScopeComponent();

    scopeBox->setTargetConstraint(_ids[rowNumber].toStdString());
    return scopeBox;
  }
  else if (columnId == 3) {
    ConstraintParamsComponent* paramSelect = static_cast<ConstraintParamsComponent*>(existingComponentToUpdate);

    if (paramSelect == nullptr)
      paramSelect = new ConstraintParamsComponent();

    paramSelect->setTargetConstraint(_ids[rowNumber].toStdString());
    return paramSelect;
  }
  else if (columnId == 4) {
    ConstraintDevicesComponent* deviceSelect = static_cast<ConstraintDevicesComponent*>(existingComponentToUpdate);

    if (deviceSelect == nullptr)
      deviceSelect = new ConstraintDevicesComponent();

    deviceSelect->setTargetConstraint(_ids[rowNumber].toStdString());
    return deviceSelect;
  }
  else if (columnId == 5) {
    ConstraintDeleteComponent* deleteButton = static_cast<ConstraintDeleteComponent*>(existingComponentToUpdate);

    if (deleteButton == nullptr)
      deleteButton = new ConstraintDeleteComponent(this);

    deleteButton->setTargetConstraint(_ids[rowNumber].toStdString());
    return deleteButton;
  }
  else {
    return nullptr;
  }
}

void ConstraintEditor::reload()
{
  _table.updateContent();
}

void ConstraintEditor::buttonClicked(Button * b)
{
  if (b->getName() == "New") {
    AlertWindow w("New Constraint",
      "",
      AlertWindow::QuestionIcon);

    w.addTextEditor("text", "", "Constraint Name");
    w.addButton("Create", 1, KeyPress(KeyPress::returnKey, 0, 0));
    w.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));

    if (w.runModalLoop() != 0) // is they picked 'ok'
    {
      // this is the text they entered..
      String text = w.getTextEditorContents("text");

      if (getGlobalSettings()->_constraints.count(text.toStdString()) > 0) {
        // already exists
        AlertWindow::showMessageBoxAsync(AlertWindow::InfoIcon,
          "Constraint Already Exists",
          "A constraint with this name already exists: " + text,
          "OK");
        return;
      }

      getGlobalSettings()->_constraints[text.toStdString()] = ConsistencyConstraint();
      reload();
    }
  }
  else if (b->getName() == "Clear") {
    bool confirmDelete = AlertWindow::showOkCancelBox(AlertWindow::WarningIcon, "Delete All Constraints",
      "Are you sure you want to delete all constraints?", "Yes", "No", nullptr);

    if (confirmDelete) {
      getGlobalSettings()->_constraints.clear();
      reload();
    }
  }
  else if (b->getName() == "Reset") {
    bool confirmDelete = AlertWindow::showOkCancelBox(AlertWindow::WarningIcon, "Reset All Constraints",
      "Are you sure you want to delete all constraints and replace them with the default constraints?",
      "Yes", "No", nullptr);

    if (confirmDelete) {
      getGlobalSettings()->_constraints.clear();
      getGlobalSettings()->generateDefaultConstraints();
      reload();
    }
  }
}

void ConstraintEditor::resized()
{
  auto lbounds = getLocalBounds();
  auto top = lbounds.removeFromTop(26);
  _newConstraint.setBounds(top.removeFromLeft(80).reduced(2));
  _clearConstraints.setBounds(top.removeFromLeft(80).reduced(2));
  _resetConstraints.setBounds(top.removeFromLeft(80).reduced(2));

  _table.setBounds(lbounds.reduced(4));
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

ConstraintEditor::ConstraintDevicesComponent::ConstraintDevicesComponent()
{
  addAndMakeVisible(_button);
  _button.addListener(this);
}

ConstraintEditor::ConstraintDevicesComponent::~ConstraintDevicesComponent()
{
}

void ConstraintEditor::ConstraintDevicesComponent::resized()
{
  _button.setBoundsInset(BorderSize<int>(2));
}

void ConstraintEditor::ConstraintDevicesComponent::setTargetConstraint(string id)
{
  _id = id;
  updateButtonText();
}

void ConstraintEditor::ConstraintDevicesComponent::buttonClicked(Button * /* b */)
{
  Viewport* vp = new Viewport();
  ConstraintDeviceSelector* cds = new ConstraintDeviceSelector(_id, &_button);
  cds->setSize(200, min(cds->getListHeight(), 300));
  vp->setViewedComponent(cds, true);
  vp->setSize(cds->getWidth(), cds->getHeight());

  CallOutBox::launchAsynchronously(vp, getScreenBounds(), nullptr);
}

void ConstraintEditor::ConstraintDevicesComponent::updateButtonText()
{
  auto ids = getGlobalSettings()->_constraints[_id]._affected.getIds();

  String text;
  bool first = true;
  for (auto& id : ids) {
    if (first) {
      text += id;
      first = false;
    }
    else {
      text += ", " + id;
    }
  }

  _button.setButtonText(text);
}

ConstraintEditor::ConstraintParamsComponent::ConstraintParamsComponent()
{
  addAndMakeVisible(_button);
  _button.addListener(this);
}

ConstraintEditor::ConstraintParamsComponent::~ConstraintParamsComponent()
{
}

void ConstraintEditor::ConstraintParamsComponent::resized()
{
  _button.setBoundsInset(BorderSize<int>(2));
}

void ConstraintEditor::ConstraintParamsComponent::setTargetConstraint(string id)
{
  _id = id;
  updateButtonText();
}

void ConstraintEditor::ConstraintParamsComponent::buttonClicked(Button * /* b */)
{
  Viewport* vp = new Viewport();
  ConstraintParameterSelector* cds = new ConstraintParameterSelector(_id, &_button);
  cds->setSize(200, min(cds->getListHeight(), 300));
  vp->setViewedComponent(cds, true);
  vp->setSize(cds->getWidth(), cds->getHeight());

  CallOutBox::launchAsynchronously(vp, getScreenBounds(), nullptr);
}

void ConstraintEditor::ConstraintParamsComponent::updateButtonText()
{
  auto params = getGlobalSettings()->_constraints[_id]._params;

  vector<string> paramStrings;

  if (params.count(INTENSITY) == 1)
    paramStrings.push_back("Intensity");
  if (params.count(RED) == 1 || params.count(GREEN) == 1 || params.count(BLUE) == 1 ||
    params.count(HUE) == 1 || params.count(SAT) == 1)
    paramStrings.push_back("Color");

  String text;
  bool first = true;
  for (auto& p : paramStrings) {
    if (first) {
      text += p;
      first = false;
    }
    else {
      text += ", " + p;
    }
  }

  _button.setButtonText(text);
}

ConstraintEditor::ConstraintDeleteComponent::ConstraintDeleteComponent(ConstraintEditor* parent) :
  _button("Delete", false), _parent(parent)
{
  addAndMakeVisible(_button);
  _button.setButtonText("Delete");
  _button.setColor(Colours::red);
  _button.addListener(this);
}

ConstraintEditor::ConstraintDeleteComponent::~ConstraintDeleteComponent()
{
}

void ConstraintEditor::ConstraintDeleteComponent::resized()
{
  _button.setBoundsInset(BorderSize<int>(2));
}

void ConstraintEditor::ConstraintDeleteComponent::setTargetConstraint(string id)
{
  _id = id;
}

void ConstraintEditor::ConstraintDeleteComponent::buttonClicked(Button * /* b */)
{
  bool confirmDelete = AlertWindow::showOkCancelBox(AlertWindow::WarningIcon, "Delete Constraint: " + _id,
    "Are you sure you want to delete this constraint?", "Yes", "No", nullptr);

  if (confirmDelete) {
    getGlobalSettings()->_constraints.erase(_id);
    _parent->reload();
  }
}
