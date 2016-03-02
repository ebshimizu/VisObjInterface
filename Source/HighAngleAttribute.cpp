/*
  ==============================================================================

    HighAngleAttribute.cpp
    Created: 29 Feb 2016 11:26:28am
    Author:  falindrith

  ==============================================================================
*/

#include "HighAngleAttribute.h"

HighAngleAttribute::HighAngleAttribute() : AttributeControllerBase("High Angle")
{
}

HighAngleAttribute::HighAngleAttribute(String name) : AttributeControllerBase(name)
{
}

HighAngleAttribute::~HighAngleAttribute()
{
}

double HighAngleAttribute::evaluateScene(Device * key, Device * fill, Device * rim)
{
  double targetAngle = 0.5416;
  
  double keyAngle = key->getParam<LumiverseOrientation>("polar")->asPercent();
  double fillAngle = fill->getParam<LumiverseOrientation>("polar")->asPercent();

  return (1 - abs(targetAngle - fillAngle)) * 50 + (1 - abs(targetAngle - keyAngle)) * 50;
}
