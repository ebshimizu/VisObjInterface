/*
  ==============================================================================

    TintAttribute.h
    Created: 1 Apr 2016 1:20:16pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef TINTATTRIBUTE_H_INCLUDED
#define TINTATTRIBUTE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

class TintAttribute : public AttributeControllerBase, public ChangeListener
{
public:
  TintAttribute();
  ~TintAttribute();

  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  virtual double evaluateScene(Snapshot* s) override;

  virtual void preProcess() override;

  virtual void mouseDown(const MouseEvent& e) override;

  virtual void changeListenerCallback(ChangeBroadcaster* source) override;

private:
  // Target color for a single run of the attribute
  Colour _targetColor;

  // Calculates tint for whole scene (difference of average color from target color)
  unordered_map<string, double> _weights;

  // area that gets painted the current color and that allows the color to be changed
  Rectangle<int> _colorSelect;
};



#endif  // TINTATTRIBUTE_H_INCLUDED
