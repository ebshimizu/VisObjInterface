/*
  ==============================================================================

    AttributeControllerBase.h
    Created: 4 Jan 2016 1:50:03pm
    Author:  Evan

  ==============================================================================
*/

#ifndef ATTRIBUTECONTROLLERBASE_H_INCLUDED
#define ATTRIBUTECONTROLLERBASE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "globals.h"

//==============================================================================
/*
*/
class AttributeControllerBase : public Component, public Button::Listener
{
public:
  AttributeControllerBase(String name, int w = 100, int h = 100);
  ~AttributeControllerBase();

  virtual void paint (Graphics&);
  virtual void resized();

  AttributeConstraint getStatus();
  void setStatus(AttributeConstraint c);

  virtual void buttonClicked(Button* b) override;

  // This function is implemented by every subclass.
  // The return value should be the strenght of the attribute in ths scene.
	virtual double evaluateScene(Snapshot* s, Image& i) = 0;

  // An optional pre-processing step is available for each attribute to perform
  // when the application starts.
  virtual void preProcess() { };

  // Set of parameters that automatically get locked for the evaluation of this attribute
  // allows the search to proceed more quickly though the relevant spaces
  set<EditParam> _autoLockParams;

  // popualtes the image field of the histogram attribute
  Image generateImage(Snapshot* s);
  Image generateImage(Snapshot* s, int w, int h);

  // for integration of non-semantic attributes into the system
  virtual bool isNonSemantic() { return false; }
  virtual list<Snapshot*> nonSemanticSearch() { return list<Snapshot*>(); }

protected:
	// Attribute name
	String _name;

  // size of the canonical image for the given attributre
  int _canonicalHeight;
  int _canonicalWidth;

private:
	// Buttons and controls
  Array<TextButton*> _buttons;

  // Indicates which button is active for the search step
  AttributeConstraint _status;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeControllerBase)
};


#endif  // ATTRIBUTECONTROLLERBASE_H_INCLUDED
