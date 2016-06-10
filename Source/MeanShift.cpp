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

Eigen::VectorXd MeanShift::shift_point(const Eigen::VectorXd &point, const vector<Eigen::VectorXd>& points, double kernelBandwidth,
  function<double(Eigen::VectorXd, Eigen::VectorXd)> distFunc) {
  Eigen::VectorXd shifted_point = point;
  
  for (int dim = 0; dim < shifted_point.size(); dim++) {
    shifted_point[dim] = 0;
  }

  double total_weight = 0;
  
  //for (int i = 0; i < points.size(); i++) {
  int i = 0;
  for (auto& p : points) {
    Eigen::VectorXd temp_point = p;
    double distance = distFunc(point, temp_point);
    double weight = _kernelFunc(distance, kernelBandwidth);

    shifted_point += temp_point * weight;
    total_weight += weight;
    i++;
  }
  
  shifted_point /= total_weight;

  return shifted_point;
}

vector<Eigen::VectorXd> MeanShift::cluster(vector<Eigen::VectorXd> points, double kernelBandwidth,
  function<double(Eigen::VectorXd, Eigen::VectorXd)> distFunc)
{
  double epsilon = getGlobalSettings()->_meanShiftEps;
  
  vector<bool> stop_moving;
  stop_moving.resize(points.size());
  vector<Eigen::VectorXd> shifted_points = points;
  double max_shift_distance;

  do {
    max_shift_distance = 0;
    
    for (int i = 0; i < shifted_points.size(); i++) {
      if (!stop_moving[i]) {
        Eigen::VectorXd newPoint = shift_point(shifted_points[i], points, kernelBandwidth, distFunc);
        double shift_distance = distFunc(newPoint, shifted_points[i]);

        if (shift_distance > max_shift_distance) {
          max_shift_distance = shift_distance;
        }
        if (shift_distance <= epsilon) {
          stop_moving[i] = true;
        }
        
        shifted_points[i] = newPoint;
      }
    }

    printf("max_shift_distance: %f\n", max_shift_distance);
  } while (max_shift_distance > epsilon);

  // return the cluster centers, basically remove elements that are nearly identical to another thing
  // in the list
  for (auto it = shifted_points.begin(); it != shifted_points.end(); ) {
    bool del = false;
    
    for (auto it2 = shifted_points.begin(); it2 != shifted_points.end(); it2++) {
      if (it == it2)
        continue;

      if (distFunc(*it, *it2) < epsilon) {
        // delete it and break
        it = shifted_points.erase(it);
        del = true;
        break;
      }
    }

    if (!del)
      it++;
  }


  return shifted_points;
}