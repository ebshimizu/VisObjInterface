/*
  ==============================================================================

    Histogram.cpp
    Created: 29 Nov 2016 8:18:28pm
    Author:  falindrith

  ==============================================================================
*/

#include "Histogram.h"

Histogram1D::Histogram1D(int numBins) {
  _count = 0;

  if (numBins < 1)
    _numBins = 1;
  else
    _numBins = numBins;
}

Histogram1D::Histogram1D(const Histogram1D & other) :
  _histData(other._histData),
  _count(other._count),
  _numBins(other._numBins)
{
}

Histogram1D::Histogram1D(map<unsigned int, unsigned int> data, unsigned int count, int numBins) :
  _histData(data), _count(count), _numBins(numBins)
{
}

Histogram1D::~Histogram1D()
{
}

void Histogram1D::addValToBin(double x)
{
  int bin = (int)(x * _numBins);
  _histData[bin] += 1;
  _count++;
}

void Histogram1D::addValToBin(uint8 x)
{
  addValToBin(x / 255.0);
}

unsigned int Histogram1D::getBin(unsigned int id)
{
  return _histData[id];
}

unsigned int Histogram1D::getNthBin(unsigned int n)
{
  unsigned int i = 0;
  for (const auto& d : _histData) {
    if (i == n) {
      return d.first;
    }

    i++;
  }

  // returns 0 if nothing is found
  return 0;
}

void Histogram1D::addToBin(unsigned int amt, unsigned int id)
{
  _histData[id] += amt;
  _count += amt;
}

void Histogram1D::removeFromBin(unsigned int amt, unsigned int id)
{
  // don't do a thing if the bin isn't actually here
  if (_histData.count(id) == 0 || _histData[id] == 0)
    return;

  _histData[id] -= amt;
  _count -= amt;
}

double Histogram1D::diff(Histogram1D & other)
{
  // histograms must have the same number of bins to be compared
  if (_histData.size() != other._histData.size())
    return -1;

  double sum = 0;
  for (auto& bin : _histData) {
    sum += sqrt(pow((double)bin.second - (double)(other._histData[bin.first]), 2));
  }

  return sum;
}

Histogram1D Histogram1D::consolidate(int targetNumBins)
{
  // distance threshold
  int maxDist = 1;


  map<unsigned int, unsigned int> newData = _histData;

  while (newData.size() > targetNumBins) {
    for (auto& b : newData) {
      // if the bin is below the threshold
      //if (b.second / (float)_count < t) {
      // find closest larger bin and merge
      int start = b.first;

      int max = -1;
      for (int i = 1; i <= maxDist; i++) {
        // do a search starting at the position of the current bin
        if (newData.count(start - i) > 0) {
          // check if bin is larger than current bin
          if (newData[start - i] >= newData[start]) {
            if (max == -1 || newData[max] < newData[start - i]) {
              max = start - i;
            }
          }
        }

        // same thing for forward
        if (newData.count(start + i) > 0) {
          if (newData[start + i] >= newData[start]) {
            if (max == -1 || newData[max] < newData[start + i]) {
              max = start + i;
            }
          }
        }

        if (max != -1) {
          newData[max] += newData[start];
          newData[start] = 0;
          break;
        }
      }
      //}
    }

    // delete bins that are 0
    for (int i = 0; i <= _numBins; i++) {
      if (newData[i] == 0)
        newData.erase(i);
    }

    // increment threshold
    //t += 0.01f;
    maxDist += 1;
  }

  return Histogram1D(newData, _count, _numBins);
}

double Histogram1D::avg()
{
  double interval = 1.0 / _numBins;
  double sum = 0;

  for (const auto& vals : _histData) {
    double binVal = interval * vals.first;
    sum += binVal * vals.second;
  }

  return sum / (double)_count;
}

double Histogram1D::variance()
{
  double a = avg();

  double sum = 0;
  double interval = 1.0 / _numBins;

  for (const auto& vals : _histData) {
    // bin deviation
    double dev = (vals.first * interval) - a;
    dev *= dev;

    // count the number of elements in the bin, add to running total
    sum += dev * vals.second;
  }

  // return mean of sum
  return sum / _count;
}

double Histogram1D::stdev()
{
  return sqrt(variance());
}

