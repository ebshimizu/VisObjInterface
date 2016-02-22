/*
  ==============================================================================

    AttributeSearchCluster.h
    Created: 16 Feb 2016 3:20:25pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef ATTRIBUTESEARCHCLUSTER_H_INCLUDED
#define ATTRIBUTESEARCHCLUSTER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeSearchResult.h"
#include "AttributeSorting.h"

//==============================================================================
class AttributeSearchCluster : public Component
{
public:
  AttributeSearchCluster(Array<AttributeSearchResult*> elems);
  ~AttributeSearchCluster();

  void paint(Graphics& g);
  void resized();

  void setWidth(int width);
  void sort();  // Uses the value in the globals to sort
  void sort(AttributeSorter* s);

private:
  Array<AttributeSearchResult*> _elems;
};

#endif  // ATTRIBUTESEARCHCLUSTER_H_INCLUDED
