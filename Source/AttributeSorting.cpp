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

  Device* d1 = getSpecifiedDevice(L_KEY, firstScene);
  Device* d2 = getSpecifiedDevice(L_KEY, secondScene);

  double d1Hue = d1->getColor()->getHSV()[0];
  double d2Hue = d2->getColor()->getHSV()[0];

  delete firstScene;
  delete secondScene;

  if (d1Hue < d2Hue)
    return -1;
  if (d1Hue > d2Hue)
    return 1;
  else
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

  Device* d1 = getSpecifiedDevice(L_KEY, firstScene);
  Device* d2 = getSpecifiedDevice(L_KEY, secondScene);

  double d1Int = d1->getIntensity()->asPercent();
  double d2Int = d2->getIntensity()->asPercent();

  delete firstScene;
  delete secondScene;

  if (d1Int < d2Int)
    return -1;
  if (d1Int > d2Int)
    return 1;
  else
    return 0;
}

int KeyAzmSorter::compareElements(AttributeSearchResult * first, AttributeSearchResult * second)
{
  Snapshot* firstScene = vectorToSnapshot(first->getSearchResult()->_scene);
  Snapshot* secondScene = vectorToSnapshot(second->getSearchResult()->_scene);

  Device* d1 = getSpecifiedDevice(L_KEY, firstScene);
  Device* d2 = getSpecifiedDevice(L_KEY, secondScene);

  double d1Azm = d1->getParam<LumiverseOrientation>("azimuth")->asPercent();
  double d2Azm = d2->getParam<LumiverseOrientation>("azimuth")->asPercent();

  delete firstScene;
  delete secondScene;

  if (d1Azm < d2Azm)
    return -1;
  if (d1Azm > d2Azm)
    return 1;
  else
    return 0;
}