double Histogram1D::percentile(float pct)
{
  float target = pct / 100.0f;

  // defined as the first value such that n% of all values are <= than that value
  unsigned int total = 0;
  double interval = 1.0 / _numBins;
  unsigned int highest = 0;

  for (const auto& vals : _histData) {
    total += vals.second;
    if ((float)total / _count <= target)
      highest = vals.first;
    else
      break;
  }

  return highest * interval;
}

double Histogram1D::percentGreaterThan(float pct)
{
  pct /= 100.0;
  unsigned int firstBin = (unsigned int)(pct * _numBins);
  unsigned int total = 0;

  for (int i = firstBin; i < _numBins; i++) {
    total += _histData[i];
  }

  return total / (double)_count;
}

double Histogram1D::percentLessThan(float pct)
{
  return 1 - percentGreaterThan(pct);
}

// ============================================================================

Histogram3D::Histogram3D(int n) :
  _x(n), _y(n), _z(n), _xmin(0), _xmax(1), _ymin(0), _ymax(1), _zmin(0), _zmax(0)
{
  _count = 0;
  _histData.resize(_x * _y * _z);
}

Histogram3D::Histogram3D(int x, int y, int z) :
  _x(x), _y(y), _z(z), _xmin(0), _xmax(1), _ymin(0), _ymax(1), _zmin(0), _zmax(0)
{
  _count = 0;
  _histData.resize(_x * _y * _z);
}

Histogram3D::Histogram3D(int x, int y, int z, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax) :
  _x(x), _y(y), _z(z), _xmin(xmin), _xmax(xmax), _ymin(ymin), _ymax(ymax), _zmin(zmin), _zmax(zmax)
{
  _count = 0;
  _histData.resize(_x * _y * _z);
}

Histogram3D::Histogram3D(const Histogram3D & other) :
  _x(other._x), _y(other._y), _z(other._z), _xmin(other._xmin), _xmax(other._xmax),
  _ymin(other._ymin), _ymax(other._ymax), _zmin(other._zmin), _zmax(other._zmax)
{
  _count = other._count;
  _histData.resize(_x * _y * _z);

  memcpy(&_histData.front(), &other._histData.front(), sizeof(unsigned int) * _x * _y * _z);
}

Histogram3D & Histogram3D::operator=(const Histogram3D & other)
{
  _x = other._x;
  _y = other._y;
  _z = other._z;
  _xmin = other._xmin;
  _xmax = other._xmax;
  _ymin = other._ymin;
  _ymax = other._ymax;
  _zmin = other._zmin;
  _zmax = other._zmax;
  _count = other._count;

  _histData.resize(_x * _y * _z);

  memcpy(&_histData.front(), &other._histData.front(), sizeof(unsigned int) * _x * _y * _z);
  return *this;
}

Histogram3D::~Histogram3D()
{
}

void Histogram3D::addValToBin(double x, double y, double z)
{
  // scale x, y, z to proper values and find what bin they're in
  int xbin = (int)Lumiverse::clamp((float)((x - _xmin) / (_xmax - _xmin)) * _x, 0.0f, _x - 1.0f);
  int ybin = (int)Lumiverse::clamp((float)((y - _ymin) / (_ymax - _ymin)) * _y, 0.0f, _y - 1.0f);
  int zbin = (int)Lumiverse::clamp((float)((z - _zmin) / (_zmax - _zmin)) * _z, 0.0f, _z - 1.0f);

  addToBin(1, xbin, ybin, zbin);
}

void Histogram3D::addToBin(int amt, int x, int y, int z)
{
  _histData[getIndex(x, y, z)] += amt;
  _count += amt;
}

void Histogram3D::removeFromBin(int amt, int x, int y, int z)
{
  _histData[getIndex(x, y, z)] -= amt;
  _count -= amt;
}

unsigned int Histogram3D::getBin(int x, int y, int z)
{
  return _histData[getIndex(x, y, z)];
}

