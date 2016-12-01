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

int DefaultSorter::compareElements(shared_ptr<SearchResultContainer> first, shared_ptr<SearchResultContainer> second)
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

int AvgHueSorter::compareElements(shared_ptr<SearchResultContainer> first, shared_ptr<SearchResultContainer> second)
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

int AvgBrightSorter::compareElements(shared_ptr<SearchResultContainer> first, shared_ptr<SearchResultContainer> second)
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

int StyleSorter::compareElements(shared_ptr<SearchResultContainer> first, shared_ptr<SearchResultContainer> second)
{
  Snapshot* s1 = vectorToSnapshot(first->getSearchResult()->_scene);
  Snapshot* s2 = vectorToSnapshot(second->getSearchResult()->_scene);

  double sv1 = getStyleTerm(_s, s1, first->getImage());
  double sv2 = getStyleTerm(_s, s2, second->getImage());

  delete s1;
  delete s2;

  if (sv1 < sv2)
    return -1;
  if (sv1 > sv2)
    return 1;
  else
    return 0;
}

int CacheSorter::compareElements(shared_ptr<SearchResultContainer> first, shared_ptr<SearchResultContainer> second)
{
  if (first->_sortVals[_s] < second->_sortVals[_s])
    return -1;
  else if (first->_sortVals[_s] > second->_sortVals[_s])
    return 1;
  else
    return 0;
}

int ExtraSorter::compareElements(shared_ptr<SearchResultContainer> first, shared_ptr<SearchResultContainer> second)
{
  if (first->getSearchResult()->_extraFuncs[_key] < second->getSearchResult()->_extraFuncs[_key])
    return -1;
  else if (first->getSearchResult()->_extraFuncs[_key] > second->getSearchResult()->_extraFuncs[_key])
    return 1;
  else
    return 0;
}
