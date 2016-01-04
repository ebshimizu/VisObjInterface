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

  // This function evaluates the current scene and returns a numeric value
  // indicating how much of the attribute is present in the scene.
  // NOTE: this may change to indicate relative strength (like >)
	virtual double evaluateScene(set<Device*> devices) = 0;

protected:
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