string Histogram3D::toString()
{
  stringstream ss;

  for (int z = 0; z < _z; z++) {
    ss << "z = " << z << "\n";
    for (int y = 0; y < _y; y++) {
      bool first = true;
      for (int x = 0; x < _x; x++) {
        if (!first) {
          ss << "\t" << _histData[getIndex(x, y, z)];
        }
        else {
          ss << _histData[getIndex(x, y, z)];
          first = false;
        }
      }
      ss << "\n";
    }
    ss << "\n";
  }

  return ss.str();
}

double Histogram3D::L2dist(Histogram3D & other)
{
  if (other._x != _x || other._y != _y || other._z != _z) {
    // invalid dimensions
    Lumiverse::Logger::log(Lumiverse::LOG_LEVEL::ERR, "Invalid dimensions provided for histogram difference.");
    return -1;
  }

  double err = 0;
  for (int i = 0; i < _x * _y * _z; i++) {
    err += pow((double)_histData[i] - (double)other._histData[i], 2.0);
  }

  return sqrt(err);
}

double Histogram3D::emd(Histogram3D & other, vector<vector<double>>& gd)
{
  return _emd(normalized(), other.normalized(), gd);
}

vector<double> Histogram3D::normalized()
{
  vector<double> norm;
  norm.resize(_x * _y * _z);

  for (int i = 0; i < _x * _y * _z; i++) {
    if (_count != 0) {
      norm[i] = _histData[i] / (double)_count;
    }
  }

  return norm;
}

vector<vector<double>> Histogram3D::getGroundDistances()
{
  // L1 distance (manhattan distance) between each bin since
  // this is a fixed size histogram for now
  vector<vector<double>> gd;
  gd.resize(_x * _y * _z);

  // fun nested loop time 
  for (int i = 0; i < _z; i++) {
    for (int j = 0; j < _y; j++) {
      for (int k = 0; k < _x; k++) {
        vector<double> distances;
        distances.resize(_x * _y * _z);
        Eigen::Vector3d binVal = getBinVal(k, j, i);

        // ok now go through it again
        for (int i2 = 0; i2 < _z; i2++) {
          for (int j2 = 0; j2 < _y; j2++) {
            for (int k2 = 0; k2 < _x; k2++) {
              // calculate l2 dist in Lab space
              distances[getIndex(k2, j2, i2)] = (getBinVal(k2, j2, i2) - binVal).norm();
            }
          }
        }

        gd[getIndex(k, j, i)] = distances;
      }
    }
  }

  return gd;
}

int Histogram3D::getIndex(int x, int y, int z)
{
  return x + y * _x + z * _x * _y;
}

Eigen::Vector3d Histogram3D::getBinVal(int x, int y, int z)
{
  // we'll return the midpoint of the bin 
  Eigen::Vector3d binVal;
  binVal[0] = (x + 0.5) * ((_xmax - _xmin) / _x) + _xmin;
  binVal[1] = (y + 0.5) * ((_ymax - _ymin) / _y) + _ymin;
  binVal[2] = (z + 0.5) * ((_zmax - _zmin) / _z) + _zmin;

  return binVal;
}

// ============================================================================

LabxyHistogram::LabxyHistogram(int n, double lambda) : _bins({ n, n, n, n, n }), _lambda(lambda)
{
  _count = 0;
  _bounds = vector<double>({ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 });
  _histData.resize(numTotalBins());
}

LabxyHistogram::LabxyHistogram(int l, int a, int b, int x, int y, double lambda) :
  _bins({ l, a, b, x, y }), _lambda(lambda)
{
  _count = 0;
  _bounds = vector<double>({ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 });
  _histData.resize(numTotalBins());
}

LabxyHistogram::LabxyHistogram(int l, int a, int b, int x, int y, vector<double> bounds, double lambda) :
  _bins({ l, a, b, x, y }), _lambda(lambda)
{
  _count = 0;
  _bounds = bounds;
  _histData.resize(numTotalBins());
}

LabxyHistogram::LabxyHistogram(const LabxyHistogram & other) :
  _bins(other._bins), _count(other._count), _lambda(other._lambda)
{
  _count = other._count;
  _bounds = other._bounds;
  _histData = other._histData;
}

LabxyHistogram & LabxyHistogram::operator=(const LabxyHistogram & other)
{
  _bins = other._bins;
  _bounds = other._bounds;
  _count = other._count;
  _lambda = other._lambda;

  _histData.clear();
  _histData = other._histData;

  return *this;
}

