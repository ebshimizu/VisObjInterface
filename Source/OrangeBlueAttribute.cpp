/*
  ==============================================================================

    OrangeBlueAttribute.cpp
    Created: 26 Apr 2016 4:55:57pm
    Author:  falindrith

  ==============================================================================
*/

#include "OrangeBlueAttribute.h"

OrangeBlueAttribute::OrangeBlueAttribute(int w, int h) : HistogramAttribute("Orange Blue", w, h)
{
  _targetBlue = 177;
  _targetOrange = 32;
}

OrangeBlueAttribute::~OrangeBlueAttribute()
{
}

double OrangeBlueAttribute::evaluateScene(Snapshot * s)
{
  Image i = generateImage(s);

  // Target hue values
  // blue: 177
  // orange: 32
  Histogram1D hue = getHueHist(i, 360);
  Histogram1D hue2 = hue.consolidate(2);

  int c1 = hue2.getNthBin(0);
  int c2 = hue2.getNthBin(1);

  // determine which one is closer to blue/orange
  int blue = closestToRange(c1, c2, _targetBlue, _targetBlue);
  int orange = (blue == c1) ? c2 : c1;

  float score = 0;

  // make sure sat and value are highish
  Histogram1D sat = getSatHist(i, 20);
  Histogram1D val = getGrayscaleHist(i, 20);

  // calculate objective scores
  int blueDiff = abs(blue - _targetBlue);   // max should be 180
  double blueScore = (180 - blueDiff) / 180.0;   // max should now be 1
  blueScore *= blueScore;

  int orangeDiff = abs(orange - _targetOrange);
  double orangeScore = (180 - orangeDiff) / 180.0;
  orangeScore *= orangeScore;

  // color contrast score
  float cdiff = abs(blue - orange) / 180.0;

  // color score, modulated by sat
  double colorScore = ((blueScore + orangeScore) * sat.avg()) / 2.0;

  // final score is weighted combo of how close colors are to orange/blue and
  // color difference between them, then modulated by overall brightness.
  return ((colorScore * 0.75 + cdiff * 0.25) * val.avg()) * 100;
}

unsigned int OrangeBlueAttribute::closestToRange(int x, int y, int min, int max)
{
  int x0 = x;
  int y0 = y;

  // check ranges
  if (min < 0) {
    if (x > (360 + min))
      x -= 360;

    if (y > (360 + min))
      y -= 360;
  }

  bool xinrange = (min <= x) && (x <= max);
  bool yinrange = (min <= y) && (y <= max);

  if (xinrange && yinrange) {
    int center = (max - min) / 2;
    
    return abs(x - center) < (abs(y - center)) ? x0 : y0;
  }

  if (xinrange)
    return x0;
  
  if (yinrange)
    return y0;

  int xminDist = abs(x - min) < (x - max) ? abs(x - min) : abs(x - max);
  int yminDist = abs(y - min) < (y - max) ? abs(y - min) : abs(y - max);

  return (xminDist < yminDist) ? x0 : y0;
}
