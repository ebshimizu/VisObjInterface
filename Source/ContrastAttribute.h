/*
  ==============================================================================

    ContrastAttribute.h
    Created: 29 Feb 2016 11:25:00am
    Author:  falindrith

  ==============================================================================
*/

#ifndef CONTRASTATTRIBUTE_H_INCLUDED
#define CONTRASTATTRIBUTE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

//==============================================================================
/*
At present there are a few issues with how the contrast attribute works, primarily
the issue of trying to calculate contrast without actually using pixel values.

This can probably be remedied with a neural net but for speed a basic version
has been implemented that compares weighted brightnesses.
*/
class ContrastAttribute : public AttributeControllerBase
{
public:
  ContrastAttribute(string area1, string area2);
  ~ContrastAttribute();

  virtual double evaluateScene(Snapshot* s) override;

  virtual void preProcess() override;

private:
  unordered_map<string, double> _area1Weights;
  unordered_map<string, double> _area2Weights;
  string _area1;
  string _area2;
  double _area1Sum;
  double _area2Sum;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ContrastAttribute)

};

#endif  // CONTRASTATTRIBUTE_H_INCLUDED