LabxyHistogram::~LabxyHistogram()
{
}

void LabxyHistogram::add(double l, double a, double b, float x, float y)
{
  // scale x, y, z to proper values and find what bin they're in


  // TODO: add to bins proportionally
  addToBin(1, nearest(l, 0), nearest(a, 1), nearest(b, 2), nearest(x, 3), nearest(y, 4));
  //addProportional(1, { l, a, b, x, y }, {});
}

void LabxyHistogram::addToBin(double amt, int l, int a, int b, int x, int y)
{
  _histData[getIndex(l, a, b, x, y)] += amt;
  _count += amt;
}

void LabxyHistogram::removeFromBin(double amt, int l, int a, int b, int x, int y)
{
  _histData[getIndex(l, a, b, x, y)] -= amt;
  _count -= amt;
}

double LabxyHistogram::getBin(int l, int a, int b, int x, int y)
{
  return _histData[getIndex(l, a, b, x, y)];
}

double LabxyHistogram::EMD(LabxyHistogram & other, const vector<vector<double>>& gd)
{
  return _emd(normalized(), other.normalized(), gd);
}

vector<double> LabxyHistogram::normalized()
{
  vector<double> nrm;
  nrm.resize(numTotalBins());

  for (int i = 0; i < nrm.size(); i++) {
    nrm[i] = _histData[i] / _count;
  }

  return nrm;
}

vector<vector<double>> LabxyHistogram::getGroundDistances()
{
  vector<vector<double>> gd;
  gd.resize(numTotalBins());

  // fun nested loop time 
  for (int y = 0; y < _bins[4]; y++) {
    for (int x = 0; x < _bins[3]; x++) {
      for (int b = 0; b < _bins[2]; b++) {
        for (int a = 0; a < _bins[1]; a++) {
          for (int l = 0; l < _bins[0]; l++) {
            vector<double> distances;
            distances.resize(numTotalBins());
            Eigen::VectorXd binVal = getBinVal(l, a, b, x, y);

            // ok now go through it again
            for (int y2 = 0; y2 < _bins[4]; y2++) {
              for (int x2 = 0; x2 < _bins[3]; x2++) {
                for (int b2 = 0; b2 < _bins[2]; b2++) {
                  for (int a2 = 0; a2 < _bins[1]; a2++) {
                    for (int l2 = 0; l2 < _bins[0]; l2++) {
                      Eigen::VectorXd val = getBinVal(l2, a2, b2, x2, y2);
                      Eigen::VectorXd diff = binVal - val;
                      Eigen::VectorXd delta2 = (diff).cwiseProduct(diff);
                      distances[getIndex(l2, a2, b2, x2, y2)] = sqrt(delta2[0] + delta2[1] + delta2[2] + _lambda * (delta2[3] + delta2[4]));
                    }
                  }
                }
              }
            }

            gd[getIndex(l, a, b, x, y)] = distances;
          }
        }
      }
    }
  }

  return gd;
}

void LabxyHistogram::setLambda(double lambda)
{
  _lambda = lambda;
}

inline int LabxyHistogram::getIndex(int l, int a, int b, int x, int y)
{
  return l + a * _bins[0] + b * _bins[0] * _bins[1] + x * _bins[0] * _bins[1] * _bins[2] + y * _bins[0] * _bins[1] * _bins[2] * _bins[3];
}

inline int LabxyHistogram::numTotalBins()
{
  return _bins[0] * _bins[1] * _bins[2] * _bins[3] * _bins[4];
}

void LabxyHistogram::addProportional(float weight, vector<double> coord, vector<int> bin)
{
  // also a base case if there's nothing to allocate
  if (weight == 0)
    return;

  int currentDepth = (int)bin.size();
  int idx = currentDepth * 2;
  double binVal = Lumiverse::clamp((float)((coord[currentDepth] - _bounds[idx]) / ((_bounds[idx + 1] - _bounds[idx]) / (_bins[currentDepth] - 1))), 0, _bins[currentDepth] - 1.0f);
  int high = (int)ceil(binVal);
  int low = (int)floor(binVal);

  // % of weight in high bin.
  // this is not necessarily the closest bin.
  float a;

  if (abs(binVal - high) < abs(binVal - low)) {
    a = 1 - (float)abs(binVal - high);
  }
  else {
    a = (float)abs(binVal - low);
  }

  // 4 is max depth here, the y bin axis, base case
  if (currentDepth == 4) {
    addToBin(weight * a, bin[0], bin[1], bin[2], bin[3], high);
    addToBin(weight * (1 - a), bin[0], bin[1], bin[2], bin[3], low);
  }
  else {
    vector<int> hbin(bin);
    vector<int> lbin(bin);

    hbin.push_back(high);
    lbin.push_back(low);

    addProportional(weight * a, coord, hbin);
    addProportional(weight * (1 - a), coord, lbin);
  }
}

