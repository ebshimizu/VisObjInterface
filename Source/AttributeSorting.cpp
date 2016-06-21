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
#include "AttributeSearch.h"

using namespace Lumiverse;
using namespace Lumiverse::ShowControl;

int DefaultSorter::compareElements(SearchResultContainer * first, SearchResultContainer * second)
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

int AvgHueSorter::compareElements(SearchResultContainer * first, SearchResultContainer * second)
{
  // get hue from images
  Image i1 = first->getImage().rescaled(100, 100);
  Image i2 = second->getImage().rescaled(100, 100);

  double avgHue1 = 0;
  double avgHue2 = 0;

  for (int y = 0; y < i1.getHeight(); y++) {
    for (int x = 0; x < i1.getWidth(); x++) {
      avgHue1 += i1.getPixelAt(x, y).getHue();
      avgHue2 += i2.getPixelAt(x, y).getHue();
    }
  }

  avgHue1 /= (i1.getWidth() * i1.getHeight());
  avgHue2 /= (i2.getWidth() * i2.getHeight());

  if (avgHue1 < avgHue2)
    return -1;
  if (avgHue1 > avgHue2)
    return 1;
  else
    return 0;
}

int AvgBrightSorter::compareElements(SearchResultContainer * first, SearchResultContainer * second)
{
  // use average luminance in Lab
  Eigen::VectorXd s1 = first->getFeatures();
  Eigen::VectorXd s2 = second->getFeatures();

  double avgL1 = 0;
  double avgL2 = 0;

  for (int i = 0; i < s1.size() / 3; i++) {
    int idx = i * 3;
    avgL1 += s1[idx];
    avgL2 += s2[idx];
  }

  avgL1 /= (s1.size() / 3);
  avgL2 /= (s2.size() / 3);

  if (avgL1 < avgL2)
    return -1;
  if (avgL1 > avgL2)
    return 1;
  else
    return 0;
}
