/*
  ==============================================================================

    StatusBar.h
    Created: 15 Dec 2015 4:21:58pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef STATUSBAR_H_INCLUDED
#define STATUSBAR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class StatusBar    : public Component
{
public:
  StatusBar();
  ~StatusBar();

  void paint(Graphics&);
  void resized();
  void setStatusMessage(String msg, bool error = false, bool warning = false);
  String getStatusMessage();
  bool isError();

private:
  String _currentText;
  bool _error;
  bool _warning;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatusBar)
};


#endif  // STATUSBAR_H_INCLUDED
