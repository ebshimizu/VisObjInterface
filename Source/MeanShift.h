/*
  ==============================================================================

    MeanShift.h
    Created: 13 Apr 2016 3:07:58pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef MEANSHIFT_H_INCLUDED
#define MEANSHIFT_H_INCLUDED

#include <vector>
#include "globals.h"

// Modified version of https://github.com/mattnedrich/MeanShift_cpp to use Eigen::VectorXd
// instead of vector<double>
class MeanShift {
public:
  MeanShift() { set_kernel(NULL); }
  MeanShift(function<double(double, double)> kernelFunction) { set_kernel(kernelFunction); }
  list<Eigen::VectorXd> cluster(list<Eigen::VectorXd> points, double kernelBandwidth);

private:
  function<double(double, double)> _kernelFunc;

  void set_kernel(function<double(double, double)>);
  Eigen::VectorXd shift_point(const Eigen::VectorXd& point, const list<Eigen::VectorXd>& points, double kernelBandwidth);
};



#endif  // MEANSHIFT_H_INCLUDED
