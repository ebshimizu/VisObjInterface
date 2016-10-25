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
#include "AttributeStyles.h"

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

class ImageAttribute : public HistogramAttribute, public ComboBoxListener
{
public:
  enum Mode {
    FULL,
    L,
    AB,
    LAB
  };

  ImageAttribute(string name, string filepath, float weight = 50);
  ImageAttribute(string name, Image img, float weight = 50);
  ImageAttribute(string name, Snapshot* s, float weight = 50);
  ImageAttribute(ImageAttribute& other);
  ~ImageAttribute();

  // Sets style to specified.
  void setStyle(Style style);

  // intializes UI components and common initialization
  void initUI();

  virtual double evaluateScene(Snapshot* s, Image& img);

  virtual void preProcess();

  virtual void resized() override;
  virtual void buttonClicked(Button* b) override;

  Image getOriginalImage();

  // Returns the average Lab distance from the given confguration to the target
  // histogram image
  double avgLabDistance(Snapshot* s);
  
  virtual void comboBoxChanged(ComboBox* box) override;

  void lockMode();
  void unlockMode();

  void setName(string name);

private:
  float _weight;

#ifdef SPARSE5D
  SparseHistogram _fullSource;  // 5D
#endif
#ifdef LABXYHIST
  LabxyHistogram _fullSource;
#endif

  SparseHistogram _LSource;     // 1D
  SparseHistogram _abSource;    // 2D
  SparseHistogram _LabSource;   // 3D

  Image _sourceImg;
  Image _originalImg;
  vector<vector<double> > _metric;

  TextButton _showImgButton;
  ComboBox _modeSelect;
  Mode _mode;

  // additional style function 
  function<double(Snapshot*, Image&)> _styleFunction;
};

class DirectionalTestAttribute : public ImageAttribute {
public:
  DirectionalTestAttribute(string name, Image img, float weight = 50);
  ~DirectionalTestAttribute();

  virtual double evaluateScene(Snapshot* s, Image& img) override;
};

#endif  // IMAGEATTRIBUTE_H_INCLUDED
