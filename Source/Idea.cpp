/*
  ==============================================================================

    Idea.cpp
    Created: 23 Nov 2016 2:19:45pm
    Author:  falindrith

  ==============================================================================
*/

#include "Idea.h"
#include "MainComponent.h"

Idea::Idea(Image src, IdeaType type) : _src(src), _type(type)
{
  updateType();
  initUI();

  _selected = false;
  _isBeingDragged = false;
  _isRegionLocked = false;
  _focusArea = Rectangle<float>::leftTopRightBottom(0, 0, 1, 1);
  _lock.setToggleState(_isRegionLocked, dontSendNotification);
}

Idea::Idea(File srcFolder, JSONNode data)
{
  initUI();

  // set name
  setName(data.name());

  // find the image
  string file = data.find("image_name")->as_string();
  File img = srcFolder.getChildFile(String(file));
  FileInputStream in(img);

  // load imageBox
  PNGImageFormat pngReader;
  _src = pngReader.decodeImage(in);

  // type
  _type = (IdeaType)data.find("idea_type")->as_int();

  _typeSelector.setSelectedId((int)_type, dontSendNotification);
  updateType();

  // bounds
  auto bounds = data.find("bounds");
  _focusArea = Rectangle<float>::leftTopRightBottom(bounds->find("topX")->as_float(), bounds->find("topY")->as_float(),
    bounds->find("botX")->as_float(), bounds->find("botY")->as_float());

  _selected = false;
  _isBeingDragged = false;
  
  JSONNode::iterator l = data.find("region_lock");
  if (l != data.end()) {
    _isRegionLocked = l->as_bool();
  }
  else {
    _isRegionLocked = false;
  }

  _lock.setToggleState(_isRegionLocked, dontSendNotification);
}

Idea::~Idea()
{
}

void Idea::paint(Graphics & g)
{
  if (_selected) {
    g.setColour(Colour(0xff606060));
    g.fillAll();
  }

  auto lbounds = getLocalBounds();
  lbounds.removeFromTop(_headerSize);

  g.drawImageWithin(_src, lbounds.getX(), lbounds.getY(), lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);

  g.setColour(Colour(0xffb3b3b3));
  if (_isBeingDragged) {
    // use the points
    vector<Point<float> > pts = { _firstPt, _secondPt };
    auto select = Rectangle<float>::findAreaContainingPoints(pts.data(), 2);
    g.drawRect(select, 2);
  }
  else {
    // use the stored rectangle
    // convert the top left and bottom right to absolute coords and fit
    vector<Point<float> > pts;
    pts.push_back(relativeImageCoordsToLocal(_focusArea.getTopLeft()));
    pts.push_back(relativeImageCoordsToLocal(_focusArea.getBottomRight()));
    auto select = Rectangle<float>::findAreaContainingPoints(pts.data(), 2);
    g.drawRect(select, 2);
  }
}

void Idea::resized()
{
  auto lbounds = getLocalBounds();
  auto head1 = lbounds.removeFromTop(24);
  _delete.setBounds(head1.removeFromRight(24).reduced(2));
  _lock.setBounds(head1.removeFromRight(24).reduced(2));
  _nameEntry.setBounds(head1.reduced(2));

  auto top = lbounds.removeFromTop(24);
  _typeSelector.setBounds(top.reduced(2));

  // image
}

void Idea::mouseDown(const MouseEvent & e)
{
  // uses right click drag to select regions
  if (e.mods.isRightButtonDown() && !_isRegionLocked) {
    _firstPt = e.position;
    _secondPt = e.position;
    _isBeingDragged = true;
    repaint();
  }
  else if (e.mods.isLeftButtonDown()) {
    // selection, signal to parent container to adjust, then select and signal again
    IdeaList* parent = dynamic_cast<IdeaList*>(getParentComponent());
    parent->clearActiveIdea();
    _selected = true;
    parent->updateActiveIdea();
    repaint();
  }
}

void Idea::mouseDrag(const MouseEvent & e)
{
  if (e.mods.isRightButtonDown() && !_isRegionLocked) {
    _secondPt = e.position;
    repaint();
  }
}

void Idea::mouseUp(const MouseEvent & e)
{
  if (e.mods.isRightButtonDown() && !_isRegionLocked) {
    // save the rectangle
    vector<Point<float> > pts;
    pts.push_back(localToRelativeImageCoords(_firstPt));
    pts.push_back(localToRelativeImageCoords(_secondPt));
    _focusArea = Rectangle<float>::findAreaContainingPoints(pts.data(), 2);
    _isBeingDragged = false;
    repaint();
  }
}

void Idea::comboBoxChanged(ComboBox * b)
{
  _type = (IdeaType)b->getSelectedId();
  updateType();
}

