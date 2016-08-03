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
#include "HistogramAttribute.h"

class TintAttribute : public HistogramAttribute, public ChangeListener
{
public:
  TintAttribute();
  TintAttribute(int w, int h);
  ~TintAttribute();

  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  virtual double evaluateScene(Snapshot* s, Image& img) override;

  virtual void mouseDown(const MouseEvent& e) override;

  virtual void changeListenerCallback(ChangeBroadcaster* source) override;

private:
  // Target color for a single run of the attribute
  Colour _targetColor;

  // area that gets painted the current color and that allows the color to be changed
  juce::Rectangle<int> _colorSelect;
};



#endif  // TINTATTRIBUTE_H_INCLUDED
