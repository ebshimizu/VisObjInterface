/*
  ==============================================================================

    SparseHistogram.cpp
    Created: 29 Nov 2016 8:45:57pm
    Author:  falindrith

  ==============================================================================
*/

#include "SparseHistogram.h"
#include "FastEMD-3.1/FastEMD/emd_hat_signatures_interface.hpp"
#include "FastSimplex/network_simplex_simple.h"

SparseHistogram::SparseHistogram(int dims, vector<float> bounds, float lambda) :
  _n(dims), _bounds(bounds), _lambda(lambda), _totalWeight(0) {

}

SparseHistogram::~SparseHistogram() {
}

void SparseHistogram::add(vector<float> pt)
{
  addToBin(1, pt);
}

void SparseHistogram::addToBin(float amt, vector<float> pt)
{
  _histData[closestBin(pt)] += amt;
  _totalWeight += amt;
}

void SparseHistogram::removeFromBind(float amt, vector<float> pt)
{
  _histData[closestBin(pt)] -= amt;
  _totalWeight -= amt;
}

float SparseHistogram::getBin(vector<float> pt)
{
  return _histData[closestBin(pt)];
}

float SparseHistogram::getBin(HistogramFeature pt)
{
  if (_histData.count(pt) > 0) {
    return _histData[pt];
  }
  else {
    return 0;
  }
}

HistogramFeature SparseHistogram::getLargestBin()
{
  HistogramFeature largest = _histData.begin()->first;
  for (auto d : _histData) {
    if (d.first > largest) {
      largest = d.first;
    }
  }

  return largest;
}

float SparseHistogram::EMD(SparseHistogram & other)
{
  function<float(feature_tt*, feature_tt*)> dist = [this](feature_tt* f1, feature_tt* f2) {
    float sum = 0;

    for (int i = 0; i < f1->_v.size(); i++) {
      if (_lambda < 0) {
        sum += pow(f1->_v[i] - f2->_v[i], 2);
      }
      else {
        if (i >= f1->_v.size() - 2) {
          sum += _lambda * pow(f1->_v[i] - f2->_v[i], 2);
        }
        else {
          sum += pow(f1->_v[i] - f2->_v[i], 2);
        }
      }
    }

    return sqrt(sum);
  };

  // construct stuctures for FastSimplex
  vector<feature_tt> f1, f2;
  vector<float> weights1 = normalizedWeights(f1);
  vector<float> weights2 = other.negNormalizedWeights(f2);

  unsigned int n1 = f1.size();
  unsigned int n2 = f2.size();

  Digraph di(n1, n2);
  NetworkSimplexSimple<Digraph, float, float, unsigned int> net(di, true, n1 + n2, n1 * n2, 1000);
  unsigned int arcId = 0;
  for (unsigned int i = 0; i < n1; i++) {
    for (unsigned int j = 0; j < n2; j++) {
      float d = dist(&f1[i], &f2[j]);
      net.setCost(di.arcFromId(arcId), d);
      arcId++;
    }
  }

  net.supplyMap(&weights1[0], n1, &weights2[0], n2);
  int ret = net.run();

  double resultDist = net.totalCost();
  return (float)resultDist;
}

vector<int> SparseHistogram::weights(vector<feature_tt>& out)
{
  out.clear();
  vector<int> weights;
  for (auto& f : _histData) {
    weights.push_back((int)f.second);
    out.push_back(f.first._data);
  }

  return weights;
}

vector<int> SparseHistogram::negWeights(vector<feature_tt>& out)
{
  out.clear();
  vector<int> weights;
  for (auto& f : _histData) {
    weights.push_back((int)-f.second);
    out.push_back(f.first._data);
  }

  return weights;
}

vector<float> SparseHistogram::normalizedWeights(vector<feature_tt>& out)
{
  out.clear();
  vector<float> nrmWeights;
  for (auto& f : _histData) {
    nrmWeights.push_back(f.second / _totalWeight);
    out.push_back(f.first._data);
  }

  return nrmWeights;
}

vector<float> SparseHistogram::negNormalizedWeights(vector<feature_tt>& out)
{
  out.clear();
  vector<float> nrmWeights;
  for (auto& f : _histData) {
    // add just a bit of extra negative weight to make solver happy
    nrmWeights.push_back((-f.second / _totalWeight));
    out.push_back(f.first._data);
  }

  return nrmWeights;
}

void SparseHistogram::setLambda(double lambda)
{
  _lambda = lambda;
}

int SparseHistogram::getDims()
{
  return _n;
}

map<double, double> SparseHistogram::getDimension(int dim)
{
  map<double, double> data;
  for (auto b : _histData) {
    data[b.first._data._v[dim]] += b.second;
  }

  return data;
}

float SparseHistogram::getTotalWeight()
{
  return _totalWeight;
}

HistogramFeature SparseHistogram::closestBin(vector<float> pt)
{
  vector<float> bin;
  bin.resize(pt.size());

  for (int i = 0; i < pt.size(); i++) {
    bin[i] = closest(pt[i], i * 2);
  }

  return HistogramFeature(bin);
}

float SparseHistogram::closest(float val, int boundsIndex)
{
  float bin = (int)((abs(val) - _bounds[boundsIndex]) / _bounds[boundsIndex + 1]) *
    _bounds[boundsIndex + 1] + _bounds[boundsIndex] + (_bounds[boundsIndex + 1] / 2);

  if (bin == 0)
    return 0;
  else
    return (val < 0) ? -bin : bin;
}
