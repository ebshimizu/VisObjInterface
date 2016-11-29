/*
  ==============================================================================

    Idea.cpp
    Created: 23 Nov 2016 2:19:45pm
    Author:  falindrith

  ==============================================================================
*/

#include "Idea.h"
#include "MainComponent.h"
#include "ImageAttribute.h"
#include "KMeans.h"
#include "hsvPicker.h"

Idea::Idea(Image src, IdeaType type) : _src(src), _type(type)
{
  initUI();
  updateType();

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

  // colors
  JSONNode::iterator c = data.find("colors");
  if (c != data.end()) {
    JSONNode::iterator cit = c->begin();
    while (cit != c->end()) {
      // color load
      Eigen::Vector3d color(cit->find("hue")->as_float(), cit->find("sat")->as_float(), cit->find("val")->as_float());
      _colors.push_back(color);
      _weights.push_back(cit->find("weight")->as_float());

      cit++;
    }

    updateType(true);
  }
  else {
    // full recompute, data not stored
    updateType();
  }
}

Idea::~Idea()
{
  if (_colorControls != nullptr) {
    delete _colorControls;
  }
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

  if (_type == COLOR_PALETTE) {
    // they might be null for some reason
    if (_colorControls != nullptr) {
      _colorControls->setBounds(lbounds.removeFromTop(40));
    }
  }

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

    // recompute necessary idea info
    updateType();

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

  // colors
  JSONNode colors;
  colors.set_name("colors");
  for (int i = 0; i < _colors.size(); i++) {
    JSONNode color;
    color.set_name(String(i).toStdString());
    color.push_back(JSONNode("hue", _colors[i][0]));
    color.push_back(JSONNode("sat", _colors[i][1]));
    color.push_back(JSONNode("val", _colors[i][2]));
    color.push_back(JSONNode("weight", _weights[i]));
    colors.push_back(color);
  }
  root.push_back(colors);

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

vector<Eigen::Vector3d> Idea::getColors()
{
  return _colors;
}

vector<float> Idea::getWeights()
{
  return _weights;
}

IdeaType Idea::getType()
{
  return _type;
}

void Idea::updateType(bool skipRecompute)
{
  if (_type == COLOR_PALETTE) {
    // generate color palette if not locked
    if (!_isRegionLocked) {
      if (!skipRecompute) {
        generateColorPalette();
      }
    }

    // update ui
    if (_colorControls == nullptr) {
      _colorControls = new ColorPaletteControls(this);
      addAndMakeVisible(_colorControls);
      _headerSize = 24 * 2 + 40;
    }
  }
  else {
    if (_colorControls != nullptr) {
      delete _colorControls;
      _colorControls = nullptr;
    }
  }
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

Rectangle<int> Idea::relativeToAbsoluteImageRegion(Rectangle<float> rect)
{
  return Rectangle<int>(rect.getX() * _src.getWidth(), rect.getY() * _src.getHeight(),
    rect.getWidth() * _src.getWidth(), rect.getHeight() * _src.getHeight());
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
  _numColors = 5;

  _colorControls = nullptr;
}

void Idea::generateColorPalette()
{
  _colors.clear();
  _weights.clear();

  // for now we'll do the really simple thing and just run k-means on this thing.
  // k = 5 for now

  // compute a HSV histogram and pull out stats about the intensity and color
  SparseHistogram imgHist(3, { 0, 10, 0, 0.2f, 0, 0.2f });

  // pull out the selected image region
  Image img = _src.getClippedImage(relativeToAbsoluteImageRegion(_focusArea));
  double scale = (img.getHeight() > img.getWidth()) ? 100.0 / img.getHeight() : 100.0 / img.getWidth();
  Image i = img.rescaled(img.getWidth() * scale, img.getHeight() * scale);

  vector<pair<Eigen::VectorXd, int> > pts;

  for (int y = 0; y < i.getHeight(); y++) {
    for (int x = 0; x < i.getWidth(); x++) {
      auto p = i.getPixelAt(x, y);
      Eigen::VectorXf hsv;
      hsv.resize(3);

      p.getHSB(hsv[0], hsv[1], hsv[2]);

      Eigen::VectorXd pt;
      pt.resize(3);
      pt[0] = hsv[0];
      pt[1] = hsv[1];
      pt[2] = hsv[2];
      pts.push_back(pair<Eigen::VectorXd, int>(pt, 0));
    }
  }

  // cluster 
  GenericKMeans cluster;
  auto centers = cluster.cluster(_numColors, pts, InitMode::FORGY);

  // use the centers to create the distribution
  for (auto c : centers) {
    _colors.push_back(c);
  }

  updateColorWeights();
}

void Idea::altGenerateColorPalette()
{
  // step 0: check that all required dirs exist
  File temp = getGlobalSettings()->_tempDir;
  if (!temp.getChildFile("saliency").exists())
    temp.getChildFile("saliency").createDirectory();

  if (!temp.getChildFile("segments").exists())
    temp.getChildFile("segments").createDirectory();

  if (!temp.getChildFile("tmp").exists())
    temp.getChildFile("tmp").createDirectory();

  // step 1: save an image to the temp working directory
  String outPath = "";
  {
    File out = temp.getChildFile(getName() + ".png");
    FileOutputStream os(out);
    PNGImageFormat pngif;

    pngif.writeImageToStream(_src.getClippedImage(relativeToAbsoluteImageRegion(_focusArea)), os);
    os.flush();
    outPath = out.getFullPathName();
  }

  // step 2: cd to the app folder and run the matlab command to generate images
  File scriptDir = getGlobalSettings()->_paletteAppDir;
  String cmd = "matlab -r -nosplash -nodesktop -wait \"cd('" + scriptDir.getFullPathName() + "');";
  cmd += "generateImages('" + outPath + "', '" + getName() + "', '" + temp.getFullPathName() + "/');exit;\"";
  system(cmd.toStdString().c_str());

  // step 3: run the palette generation command
  File result = temp.getChildFile(getName() + "_" + String(_numColors) + ".colors");
  result.deleteFile();
  File paletteGen = scriptDir.getChildFile("getPalette.exe");
  cmd = paletteGen.getFullPathName() + " " + temp.getFullPathName() + " " + scriptDir.getChildFile("weights").getFullPathName();
  cmd += " " + scriptDir.getChildFile("c3_data.json").getFullPathName() + " " + getName() + ".png ";
  cmd += result.getFullPathName() + " " + String(_numColors);
  system(cmd.toStdString().c_str());

  // step 4: read color file
  String colorString = result.loadFileAsString();
  _colors.clear();

  // colors are rgb separated by a space
  int start = 0;
  int end = 0;
  while (start <= colorString.length()) {
    end = colorString.indexOf(start, " ");
    if (end == -1)
      end = colorString.length();

    String substr = colorString.substring(start, end);

    // parse numbers, separated by ,
    int R = substr.upToFirstOccurrenceOf(",", false, true).getIntValue();
    substr = substr.fromFirstOccurrenceOf(",", false, true);

    int G = substr.upToFirstOccurrenceOf(",", false, true).getIntValue();
    substr = substr.fromFirstOccurrenceOf(",", false, true);

    int B = substr.getIntValue();

    Colour c(R, G, B);

    Eigen::VectorXf hsv;
    hsv.resize(3);
    c.getHSB(hsv[0], hsv[1], hsv[2]);

    Eigen::VectorXd pt;
    pt.resize(3);
    pt[0] = hsv[0];
    pt[1] = hsv[1];
    pt[2] = hsv[2];

    _colors.push_back(pt);

    start = end + 1;
  }

  updateColorWeights();

  // step 5: show in interface
  // that actually happens not in this function 
}

void Idea::updateColorWeights()
{
    // palettize a small, scaled image
  Image img = _src.getClippedImage(relativeToAbsoluteImageRegion(_focusArea));
  double scale = (img.getHeight() > img.getWidth()) ? 100.0 / img.getHeight() : 100.0 / img.getWidth();
  Image scaled = img.rescaled(img.getWidth() * scale, img.getHeight() * scale);

  vector<int> counts;
  counts.resize(_colors.size());

  for (auto y = 0; y < scaled.getHeight(); y++) {
    for (auto x = 0; x < scaled.getWidth(); x++) {
      // find closest color
      Colour px = scaled.getPixelAt(x, y);
      
      Eigen::VectorXd pt;
      pt.resize(3);
      pt[0] = px.getRed() / 255.0;
      pt[1] = px.getGreen() / 255.0;
      pt[2] = px.getBlue() / 255.0;

      double min = DBL_MAX;
      int minIdx = 0;
      for (int i = 0; i < _colors.size(); i++) {
        Colour toRGB((float)_colors[i][0], (float)_colors[i][1], (float)_colors[i][2], 1.0f);
        Eigen::VectorXd rgb;
        rgb.resize(3);
        rgb[0] = toRGB.getRed() / 255.0;
        rgb[1] = toRGB.getGreen() / 255.0;
        rgb[2] = toRGB.getBlue() / 255.0;

        double dist = (pt - rgb).norm();
        if (dist < min) {
          min = dist;
          minIdx = i;
        }
      }

      counts[minIdx] += 1;
    }
  }

  _weights.clear();
  _weights.resize(_colors.size());
  for (int i = 0; i < counts.size(); i++) {
    _weights[i] = (float)counts[i] / (float)(scaled.getHeight() * scaled.getWidth());
  }
}

Idea::ColorPaletteControls::ColorPaletteControls(Idea* parent) : _parent(parent)
{
}

Idea::ColorPaletteControls::~ColorPaletteControls()
{
}

void Idea::ColorPaletteControls::paint(Graphics & g)
{
  if (_parent->_colors.size() > 0) {
    // draws color rectangles
    auto lbounds = getLocalBounds();
    int elemWidth = lbounds.getWidth() / _parent->_colors.size();

    for (auto c : _parent->_colors) {
      auto region = lbounds.removeFromLeft(elemWidth).reduced(2);

      Colour dc((float)c[0], (float)c[1], (float)c[2], 1.0f);
      g.setColour(dc);
      g.fillRect(region);
    }
  }
}

void Idea::ColorPaletteControls::resized()
{
}

void Idea::ColorPaletteControls::changeListenerCallback(ChangeBroadcaster * source)
{
  HSVPicker* p = dynamic_cast<HSVPicker*>(source);
  
  if (p != nullptr) {
    // modify the color
    Colour c = p->getCurrentColour();

    // convert to eigen vector
    Eigen::Vector3f hsv;
    c.getHSB(hsv[0], hsv[1], hsv[2]);

    _parent->_colors[_selectedColorId][0] = hsv[0];
    _parent->_colors[_selectedColorId][1] = hsv[1];
    _parent->_colors[_selectedColorId][2] = hsv[2];
    repaint();
  }
}

void Idea::ColorPaletteControls::mouseDown(const MouseEvent & e)
{
  // if we're not locked
  if (!_parent->_isRegionLocked) {
    if (e.mods.isRightButtonDown()) {
      showExtraOptions();
    }
    else {
      // popup a color selector at the proper spot.
      auto lbounds = getLocalBounds();
      int elemWidth = lbounds.getWidth() / _parent->_colors.size();

      int idx = 0;
      _selectedColorId = -1;
      Rectangle<int> selectedArea;
      for (auto c : _parent->_colors) {
        auto region = lbounds.removeFromLeft(elemWidth).reduced(2);

        if (region.contains(e.position.getX(), e.position.getY())) {
          _selectedColorId = idx;
          selectedArea = region;
        }

        idx++;
      }

      // popup a selector
      if (_selectedColorId != -1) {
        Eigen::Vector3d selected = _parent->_colors[_selectedColorId];
        Colour current((float)selected[0], (float)selected[1], (float)selected[2], 1.0f);

        HSVPicker* cs = new HSVPicker();
        cs->setName("Constraint");
        cs->setCurrentColour(current);
        cs->setSize(200, 300);
        cs->addChangeListener(this);

        auto screenBounds = this->getScreenBounds();
        screenBounds.setX(screenBounds.getX() + selectedArea.getX());
        screenBounds.setWidth(selectedArea.getWidth());
        CallOutBox::launchAsynchronously(cs, screenBounds, nullptr);
      }
    }
  }
}

void Idea::ColorPaletteControls::showExtraOptions()
{
  PopupMenu m;
  m.addItem(1, "Reset Colors");
  m.addItem(2, "Change Number of Colors");
  m.addItem(3, "Edit Weights");
  m.addItem(4, "Recompute Weights");

  int result = m.show();

  if (result == 1) {
    _parent->updateType();
  }
  else if (result == 2) {
    // dialog box for changing number of colors
    AlertWindow w("Set Number of Colors",
      "Choose number of colors to use in Palette",
      AlertWindow::QuestionIcon);

    w.addTextEditor("colors", String(_parent->_numColors), "Number of Colors:");
    w.getTextEditor("colors")->setInputRestrictions(3, "0123456789");

    w.addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
    w.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));

    if (w.runModalLoop() != 0) // is they picked 'ok'
    {
      // this is the text they entered..
      String text = w.getTextEditorContents("colors");
      _parent->_numColors = text.getIntValue();
      _parent->updateType();
      repaint();
    }
  }
  else if (result == 3) {
    // dialog for changing weightsw
    AlertWindow w("Edit Weights",
      "Edit the relative frequency of each color. Weights do not have to sum to 1.",
      AlertWindow::QuestionIcon);

    for (int i = 0; i < _parent->_weights.size(); i++) {
      w.addTextEditor(String(i), String(_parent->_weights[i]), "Color " + String(i));
      w.getTextEditor(String(i))->setInputRestrictions(20, "1234567890.");
    }

    w.addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
    w.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));

    if (w.runModalLoop() != 0) // is they picked 'ok'
    {
      // adjust all weights
      for (int i = 0; i < _parent->_weights.size(); i++) {
        String val = w.getTextEditorContents(String(i));
        _parent->_weights[i] = val.getDoubleValue();
      }
      repaint();
    }
  }
  else if (result == 4) {
    _parent->updateColorWeights();
  }
}

