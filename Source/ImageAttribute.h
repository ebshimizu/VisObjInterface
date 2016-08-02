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

class ImageAttribute : public HistogramAttribute
{
public:
  ImageAttribute(string name, string filepath, int n = 10);
  ImageAttribute(string name, Image img, int n = 10);
  ~ImageAttribute();

  virtual double evaluateScene(Snapshot* s);

  virtual void preProcess();
  
private:
  int _n;
  Histogram3D _sourceHist;
  Image _sourceImg;
};


#endif  // IMAGEATTRIBUTE_H_INCLUDED
