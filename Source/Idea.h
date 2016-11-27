/*
  ==============================================================================

    Idea.h
    Created: 23 Nov 2016 2:19:45pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef IDEA_H_INCLUDED
#define IDEA_H_INCLUDED

#include "globals.h"
#include "../JuceLibraryCode/JuceHeader.h"

enum IdeaType {
  COLOR_PALETTE = 1
};

class IdeaList;

// the Idea class contains information to apply an idea to a particular
// set of devices on the stage
class Idea : public Component, public ComboBoxListener,
  public ButtonListener, public TextEditorListener
{
public:
  Idea(Image src, IdeaType type = COLOR_PALETTE);
  Idea(File srcFolder, JSONNode data);
  ~Idea();

  void paint(Graphics& g) override;
  void resized() override;
 
  void mouseDown(const MouseEvent& e) override;
  void mouseDrag(const MouseEvent& e) override;
  void mouseUp(const MouseEvent& e) override;

  void comboBoxChanged(ComboBox* b) override;
  void buttonClicked(Button* b) override;
  void textEditorTextChanged(TextEditor& e) override;

  // converts the object to json
  JSONNode toJSON();

  // gets the image object
  Image getImage();

  // indicates if this element is selected
  bool _selected;

  void setName(const String& newName) override;

private:
  // source image
  Image _src;

  // idea type
  IdeaType _type;

  // graphical elements
  ComboBox _typeSelector;

  // Locks down the region selction to prevent accidental adjustments
  TextButton _lock;

  // Field for editing the name of the idea
  TextEditor _nameEntry;

  // deletes the idea and removes it from the idea list
  TextButton _delete;

  // bbox representing the area of the image the idea comes from
  Rectangle<float> _focusArea;

  // size of elements taking up the header of the idea.
  int _headerSize;

  // variables for rectangle area selection
  // the active points are stored in absolute coordinates for ease of drawing.
  Point<float> _firstPt;
  Point<float> _secondPt;
  bool _isBeingDragged;
  bool _isRegionLocked;

  // takes the current type and updates necesssary data for the Idea to function
  void updateType();

  // takes a point within this element and returns a location within the source image.
  // If out of bounds, clamps to [0,1]
  Point<float> localToRelativeImageCoords(Point<float> pt);
  Point<float> relativeImageCoordsToLocal(Point<float> pt);

  void initUI();
};

class IdeaList : public Component {
public:
  IdeaList();
  ~IdeaList();

  void paint(Graphics& g) override;
  void resized() override;

  shared_ptr<Idea> getActiveIdea();

  void clearActiveIdea();
  void updateActiveIdea();

  // adds an idea to the list.
  void addIdea(Image i, String name);

  // serialization function. Saves all ideas to a specified folder
  void saveIdeas(File destFolder);

  // loads ideas from disk.
  void loadIdeas(File srcFolder);

private:
  // list of ideas contained
  vector<shared_ptr<Idea> > _ideas;

  // current active idea
  int _activeIdea;

  // counter for uniquely identifying Ideas
  int _ideaID;
};


#endif  // IDEA_H_INCLUDED
