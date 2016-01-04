/*
  ==============================================================================

    AttributeControls.h
    Created: 15 Dec 2015 5:07:22pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef ATTRIBUTECONTROLS_H_INCLUDED
#define ATTRIBUTECONTROLS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

class AttributeControlsList : public Component
{
public:
  AttributeControlsList();
  ~AttributeControlsList();

  void setWidth(int width);

  void paint(Graphics& g);
  void resized();

  void addAttributeController(AttributeControllerBase* control);
  void removeAttributeController(string name);

private:
  int _width;
  int _height;

  int _componentHeight = 50;

  map<string, AttributeControllerBase*> _controls;
};

//==============================================================================
/*
*/
class AttributeControls : public Component
{
public:
  AttributeControls();
  ~AttributeControls();

  void paint (Graphics&);
  void resized();

private:
  AttributeControlsList* _container;
  Viewport* _componentView;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeControls)
};


#endif  // ATTRIBUTECONTROLS_H_INCLUDED
