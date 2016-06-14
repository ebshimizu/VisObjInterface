/*
  ==============================================================================

    KMeans.h
    Created: 14 Jun 2016 11:27:12am
    Author:  eshimizu

  ==============================================================================
*/

#ifndef KMEANS_H_INCLUDED
#define KMEANS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "globals.h"
#include "AttributeSearchResult.h"

typedef function<double(AttributeSearchResult*, AttributeSearchResult*)> distFuncType;

// An implementation of K-means for clustering search results
class KMeans {
public:
  enum InitMode {
    FORGY,    // Forgy init method: randomly select k points as mean
    RND_PART  // Randomly assign each point to a center k
  };

  KMeans();
  KMeans(distFuncType distFunc);

  Array<AttributeSearchResult*> cluster(int k, Array<AttributeSearchResult*> points, InitMode init);

private:
  Array<AttributeSearchResult*> forgy(int k, Array<AttributeSearchResult*>& points);
  Array<AttributeSearchResult*> rndpart(int k, Array<AttributeSearchResult*>& points);

  // Returns the closest center to the specified point
  int closestCenter(AttributeSearchResult* point, Array<AttributeSearchResult*>& centers);

  distFuncType _distFunc;
};

#endif  // KMEANS_H_INCLUDED
