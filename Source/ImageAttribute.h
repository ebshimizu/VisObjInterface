/*
  ==============================================================================

    ImageAttribute.h
    Created: 1 Aug 2016 5:24:38pm
    Author:  eshimizu

  ==============================================================================
*/

#ifndef IMAGEATTRIBUTE_H_INCLUDED
#define IMAGEATTRIBUTE_H_INCLUDED

#include "HistogramAttribute.h"

#define SPARSE5D
//#define LABXYHIST

// literally the only point of this component is to draw an image
class ImageDrawer : public Component
{
public:
  ImageDrawer(Image i);
  ~ImageDrawer();

  void resized() override;
  void paint(Graphics& g);

private:
  Image _img;
};

class ImageAttribute : public HistogramAttribute
{
public:
  ImageAttribute(string name, string filepath, float weight = 50);
  ImageAttribute(string name, Image img, float weight = 50);
  ImageAttribute(string name, Snapshot* s, float weight = 50);
  ~ImageAttribute();

  virtual double evaluateScene(Snapshot* s, Image& img);

  virtual void preProcess();

  virtual void resized() override;
  virtual void buttonClicked(Button* b) override;

  Image getOriginalImage();

  // Returns the average Lab distance from the given confguration to the target
  // histogram image
  double avgLabDistance(Snapshot* s);
  
private:
  float _weight;

#ifdef SPARSE5D
  Sparse5DHistogram _sourceHist;
#endif
#ifdef LABXYHIST
  LabxyHistogram _sourceHist;
#endif
  Image _sourceImg;
  Image _originalImg;
  vector<vector<double> > _metric;

  TextButton _showImgButton;
};

#endif  // IMAGEATTRIBUTE_H_INCLUDED