void Idea::buttonClicked(Button * b)
{
  if (b->getName() == "lock") {
    _isRegionLocked = !_isRegionLocked;
    b->setToggleState(_isRegionLocked, dontSendNotification);
    repaint();
  }
  if (b->getName() == "delete") {
    // first ask for confirmation, since there's no undo
    AlertWindow alert("Delete Idea",
      "Are you sure you want to delete this idea? This action is not undoable.",
      AlertWindow::AlertIconType::WarningIcon);
    alert.addButton("OK", 1);
    alert.addButton("Cancel", 0);
    int result = alert.runModalLoop();

    if (result == 0) {
      return;
    }

    IdeaList* parent = dynamic_cast<IdeaList*>(getParentComponent());
    parent->deleteIdea(this);
    return;
  }
}

void Idea::textEditorTextChanged(TextEditor & e)
{
  if (e.getName() == "name") {
    Component::setName(e.getText());
    MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

    if (mc != nullptr) {
      mc->repaintRenderArea();
    }
  }
}

JSONNode Idea::toJSON()
{
  JSONNode root;
  root.set_name(getName().toStdString());

  root.push_back(JSONNode("idea_type", (int)_type));

  JSONNode bounds;
  bounds.set_name("bounds");
  bounds.push_back(JSONNode("topX", _focusArea.getTopLeft().getX()));
  bounds.push_back(JSONNode("topY", _focusArea.getTopLeft().getY()));
  bounds.push_back(JSONNode("botX", _focusArea.getBottomRight().getX()));
  bounds.push_back(JSONNode("botY", _focusArea.getBottomRight().getY()));

  root.push_back(bounds);
  root.push_back(JSONNode("image_name", getName().toStdString() + ".png"));
  root.push_back(JSONNode("region_lock", _isRegionLocked));

  return root;
}

Image Idea::getImage()
{
  return _src;
}

void Idea::setName(const String & newName)
{
  Component::setName(newName);
  _nameEntry.setText(newName, dontSendNotification);
}

void Idea::updateType()
{
  // TODO: at some point this should actually do something
}

Point<float> Idea::localToRelativeImageCoords(Point<float> pt)
{
  float x = pt.getX();
  float y = pt.getY();

  // height is reduced by 24
  y -= _headerSize;

  // determine image dimensions
  float scaleX = (float)getWidth() / _src.getWidth();
  float scaleY = (float)(getHeight() - _headerSize) / _src.getHeight();

  // account for letterboxing
  float scale = min(scaleX, scaleY);

  // x too big - extra space on the L/R sides
  if (scaleX > scale) {
    // determine magnitude of letterbox
    float actualScale = scale * _src.getWidth();
    float diff = abs(getWidth() - actualScale);
    float offset = diff / 2;
    x -= offset;
  }
  // y too big - extra space on Top/Bot sides
  else if (scaleY > scale) {
    float actualScale = scale * _src.getHeight();
    float diff = abs((getHeight() - _headerSize) - actualScale);
    float offset = diff / 2;
    y -= offset;
  }

  // at this point x and y should be adjusted to proper scale
  Point<float> scaledPt(Lumiverse::clamp(x / (scale * _src.getWidth()), 0, 1),
    Lumiverse::clamp(y / (scale * _src.getHeight()), 0, 1));

  return scaledPt;
}

Point<float> Idea::relativeImageCoordsToLocal(Point<float> pt)
{
  // scale up the point and put in absolute coordinates (within the image bounds)
  float x = pt.getX();
  float y = pt.getY();

  x *= _src.getWidth();
  y *= _src.getHeight();

  // determine image dimensions
  float scaleX = (float)getWidth() / _src.getWidth();
  float scaleY = (float)(getHeight() - _headerSize) / _src.getHeight();

  // account for letterboxing
  float scale = min(scaleX, scaleY);

  x *= scale;
  y *= scale;

  // x too big - extra space on the L/R sides
  if (scaleX > scale) {
    // determine magnitude of letterbox
    float actualScale = scale * _src.getWidth();
    float diff = abs(getWidth() - actualScale);
    float offset = diff / 2;
    x += offset;
  }
  // y too big - extra space on Top/Bot sides
  else if (scaleY > scale) {
    float intendedScale = scaleY * _src.getHeight();
    float actualScale = scale * _src.getHeight();
    float diff = abs((getHeight() - _headerSize) - actualScale);
    float offset = diff / 2;
    y += offset;
  }

  y += _headerSize;

  return Point<float>(x, y);
}

void Idea::initUI()
{
  _typeSelector.addItem("Color Palette", (int)COLOR_PALETTE);
  _typeSelector.addListener(this);
  _typeSelector.setSelectedId((int)_type);
  addAndMakeVisible(_typeSelector);

  _lock.setButtonText("L");
  _lock.addListener(this);
  _lock.setName("lock");
  _lock.setColour(TextButton::ColourIds::buttonOnColourId, Colours::darkred);
  addAndMakeVisible(_lock);

  _nameEntry.setText(getName());
  _nameEntry.addListener(this);
  _nameEntry.setName("name");
  _nameEntry.setColour(TextEditor::ColourIds::backgroundColourId, Colour(0x00333333));
  _nameEntry.setColour(TextEditor::ColourIds::textColourId, Colours::white);
  _nameEntry.setColour(TextEditor::ColourIds::outlineColourId, Colour(0x00333333));
  _nameEntry.setColour(TextEditor::ColourIds::focusedOutlineColourId, Colour(0xff606060));
  _nameEntry.setColour(TextEditor::ColourIds::highlightColourId, Colour(0xffa0a0a0));
  addAndMakeVisible(_nameEntry);

  _delete.setButtonText("x");
  _delete.setName("delete");
  _delete.addListener(this);
  addAndMakeVisible(_delete);

  _headerSize = 24 * 2;
}

