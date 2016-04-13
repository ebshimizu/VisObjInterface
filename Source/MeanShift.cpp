/*
  ==============================================================================

    MeanShift.cpp
    Created: 13 Apr 2016 3:07:58pm
    Author:  falindrith

  ==============================================================================
*/

#include "MeanShift.h"

double gaussian_kernel(double distance, double kernel_bandwidth) {
  double temp = exp(-(distance*distance) / (kernel_bandwidth));
  return temp;
}

void MeanShift::set_kernel(function<double(double, double)> kernelFunction) {
  if (!kernelFunction) {
    _kernelFunc = gaussian_kernel;
  }
  else {
    _kernelFunc = kernelFunction;
  }
}

Eigen::VectorXd MeanShift::shift_point(const Eigen::VectorXd &point, const list<Eigen::VectorXd>& points, double kernelBandwidth) {
  Eigen::VectorXd shifted_point = point;
  
  for (int dim = 0; dim < shifted_point.size(); dim++) {
    shifted_point[dim] = 0;
  }

  double total_weight = 0;
  
  //for (int i = 0; i < points.size(); i++) {
  for (auto& p : points) {
    Eigen::VectorXd temp_point = p;
    double distance = (point - temp_point).norm(); // L2 norm (distance)
    double weight = _kernelFunc(distance, kernelBandwidth);

    shifted_point += temp_point * weight;
    total_weight += weight;
  }

  shifted_point /= total_weight;

  return shifted_point;
}

list<Eigen::VectorXd> MeanShift::cluster(list<Eigen::VectorXd> points, double kernelBandwidth) {
  double epsilon = getGlobalSettings()->_meanShiftEps;
  
  vector<bool> stop_moving;
  stop_moving.reserve(points.size());
  list<Eigen::VectorXd> shifted_points = points;
  double max_shift_distance;

  do {
    max_shift_distance = 0;
    
    int i = 0;
    //for (int i = 0; i < shifted_points.size(); i++) {
    for (auto& p : shifted_points) {
      if (!stop_moving[i]) {
        Eigen::VectorXd newPoint = shift_point(p, points, kernelBandwidth);
        double shift_distance = (newPoint - p).norm();

        if (shift_distance > max_shift_distance) {
          max_shift_distance = shift_distance;
        }
        if (shift_distance <= epsilon) {
          stop_moving[i] = true;
        }
        
        p = newPoint;
      }

      i++;
    }

    printf("max_shift_distance: %f\n", max_shift_distance);
  } while (max_shift_distance > epsilon);

  return shifted_points;
}