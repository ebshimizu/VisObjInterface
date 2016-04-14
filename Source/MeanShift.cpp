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

MeanShift::MeanShift() {
  _kernelFunc = gaussian_kernel;
}

void MeanShift::set_kernel(function<double(double, double)> kernelFunction) {
  _kernelFunc = kernelFunction;
}

Eigen::VectorXd MeanShift::shift_point(const Eigen::VectorXd &point, const list<Eigen::VectorXd>& points, double kernelBandwidth, vector<double>& weights, double w) {
  Eigen::VectorXd shifted_point = point;
  
  for (int dim = 0; dim < shifted_point.size(); dim++) {
    shifted_point[dim] = 0;
  }

  double total_weight = 0;
  
  //for (int i = 0; i < points.size(); i++) {
  int i = 0;
  for (auto& p : points) {
    Eigen::VectorXd temp_point = p;
    //double distance = (point - temp_point).norm(); // L2 norm (distance)
    double distance = abs(weights[i] - w);
    //double attrWeight = (weights[i] < 0) ? 0 : weights[i];
    double weight = _kernelFunc(distance, kernelBandwidth);// *(attrWeight * 2);

    shifted_point += temp_point * weight;
    total_weight += weight;
    i++;
  }

  shifted_point /= total_weight;

  return shifted_point;
}

list<Eigen::VectorXd> MeanShift::cluster(list<Eigen::VectorXd> points, double kernelBandwidth, vector<double>& weights) {
  double epsilon = getGlobalSettings()->_meanShiftEps;
  
  vector<bool> stop_moving;
  stop_moving.resize(points.size());
  list<Eigen::VectorXd> shifted_points = points;
  double max_shift_distance;

  do {
    max_shift_distance = 0;
    
    int i = 0;
    //for (int i = 0; i < shifted_points.size(); i++) {
    for (auto& p : shifted_points) {
      if (!stop_moving[i]) {
        Eigen::VectorXd newPoint = shift_point(p, points, kernelBandwidth, weights, weights[i]);
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