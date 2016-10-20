/*
  ==============================================================================

    AttributeStyles.cpp
    Created: 19 Oct 2016 3:16:29pm
    Author:  falindrith

  ==============================================================================
*/

#include "AttributeStyles.h"

double getStyleTerm(Style st, Snapshot* s, Image& img)
{
  switch (st)
  {
  case NO_STYLE:
    return 0;
  case SIDE_LIGHT:
    return sideLightStyle(s, img);
  case DIRECTIONAL:
    return directionalLightStyle(s, img);
  case FLAT:
    return flatLightStyle(s, img);
  default:
    return 0;
  }
}

double sideLightStyle(Snapshot * s, Image & img)
{
  double sideIntens = 0;
  double otherIntens = 0;

  for (auto d : s->getDevices()) {
    string sys = d->getMetadata("system");
    if (sys == "side left" || sys == "side right" || sys == "side") {
      sideIntens += d->getIntensity()->asPercent();
    }
    else {
      otherIntens += d->getIntensity()->asPercent();
    }
  }

  double r = sideIntens / (sideIntens + otherIntens);

  // basically the larger the diff between side and other the better
  return r * 30;
}

double directionalLightStyle(Snapshot * s, Image & img)
{
  map<string, double> sysIntens;

  for (auto d : s->getDevices()) {
    string sys = d->getMetadata("system");
    sysIntens[sys] += d->getIntensity()->asPercent();
  }

  double maxIntens = 0;
  double totalIntens = 0;

  for (auto i : sysIntens) {
    if (i.second > maxIntens)
      maxIntens = i.second;

    totalIntens += i.second;
  }

  if (totalIntens == 0)
    return 0;

  return (maxIntens / totalIntens) * 30;
}

double flatLightStyle(Snapshot* s, Image& img) {
  double frontIntens = 0;
  double backIntens = 0;

  for (auto d : s->getDevices()) {
    string sys = d->getMetadata("system");
    if (sys == "front" || sys == "front left" || sys == "front right") {
      frontIntens += d->getIntensity()->asPercent();
    }
    else if (sys == "back" || sys == "back left" || sys == "back right") {
      backIntens += d->getIntensity()->asPercent();
    }
  }

  if (backIntens == 0)
    return 50.0;

  // target ratio is > 2
  double r = frontIntens / backIntens - 2;

  // add bonus up to a limit
  return min(r * 30, 50.0);
}