// ============================================================================

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
  shared_ptr<Idea> newIdea = shared_ptr<Idea>(new Idea(i));
  _ideas.push_back(newIdea);
  newIdea->setName(name + String(_ideaID));
  addAndMakeVisible(newIdea.get());

  clearActiveIdea();
  newIdea->_selected = true;  
  updateActiveIdea();

  getGlobalSettings()->_ideaMap[newIdea] = Rectangle<float>(0, 0, 1, 1);
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

  // save stage regions
  root.push_back(JSONNode("current_file", getGlobalSettings()->_showName.toStdString()));

  JSONNode regions;
  regions.set_name("idea_map");
  for (auto i : getGlobalSettings()->_ideaMap) {
    // save each region
    JSONNode rect;
    rect.set_name(i.first->getName().toStdString());
    rect.push_back(JSONNode("topX", i.second.getTopLeft().getX()));
    rect.push_back(JSONNode("topY", i.second.getTopLeft().getY()));
    rect.push_back(JSONNode("botX", i.second.getBottomRight().getX()));
    rect.push_back(JSONNode("botY", i.second.getBottomRight().getY()));

    regions.push_back(rect);
  }
  root.push_back(regions);

  // pins
  JSONNode pins;
  pins.set_name("pins");
  int i = 1;
  for (auto p : getGlobalSettings()->_pinnedRegions) {
    JSONNode rect;

    rect.set_name(String(i).toStdString());
    rect.push_back(JSONNode("topX", p.getTopLeft().getX()));
    rect.push_back(JSONNode("topY", p.getTopLeft().getY()));
    rect.push_back(JSONNode("botX", p.getBottomRight().getX()));
    rect.push_back(JSONNode("botY", p.getBottomRight().getY()));

    pins.push_back(rect);
  }
  root.push_back(pins);

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

    JSONNode ideaMap;

    while (i != n.end()) {
      // named params
      if (i->name() == "active_idea") {
        active = i->as_string();
      }
      else if (i->name() == "current_file") {
        string show = i->as_string();
        if (String(show) != getGlobalSettings()->_showName) {
          getStatusBar()->setStatusMessage("Ideas created for different rig. Idea and pin regions may not be placed correctly.", false, true);
        }
      }
      else if (i->name() == "idea_map") {
        // load this later
        ideaMap = *i;
      }
      else if (i->name() == "pins") {
        // load pins now
        loadPins(*i);
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
    
    // now load the idea map
    loadIdeaMap(ideaMap);

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

  getGlobalSettings()->_ideaMap.erase(*todelete);

  _ideas.erase(todelete);
  resized();

  MainContentComponent* mc = dynamic_cast<MainContentComponent*>(getAppMainContentWindow()->getContentComponent());

  if (mc != nullptr) {
    mc->repaintRenderArea();
  }
}

vector<shared_ptr<Idea>> IdeaList::getIdeas()
{
  return _ideas;
}

void IdeaList::loadPins(JSONNode pins)
{
  // load the pins
  getGlobalSettings()->_pinnedRegions.clear();
  JSONNode::iterator i = pins.begin();

  while (i != pins.end()) {
    getGlobalSettings()->_pinnedRegions.add(Rectangle<float>::leftTopRightBottom(
      i->find("topX")->as_float(), i->find("topY")->as_float(),
      i->find("botX")->as_float(), i->find("botY")->as_float()));

    i++;
  }
}

void IdeaList::loadIdeaMap(JSONNode ideaMap)
{
  getGlobalSettings()->_ideaMap.clear();
  JSONNode::iterator i = ideaMap.begin();

  while (i != ideaMap.end()) {
    // find idea with corresponding name
    String name = i->name();
    shared_ptr<Idea> selected = nullptr;

    for (auto idea : _ideas) {
      if (idea->getName() == name) {
        selected = idea;
      }
    }

    if (selected != nullptr) {
      // create the rectangle
      auto region = Rectangle<float>::leftTopRightBottom(
        i->find("topX")->as_float(), i->find("topY")->as_float(),
        i->find("botX")->as_float(), i->find("botY")->as_float());

      getGlobalSettings()->_ideaMap[selected] = region;
    }

    i++;
  }
}
