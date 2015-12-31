/*
  ==============================================================================

    MainMenu.cpp
    Created: 14 Dec 2015 2:30:48pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainMenu.h"
#include "globals.h"

//==============================================================================
MainMenu::MainMenu()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

MainMenu::~MainMenu()
{
}

StringArray MainMenu::getMenuBarNames() {
  const char* names[] = { "File", "Edit", "Render", nullptr };
  return StringArray(names);
}

PopupMenu MainMenu::getMenuForIndex(int topLevelMenuIndex, const String& menuName) {
  ApplicationCommandManager* cm = getApplicationCommandManager();

  PopupMenu menu;

  if (topLevelMenuIndex == 0) {
    menu.addCommandItem(cm, command::OPEN);
  }
  else if (topLevelMenuIndex == 1) {
    // Edit
  }
  else if (topLevelMenuIndex == 2) {
    menu.addCommandItem(cm, command::ARNOLD_RENDER);
    menu.addSeparator();
    menu.addCommandItem(cm, command::RENDER_SETTINGS);
  }

  return menu;
}

void MainMenu::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
  // Custom item handling, if needed.
}
