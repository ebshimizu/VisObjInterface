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
  AttributeControllerBase(String name);
  ~AttributeControllerBase();

  void paint (Graphics&);
  void resized();

  AttributeConstraint getStatus();

  virtual void buttonClicked(Button* b) override;

  // This function does a tiny preprocess step in identifying the key, fill, and
  // rim lights before calling the subclass-defined objective function.
	double evaluateScene(map<string, Device*> devices);

protected:
  // This function evaluates the current scene and returns a numeric value
  // indicating how much of the attribute is present in the scene.
  // NOTE: this may change to indicate relative strength (like >)
  // All attribute controls must implement this function
  virtual double evaluateScene(Device* key, Device* fill, Device* rim) = 0;

	// Attribute name
	String _name;

private:
	// Buttons and controls
  Array<TextButton*> _buttons;

  // Indicates which button is active for the search step
  AttributeConstraint _status;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeControllerBase)
};


#endif  // ATTRIBUTECONTROLLERBASE_H_INCLUDED
