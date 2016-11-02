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
class GibbsPallet : public Component, public SettableTooltipClient {
public:
  GibbsPallet(String name, Image& src);

  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  void setImg(Image& img);
  void computeSchedule();

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
};

// Displays a list of the contained pallets in a grid
class GibbsPalletContainer : public Component {
public: 
  GibbsPalletContainer(int c = 4);
  ~GibbsPalletContainer();
 
  virtual void paint(Graphics& g) override;
  virtual void resized() override;

  void addPallet(GibbsPallet* pallet);
  void setCols(int c);
  void clearPallets();

  Array<GibbsPallet*> getPallets();

private:
  int _cols;

  Array<GibbsPallet*> _pallets;
};

// This component is a color constraint on the gibbs sampler. 
// Takes a set of devices and applies the specified color pallet constraint to the
// devices
class GibbsColorConstraint : public Component {

};

// This component is an intensity constraint on the gibbs sampler.
// Takes a set of devices and applies the specified intensity constraint to them
// this may end up being mostly optional, as the search itself will also sometimes
// pick particular intensity styles to apply to the lights
class GibbsIntensityConstraint : public Component {

};

#endif  // GIBBSCOMPONENTS_H_INCLUDED
