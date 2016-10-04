/*
  ==============================================================================

    StatusBar.cpp
    Created: 15 Dec 2015 4:21:58pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "StatusBar.h"
#include "LumiverseCore.h"

using namespace Lumiverse;

//==============================================================================
StatusBar::StatusBar()
{
  _currentText = "Lighting Attribute Interface v" + String(JUCE_APP_VERSION);
  _error = false;
}

StatusBar::~StatusBar()
{
}

void StatusBar::paint (Graphics& g)
{
  if (_error)
    g.fillAll(Colour(0xff8C0000));
  else if (_warning)
    g.fillAll(Colour(Colours::darkgoldenrod));
  else
    g.fillAll(Colour(0xff333333));

  g.setColour(Colours::white);
  g.drawHorizontalLine(0, 0.0, (float) getLocalBounds().getWidth());

  g.setColour(Colours::white);
  g.setFont(Font(12.0f));
  g.drawText(_currentText, getLocalBounds().reduced(10, 2), Justification::left, true);
}

void StatusBar::resized()
{
}

void StatusBar::setStatusMessage(String msg, bool error, bool warning)
{
  _error = error;
  _warning = warning;
  _currentText = msg;

  Lumiverse::Logger::log((error) ? ERR : (warning) ? WARN : LDEBUG, msg.toStdString());
  repaint();
}

String StatusBar::getStatusMessage()
{
  return _currentText;
}

bool StatusBar::isError()
{
  return _error;
}
