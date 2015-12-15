/*
  ==============================================================================

    MainMenu.h
    Created: 14 Dec 2015 2:30:48pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef MAINMENU_H_INCLUDED
#define MAINMENU_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class MainMenu    : public Component, public MenuBarModel
{
public:
  MainMenu();
  ~MainMenu();

  // ==========================================================================
  // MenuBarModel functions

  virtual StringArray getMenuBarNames();
  virtual PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName);
  virtual void menuItemSelected(int menuItemID, int topLevelMenuIndex);


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainMenu)
};


#endif  // MAINMENU_H_INCLUDED
