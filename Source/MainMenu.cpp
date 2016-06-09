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
  const char* names[] = { "File", "Edit", "Explore", "Analyze", nullptr };
  return StringArray(names);
}

PopupMenu MainMenu::getMenuForIndex(int topLevelMenuIndex, const String& menuName) {
  ApplicationCommandManager* cm = getApplicationCommandManager();

  PopupMenu menu;

  if (topLevelMenuIndex == 0) {
    menu.addCommandItem(cm, command::OPEN);
    menu.addSeparator();
    menu.addCommandItem(cm, command::SAVE);
    menu.addCommandItem(cm, command::SAVE_AS);
    menu.addCommandItem(cm, command::SAVE_RENDER);
  }
  else if (topLevelMenuIndex == 1) {
    menu.addCommandItem(cm, command::ARNOLD_RENDER);
    menu.addCommandItem(cm, command::UNDO);
    menu.addCommandItem(cm, command::REDO);
    menu.addSeparator();
    menu.addCommandItem(cm, command::GET_FROM_ARNOLD);
    menu.addSeparator();
    menu.addCommandItem(cm, command::SETTINGS);
  }
  else if (topLevelMenuIndex == 2) {
    menu.addCommandItem(cm, command::SEARCH);
    menu.addCommandItem(cm, command::STOP_SEARCH);
    menu.addCommandItem(cm, command::RECLUSTER);
    menu.addSeparator();
    menu.addCommandItem(cm, command::UNLOCK_ALL);
    menu.addCommandItem(cm, command::LOCK_ALL_AREAS_EXCEPT);
    menu.addCommandItem(cm, command::LOCK_AREA);
    menu.addCommandItem(cm, command::LOCK_ALL_SYSTEMS_EXCEPT);
    menu.addCommandItem(cm, command::LOCK_SYSTEM);
    menu.addCommandItem(cm, command::LOCK_ALL_COLOR);
    menu.addCommandItem(cm, command::LOCK_ALL_INTENSITY);
    menu.addCommandItem(cm, command::LOCK_ALL_POSITION);
  }
  else if (topLevelMenuIndex == 3) {
    menu.addCommandItem(cm, command::SAVE_RESULTS);
    menu.addCommandItem(cm, command::LOAD_RESULTS);
    menu.addSeparator();
    menu.addCommandItem(cm, command::LOAD_TRACES);
    menu.addCommandItem(cm, command::PICK_TRACE);
  }

  return menu;
}

void MainMenu::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
  // Custom item handling, if needed.
}
