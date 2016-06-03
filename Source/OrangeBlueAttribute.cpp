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

  // accuracy
  int blueacc = 0;
  int orangeacc = 0;

  // score
  double blueScore = 0;
  double orangeScore = 0;

  // Target hue values
  // blue: 177
  // orange: 32

  // target colors
  Eigen::Vector3d targetBlue(131 / 255.0, 181 / 255.0, 225 / 255.0);
  Eigen::Vector3d targetOrange(233 / 255.0, 143 / 255.0, 38 / 255.0);

  for (int y = 0; y < i.getHeight(); y++) {
    for (int x = 0; x < i.getWidth(); x++) {
      // get pixel color and see if its closer to blue or orange
      Colour c = i.getPixelAt(x, y);
      float hue = c.getHue();
      Eigen::Vector3d rgbpx(c.getRed() / 255.0, c.getGreen() / 255.0, c.getBlue() / 255.0);

      float blueDist = (targetBlue - rgbpx).norm();
      float orangeDist = (targetOrange - rgbpx).norm();

      float hueDist = 0;
      if (blueDist < orangeDist) {
        blue++;
        double s = (sqrt(3) - blueDist) / sqrt(3);
        s *= s;
        if (s > 0.65)
          blueacc++;

        blueScore += s;
      }
      else {
        orange++;
        double s = (sqrt(3) - orangeDist) / sqrt(3);
        s *= s;
        if (s > 0.65)
          orangeacc++;

        orangeScore += s;
      }
    }
  }

  //blueScore = (blueScore / blue) * ((double)blueacc / blue);
  //orangeScore = (orangeScore / orange) * ((double)orangeacc / orange);
  double score = ((blueScore + orangeScore) / (blue + orange)) * (((double)blueacc + orangeacc) / (blue + orange));

  // combine scores, hope for a good balance
  double balance = (blue > orange) ? (double)orange / blue : (double)blue / orange;
  return score * balance * 1000;
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
