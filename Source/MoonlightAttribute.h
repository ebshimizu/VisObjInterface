/*
  ==============================================================================

    MoonlightAttribute.h
    Created: 26 Apr 2016 4:55:46pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef MOONLIGHTATTRIBUTE_H_INCLUDED
#define MOONLIGHTATTRIBUTE_H_INCLUDED

#include "HistogramAttribute.h"

class MoonlightAttribute : public HistogramAttribute, public Button::Listener
{
public:
  MoonlightAttribute(int w, int h);
  ~MoonlightAttribute();

  double evaluateScene(Snapshot* s) override;

  void preProcess() override;

  virtual void buttonClicked(Button* b) override;
  virtual void resized() override;

  // Sets the foreground area to the currently selected region of the image.
  // This attribute will not work until this area is set.
  void setForegroundArea();

private:
  float getMaxSystemBrightness(string sys, Snapshot* s);
  float getAvgSystemBrightness(string sys, Snapshot* s);
  Eigen::Vector3d getAvgColor(string sys, Snapshot* s);

  map<string, DeviceSet> _systemCache;

  // Bounding box for the foreground region of the image.
  Rectangle<float> _foreground;

  // Button for setting the foreground area
  Button* _setForegroundButton;

  // image for color normalization
  Image _fullWhite;
};



#endif  // MOONLIGHTATTRIBUTE_H_INCLUDED
