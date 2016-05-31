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

      float blueDist = min(abs(hue - 177), abs((hue - 360) - 177));
      float orangeDist = min(abs(hue - 32), abs((hue - 360) - 32));

      float hueDist = min(blueDist, orangeDist);
      score -= hueDist;
    }
  }

  return score;
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
