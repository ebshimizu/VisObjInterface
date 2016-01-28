/*
  ==============================================================================

    BrightAttribute.cpp
    Created: 20 Jan 2016 6:30:19pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "BrightAttribute.h"

//==============================================================================
BrightAttribute::BrightAttribute() : AttributeControllerBase("Bright")
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

BrightAttribute::BrightAttribute(String name) : AttributeControllerBase(name)
{
}

BrightAttribute::~BrightAttribute()
{
}

double BrightAttribute::evaluateScene(Device * key, Device * fill, Device * rim)
{
  // average brightness, now with color
  return (getLightIntens(key) + getLightIntens(fill) + getLightIntens(rim)) / 3;
}

double BrightAttribute::getLightIntens(Device * d)
{
  return d->getColor()->getLab(ReferenceWhite::D65)[0] * d->getIntensity()->asPercent();
}

