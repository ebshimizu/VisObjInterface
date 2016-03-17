/*
  ==============================================================================

    AttributeSorting.cpp
    Created: 16 Feb 2016 1:41:37pm
    Author:  falindrith

  ==============================================================================
*/

#include "AttributeSorting.h"
#include "LumiverseCore.h"
#include "LumiverseShowControl/LumiverseShowControl.h"

using namespace Lumiverse;
using namespace Lumiverse::ShowControl;

int DefaultSorter::compareElements(AttributeSearchResult * first, AttributeSearchResult * second)
{
  double firstScore = first->getSearchResult()->_objFuncVal;
  double secondScore = second->getSearchResult()->_objFuncVal;

  if (firstScore < secondScore)
    return -1;
  if (firstScore > secondScore)
    return 1;
  else
    return 0;
}

int AvgHueSorter::compareElements(AttributeSearchResult * first, AttributeSearchResult * second)
{
  Snapshot* firstScene = vectorToSnapshot(first->getSearchResult()->_scene);
  Snapshot* secondScene = vectorToSnapshot(second->getSearchResult()->_scene);

  double firstHue = 0;
  for (auto d : firstScene->getDevices()) {
    auto hsv = d->getParam<LumiverseColor>("color")->getHSV();
    firstHue += hsv[0];
  }
  firstHue /= firstScene->getDevices().size();

  double secondHue = 0;
  for (auto d : secondScene->getDevices()) {
    auto hsv = d->getParam<LumiverseColor>("color")->getHSV();
    secondHue += hsv[0];
  }
  secondHue /= secondScene->getDevices().size();

  delete firstScene;
  delete secondScene;

  if (firstHue < secondHue)
    return -1;
  if (firstHue > secondHue)
    return 1;
  else
    return 0;
}

int KeyHueSorter::compareElements(AttributeSearchResult * first, AttributeSearchResult * second)
{
  Snapshot* firstScene = vectorToSnapshot(first->getSearchResult()->_scene);
  Snapshot* secondScene = vectorToSnapshot(second->getSearchResult()->_scene);

  return 0;
}

int AvgBrightSorter::compareElements(AttributeSearchResult * first, AttributeSearchResult * second)
{
  Snapshot* firstScene = vectorToSnapshot(first->getSearchResult()->_scene);
  Snapshot* secondScene = vectorToSnapshot(second->getSearchResult()->_scene);

  double firstInt = 0;
  for (auto d : firstScene->getDevices()) {
    firstInt += d->getIntensity()->asPercent();
  }
  firstInt /= firstScene->getDevices().size();

  double secondInt = 0;
  for (auto d : secondScene->getDevices()) {
    secondInt += d->getIntensity()->asPercent();
  }
  secondInt /= secondScene->getDevices().size();

  delete firstScene;
  delete secondScene;

  if (firstInt < secondInt)
    return -1;
  if (firstInt > secondInt)
    return 1;
  else
    return 0;
}

int KeyBrightSorter::compareElements(AttributeSearchResult * first, AttributeSearchResult * second)
{
  Snapshot* firstScene = vectorToSnapshot(first->getSearchResult()->_scene);
  Snapshot* secondScene = vectorToSnapshot(second->getSearchResult()->_scene);

  return 0;
}

int KeyAzmSorter::compareElements(AttributeSearchResult * first, AttributeSearchResult * second)
{
  Snapshot* firstScene = vectorToSnapshot(first->getSearchResult()->_scene);
  Snapshot* secondScene = vectorToSnapshot(second->getSearchResult()->_scene);

  return 0;
}
