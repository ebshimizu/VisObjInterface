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
  ImageAttribute(string name, string filepath, int n = 10);
  ImageAttribute(string name, Image img, int n = 10);
  ~ImageAttribute();

  virtual double evaluateScene(Snapshot* s, Image& img);
  virtual void preProcess();

  virtual void resized() override;
  virtual void buttonClicked(Button* b) override;
  
private:
  int _n;
  //Sparse5DHistogram _sourceHist;
  LabxyHistogram _sourceHist;
  Image _sourceImg;
  Image _originalImg;
  vector<vector<double> > _metric;

  TextButton _showImgButton;
};

#endif  // IMAGEATTRIBUTE_H_INCLUDED
