/*
  ==============================================================================

    NoireAttribute.h
    Created: 6 Apr 2016 2:50:15pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef NOIREATTRIBUTE_H_INCLUDED
#define NOIREATTRIBUTE_H_INCLUDED

#include "HistogramAttribute.h"

class NoireAttribute : public HistogramAttribute
{
public:
  NoireAttribute();
  ~NoireAttribute();

  virtual double evaluateScene(Snapshot* s, Image& img) override;

private:
};

#endif  // NOIREATTRIBUTE_H_INCLUDED
