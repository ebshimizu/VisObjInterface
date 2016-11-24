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

  _typeSelector.addItem("Color Palette", (int)COLOR_PALETTE);
  _typeSelector.addListener(this);
  _typeSelector.setSelectedId((int)_type);
  addAndMakeVisible(_typeSelector);

  _selected = false;
}

Idea::~Idea()
{
}

void Idea::paint(Graphics & g)
{
  if (_selected) {
    g.setColour(Colours::lightyellow);
    g.fillAll();
  }

  auto lbounds = getLocalBounds();
  lbounds.removeFromTop(24);

  g.drawImageWithin(_src, lbounds.getX(), lbounds.getY(), lbounds.getWidth(), lbounds.getHeight(), RectanglePlacement::centred);

  g.setColour(Colours::red);
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
  _typeSelector.setBounds(lbounds.removeFromTop(24).reduced(2));

  // image
}

void Idea::mouseDown(const MouseEvent & e)
{
  // uses right click drag to select regions
  if (e.mods.isRightButtonDown()) {
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
  if (e.mods.isRightButtonDown()) {
    _secondPt = e.position;
    repaint();
  }
}

void Idea::mouseUp(const MouseEvent & e)
{
  if (e.mods.isRightButtonDown()) {
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

void Idea::updateType()
{
  // TODO: at some point this should actually do something
}

Point<float> Idea::localToRelativeImageCoords(Point<float> pt)
{
  float x = pt.getX();
  float y = pt.getY();

  // height is reduced by 24
  y -= 24;

  // determine image dimensions
  float scaleX = (float)getWidth() / _src.getWidth();
  float scaleY = (float)(getHeight() - 24) / _src.getHeight();

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
    float diff = abs((getHeight() - 24) - actualScale);
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
  float scaleY = (float)(getHeight() - 24) / _src.getHeight();

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
    float diff = abs((getHeight() - 24) - actualScale);
    float offset = diff / 2;
    y += offset;
  }

  y += 24;

  return Point<float>(x, y);
}

IdeaList::IdeaList()
{
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

void IdeaList::addIdea(Image i)
{
  _ideas.push_back(shared_ptr<Idea>(new Idea(i)));
  addAndMakeVisible(_ideas[_ideas.size() - 1].get());
  clearActiveIdea();
  _ideas[_ideas.size() - 1]->_selected = true;
  updateActiveIdea();
  resized();
}
