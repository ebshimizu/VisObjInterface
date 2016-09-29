/*
  ==============================================================================

    ColoredTextButton.cpp
    Created: 11 Jul 2016 10:45:49am
    Author:  eshimizu

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "ColoredTextButton.h"

//==============================================================================
ColoredTextButton::ColoredTextButton(String name, bool useColorNameAsText) : TextButton(name),
  _useColorNameAsText(useColorNameAsText)
{
  if (_useColorNameAsText)
    setButtonText(_buttonColor.toDisplayString(false));
}

ColoredTextButton::~ColoredTextButton()
{
}

void ColoredTextButton::paint (Graphics& g)
{
	LookAndFeel& lf = getLookAndFeel();

	lf.drawButtonBackground(g, *this,
		_buttonColor,
		isOver(), isDown());

	lf.drawButtonText(g, *this, isOver(), isDown());
}

void ColoredTextButton::setColor(Colour newColor)
{
	_buttonColor = newColor;

  if (_useColorNameAsText) {
    String colorDisp = "H: " + String((int)(_buttonColor.getHue() * 360.0)) + " S: " + String((int)(_buttonColor.getSaturation() * 100)) + " V: " + String((int)(_buttonColor.getBrightness() * 100));
    setButtonText(colorDisp);
  }
}
