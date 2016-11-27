/*
  ==============================================================================

    GibbsComponents.h
    Created: 2 Nov 2016 4:25:39pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef GIBBSCOMPONENTS_H_INCLUDED
#define GIBBSCOMPONENTS_H_INCLUDED

#include "globals.h"
#include "GibbsSchedule.h"

// This component displays and image and contains a gibbs schedule corresponding
// to that image
class GibbsPalette : public Component, public SettableTooltipClient {
public:
  GibbsPalette(String name, Image& src);

  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  void setImg(Image& img);

  // If you want to manually define a schedule somehow, use this function to set it
  // right now this is mostly for debug purposes, though it seems possible to
  // show users what the actual pallet is and let them modify it.
  void setCustomSchedule(shared_ptr<GibbsSchedule> sched);

  virtual void mouseDown(const MouseEvent& event) override;

  // gets a pointer to the schedule created by this pallet
  shared_ptr<GibbsSchedule> getSchedule();

private:
  shared_ptr<GibbsSchedule> _schedule;
  Image _img;
  String _name;

  // colors are stored as hsv
  vector<Eigen::VectorXd> _colors;

  // relative color weights (basically how much of the image is close to the specified color)
  vector<float> _weights;
  double _avgIntens;
};

// Displays a list of the contained pallets in a grid
class GibbsPalletContainer : public Component {
public: 
  GibbsPalletContainer(int c = 2);
  ~GibbsPalletContainer();
 
  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  void addPallet(GibbsPalette* pallet);
  void setCols(int c);
  void clearPallets();

  Array<GibbsPalette*> getPallets();

private:
  int _cols;

  Array<GibbsPalette*> _pallets;
};

// This component is a color constraint on the gibbs sampler. 
// Takes a set of devices and applies the specified color pallet constraint to the
// devices
class GibbsColorConstraint : public Component, public ButtonListener,
  public ChangeListener, public TextEditorListener
{
public:
  GibbsColorConstraint();

  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  virtual void buttonClicked(Button* b) override;

  void getDistributions(normal_distribution<float>& hue, normal_distribution<float>& sat, normal_distribution<float>& val);

  void changeListenerCallback(ChangeBroadcaster* source);

  void setSelectedColor(Colour c);

  void setColorWeight(float weight);
  void textEditorTextChanged(TextEditor& e);
  float getColorWeight();

private:
  Colour _color;
  float _sigma;
  float _weight;

  TextButton _setColor;
  TextEditor _weightInput;
};

class GibbsConstraintContainer : public Component, public SliderListener, public ButtonListener, public ComboBoxListener
{
public:
  GibbsConstraintContainer();
  ~GibbsConstraintContainer();

  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  virtual void sliderValueChanged(Slider* s) override;

  vector<vector<normal_distribution<float>>> getColorDists();
  normal_distribution<float> getIntensDist();

  // Adds a new color constraint element to the gui
  void addColorConstraint();

  // adds a set of color constraints to the gui
  void addColors(vector<Eigen::VectorXd> colors, double intens, vector<float> weights);

  virtual void buttonClicked(Button* b);

  // fills in the average, max, and number of bright lights parameters
  void getIntensDistProps(double& avg, double& max, int& k);

  // returns true if the scope is set to Systems, false if set to lights
  bool useSystems();

  // updates the bounds on the lights slider
  void updateBounds();

  // update bounds n stuff
  void comboBoxChanged(ComboBox* b);

  // returns color relative weights
  vector<float> getColorWeights();

private:
  Array<GibbsColorConstraint*> _colors;
  Array<TextButton*> _deleteButtons;

  TextButton _add;
  Slider _intens;
  Slider _numLights;
  ComboBox _scope;

  int _counter;
  int _k;
};

// This component is an intensity constraint on the gibbs sampler.
// Takes a set of devices and applies the specified intensity constraint to them
// this may end up being mostly optional, as the search itself will also sometimes
// pick particular intensity styles to apply to the lights
class GibbsIntensityConstraint : public Component {

};

#endif  // GIBBSCOMPONENTS_H_INCLUDED
