/*
  ==============================================================================

    TestAttribute.cpp
    Created: 4 Jan 2016 3:10:38pm
    Author:  Evan

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "TestAttribute.h"

//==============================================================================
TestAttribute::TestAttribute() : AttributeControllerBase("Test (Warm)")
{
}

TestAttribute::TestAttribute(String name) : AttributeControllerBase(name)
{
}

TestAttribute::~TestAttribute()
{
}

double TestAttribute::evaluateScene(set<Device*> devices)
{
  // DEBUG: TEMPORARY
  return 1;
}
