/*
  ==============================================================================

    SaturationAttribute.h
    Created: 31 Mar 2016 10:48:48am
    Author:  falindrith

  ==============================================================================
*/

#ifndef SATURATIONATTRIBUTE_H_INCLUDED
#define SATURATIONATTRIBUTE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"

// Saturation here is what we'll call a non-semantic attribute. What this means
// is that we have a very well-defined way to increase/decrease the value of the
// attribute in the entire scene, and thus the semantic search is mostly useless.
// You can still use it in semantic searches though, in which case it still defines
// an attribute function.
class SaturationAttribute : public AttributeControllerBase
{
public:
  SaturationAttribute();
  ~SaturationAttribute();

  virtual double evaluateScene(Snapshot* s) override;

  virtual void preProcess() override;

  virtual bool isNonSemantic() { return true; }

  // Returns a set of scenes that modify the current scene in a very specific
  // non-senmantic way. Here we just adjust the saturation of all lights.
  virtual list<Snapshot*> nonSemanticSearch() override;

private:
  unordered_map<string, double> _weights;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SaturationAttribute);
};




#endif  // SATURATIONATTRIBUTE_H_INCLUDED