int LabxyHistogram::nearest(double val, int axis)
{
  // bins here are specified as evenly distributed points within the max-min range specified in bounds.
  int idx = axis * 2;

  double binVal = (val - _bounds[idx]) / ((_bounds[idx + 1] - _bounds[idx]) / (_bins[axis] - 1));
  binVal = Lumiverse::clamp((float)binVal, 0, _bins[axis] - 1.0f);

  return (int)round(binVal);
}

Eigen::VectorXd LabxyHistogram::getBinVal(int l, int a, int b, int x, int y)
{
  Eigen::VectorXd val;
  val.resize(5);

  val[0] = l * ((_bounds[1] - _bounds[0]) / (_bins[0] - 1)) + _bounds[0];
  val[1] = a * ((_bounds[3] - _bounds[2]) / (_bins[1] - 1)) + _bounds[2];
  val[2] = b * ((_bounds[5] - _bounds[4]) / (_bins[2] - 1)) + _bounds[4];
  val[3] = x * ((_bounds[7] - _bounds[6]) / (_bins[3] - 1)) + _bounds[6];
  val[4] = y * ((_bounds[9] - _bounds[8]) / (_bins[4] - 1)) + _bounds[8];

  return val;
}

// ============================================================================

HistogramFeature::HistogramFeature()
{
}

HistogramFeature::HistogramFeature(vector<float> pt)
{
  _data._v = pt;
}

bool HistogramFeature::operator==(const HistogramFeature & other) const
{
  if (other._data._v.size() != _data._v.size())
    return false;

  for (int i = 0; i < _data._v.size(); i++) {
    if (other._data._v[i] != _data._v[i])
      return false;
  }

  return true;
}

bool HistogramFeature::operator>(const HistogramFeature & other) const
{
  bool isThisLarger = true;
  for (int i = 0; i < _data._v.size(); i++) {
    isThisLarger &= _data._v[i] > other._data._v[i];
  }
  return isThisLarger;
}



SparseHistogram::SparseHistogram(int dims, vector<float> bounds, float lambda) :
  _n(dims), _bounds(bounds), _lambda(lambda), _totalWeight(0) {

}

SparseHistogram::SparseHistogram(const SparseHistogram & other)
{
  _histData = other._histData;
  _totalWeight = other._totalWeight;
  _lambda = other._lambda;
  _bounds = other._bounds;
  _n = other._n;
}

SparseHistogram & SparseHistogram::operator=(const SparseHistogram & other)
{
  _histData = other._histData;
  _totalWeight = other._totalWeight;
  _lambda = other._lambda;
  _bounds = other._bounds;
  _n = other._n;
  return *this;
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
  if (_histData.size() == 0)
    return HistogramFeature();

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

  unsigned int n1 = (unsigned int)f1.size();
  unsigned int n2 = (unsigned int)f2.size();

  Digraph di(n1, n2);
  lemon::NetworkSimplexSimple<Digraph, float, float, unsigned int> net(di, true, n1 + n2, n1 * n2, 1000);
  unsigned int arcId = 0;
  for (unsigned int i = 0; i < n1; i++) {
    for (unsigned int j = 0; j < n2; j++) {
      float d = dist(&f1[i], &f2[j]);
      net.setCost(di.arcFromId(arcId), d);
      arcId++;
    }
  }

  net.supplyMap(&weights1[0], n1, &weights2[0], n2);
  /* int ret = */ net.run();

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
  _lambda = (float)lambda;
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

vector<float> SparseHistogram::getBounds()
{
  return _bounds;
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