IdeaList::IdeaList()
{
  _ideaID = 0;
}

IdeaList::~IdeaList()
{
  _ideas.clear();
}

void IdeaList::paint(Graphics & g)
{
  g.fillAll(Colour(0xff333333));
}

void IdeaList::resized()
{
  // the elements are all set to be squares with the width determininning the size
  // update the size first
  float side = getWidth();
  setSize(side, side * _ideas.size());

  auto lbounds = getLocalBounds();
  // position elements
  for (int i = 0; i < _ideas.size(); i++) {
    _ideas[i]->setBounds(lbounds.removeFromTop(side).reduced(2));
  }
}

shared_ptr<Idea> IdeaList::getActiveIdea()
{
  //out of bounds
  if (_activeIdea >= _ideas.size()) {
    return nullptr;
  }

  return _ideas[_activeIdea];
}

void IdeaList::clearActiveIdea()
{
  for (auto i : _ideas)
    i->_selected = false;
}

void IdeaList::updateActiveIdea()
{
  _activeIdea = 0;
  for (int i = 0; i < _ideas.size(); i++) {
    if (_ideas[i]->_selected)
      _activeIdea = i;
    _ideas[i]->repaint();
  }

  getGlobalSettings()->_activeIdea = getActiveIdea();

  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

  if (mc != nullptr) {
    mc->repaintRenderArea();
  }
}

void IdeaList::addIdea(Image i, String name)
{
  _ideas.push_back(shared_ptr<Idea>(new Idea(i)));
  _ideas[_ideas.size() - 1]->setName(name + String(_ideaID));
  addAndMakeVisible(_ideas[_ideas.size() - 1].get());
  clearActiveIdea();
  _ideas[_ideas.size() - 1]->_selected = true;
  updateActiveIdea();
  resized();

  _ideaID++;
}

void IdeaList::saveIdeas(File destFolder)
{
  // the data is contained within a JSON file that basically contains the entire
  // Idea object except the image (which is loaded from a filepath field in the
  // JSON node

  JSONNode root;

  // can't match by ID, have to load all ideas then match name to get active
  root.push_back(JSONNode("active_idea", getActiveIdea()->getName().toStdString()));

  for (auto i : _ideas) {
    // create json node
    JSONNode idea = i->toJSON();
    root.push_back(idea);

    // save image, overwrite
    File img = destFolder.getChildFile(i->getName() + ".png");
    img.deleteFile();
    FileOutputStream os(img);
    PNGImageFormat pngif;

    pngif.writeImageToStream(i->getImage(), os);
  }

  ofstream ideaFile;
  ideaFile.open(destFolder.getChildFile("data.ilib").getFullPathName().toStdString(), ios::out | ios::trunc);
  ideaFile << root.write_formatted();
}

void IdeaList::loadIdeas(File srcFolder)
{
  _ideas.clear();
  removeAllChildren();

  // load time
  // first find the json file
  // Check to see if we can load the file.
  ifstream data;
  data.open(srcFolder.getChildFile("data.ilib").getFullPathName().toStdString(),
    ios::in | ios::binary | ios::ate);

  if (data.is_open()) {
    // "+ 1" for the ending
    streamoff size = data.tellg();
    char* memblock = new char[(unsigned int)size + 1];

    data.seekg(0, ios::beg);

    data.read(memblock, size);
    data.close();

    // It's not guaranteed that the following memory after memblock is blank.
    // C-style string needs an end.
    memblock[size] = '\0';

    JSONNode n = libjson::parse(memblock);

    // iterate throught he json nodes and reconstruct the objects
    JSONNode::iterator i = n.begin();
    String active;

    while (i != n.end()) {
      // named params
      if (i->name() == "active_idea") {
        active = i->as_string();
      }
      // is an object
      else {
        shared_ptr<Idea> newIdea = shared_ptr<Idea>(new Idea(srcFolder, *i));
        addAndMakeVisible(newIdea.get());
        _ideas.push_back(newIdea);
      }

      i++;
    }

    // find the active idea
    for (int j = 0; j < _ideas.size(); j++) {
      if (_ideas[j]->getName() == active) {
        _activeIdea = j;
        _ideas[j]->_selected = true;
      }
    }
    
    updateActiveIdea();
    delete memblock;
  }

  resized();
}

void IdeaList::deleteIdea(Idea* idea)
{
  vector<shared_ptr<Idea>>::iterator todelete;
  for (auto it = _ideas.begin(); it != _ideas.end(); it++) {
    if (it->get() == idea) {
      todelete = it;
    }
  }

  _ideas.erase(todelete);
  resized();
}
