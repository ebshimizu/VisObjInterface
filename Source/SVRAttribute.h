/*
  ==============================================================================

    SVRAttribute.h
    Created: 3 Mar 2016 4:27:53pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef SVRATTRIBUTE_H_INCLUDED
#define SVRATTRIBUTE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeControllerBase.h"
#include "svm.h"

//==============================================================================
/*
*/
class SVRAttribute : public AttributeControllerBase
{
public:
  SVRAttribute(string filename, string name);
  ~SVRAttribute();

  // Converts the devices present to a feature vector representation
  Eigen::VectorXd devicesToVector(Device* key, Device* fill, Device* rim);

protected:
  virtual double evaluateScene(Device* key, Device* fill, Device* rim) override;

private:
  string _filename;
  svm_model* _model;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SVRAttribute);
};



#endif  // SVRATTRIBUTE_H_INCLUDED
