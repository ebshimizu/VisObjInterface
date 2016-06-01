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
  _targetBlue = 177 / 360.0;
  _targetOrange = 32 / 360.0;
}

OrangeBlueAttribute::~OrangeBlueAttribute()
{
}

double OrangeBlueAttribute::evaluateScene(Snapshot * s)
{
  Image i = generateImage(s);

  // Track number of blue/orange pixels
  int blue = 0;
  int orange = 0;

  // score
  double score = 0;

  // Target hue values
  // blue: 177
  // orange: 32
  for (int y = 0; y < i.getHeight(); y++) {
    for (int x = 0; x < i.getWidth(); x++) {
      // get pixel color and see if its closer to blue or orange
      Colour c = i.getPixelAt(x, y);
      float hue = c.getHue();

      float blueDist = min(abs(hue - _targetBlue), abs((hue - 1) - _targetBlue));
      float orangeDist = min(abs(hue - _targetOrange), abs((hue - 1) - _targetOrange));

      float hueDist = 0;
      if (blueDist < orangeDist) {
        blue++;
        hueDist = blueDist;
      }
      else {
        orange++;
        hueDist = orangeDist;
      }

      // max hueDist val should be 1 (max diff is 0.5)
      score += pow((1 - hueDist * 2), 2) * c.getSaturation();
    }
  }

  return (score / (i.getWidth() * i.getHeight())) * 100;
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
