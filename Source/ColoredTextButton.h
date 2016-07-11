/*
  ==============================================================================

    ColoredTextButton.h
    Created: 11 Jul 2016 10:45:49am
    Author:  eshimizu

  ==============================================================================
*/

#ifndef COLOREDTEXTBUTTON_H_INCLUDED
#define COLOREDTEXTBUTTON_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class ColoredTextButton    : public TextButton 
{
public:
	ColoredTextButton(String name);
	~ColoredTextButton();

	void paint (Graphics&) override;
	void setColor(Colour newColor);

private:
	Colour _buttonColor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColoredTextButton)
};


#endif  // COLOREDTEXTBUTTON_H_INCLUDED
