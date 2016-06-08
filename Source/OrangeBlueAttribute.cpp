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
  int reallyBlue = 0;
  int reallyOrange = 0;

  // target colors
  Eigen::Vector3d targetBlue(40 / 255.0, 102 / 255.0, 215 / 255.0);
  Eigen::Vector3d targetOrange(227 / 255.0, 138 / 255.0, 23 / 255.0);

  for (int y = 0; y < i.getHeight(); y++) {
    for (int x = 0; x < i.getWidth(); x++) {
      // get pixel color and see if its closer to blue or orange
      Colour c = i.getPixelAt(x, y);
      Eigen::Vector3d rgbpx(c.getRed() / 255.0, c.getGreen() / 255.0, c.getBlue() / 255.0);

      float blueDist = (targetBlue - rgbpx).norm();
      float orangeDist = (targetOrange - rgbpx).norm();

      if (blueDist < orangeDist) {
        double s = (sqrt(3) - blueDist) / sqrt(3);

        if (s > 0.9)
          reallyBlue++;
        if (s > 0.61)
          blue++;
      }
      else {
        double s = (sqrt(3) - orangeDist) / sqrt(3);

        if (s > 0.9)
          reallyOrange++;
        if (s > 0.65)
          orange++;
      }
    }
  }

  double bluePct = (double)blue / (i.getWidth() * i.getHeight());
  double orangePct = (double)orange / (i.getWidth() * i.getHeight());
  double reallyBluePct = (double)reallyBlue / (i.getWidth() * i.getHeight());
  double reallyOrangePct = (double)reallyOrange / (i.getWidth() * i.getHeight());

  // Aiming for 35% orange, 35% blue, and 10% superblue/orange (which are counted in blue/orange)
  double blueScore = min(1.0, 1 - ((.3 - bluePct) / .3));
  double orangeScore = min(1.0, 1 - ((.3 - orangePct) / .3));
  double rblueScore = min(1.0, 1 - ((.1 - reallyBluePct) / .1));
  double rorangeScore = min(1.0, 1 - ((.1 - reallyOrangePct) / .1));

  return (orangeScore * 0.4 + blueScore * 0.4 + rblueScore * 0.1 + rorangeScore * 0.1) * 100;
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
