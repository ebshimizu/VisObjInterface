/*
  ==============================================================================

    AttributeSorting.h
    Created: 16 Feb 2016 1:41:37pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef ATTRIBUTESORTING_H_INCLUDED
#define ATTRIBUTESORTING_H_INCLUDED

#include "AttributeSearchResult.h"

class AttributeSorter
{
public:
  AttributeSorter() { }
  ~AttributeSorter() { }
   
  virtual int compareElements(AttributeSearchResult* first, AttributeSearchResult* second) = 0;
};

class DefaultSorter : public AttributeSorter
{
public:
  DefaultSorter() { }
  ~DefaultSorter() { }

  virtual int compareElements(AttributeSearchResult* first, AttributeSearchResult* second);
};

class AvgHueSorter : public AttributeSorter
{
public:
  AvgHueSorter() { }
  ~AvgHueSorter() { }

  virtual int compareElements(AttributeSearchResult* first, AttributeSearchResult* second);
};

class KeyHueSorter : public AttributeSorter
{
public:
  KeyHueSorter() { }
  ~KeyHueSorter() { }

  virtual int compareElements(AttributeSearchResult* first, AttributeSearchResult* second);
};

class AvgBrightSorter : public AttributeSorter
{
public:
  AvgBrightSorter() { }
  ~AvgBrightSorter() { }

  virtual int compareElements(AttributeSearchResult* first, AttributeSearchResult* second);
};

class KeyBrightSorter : public AttributeSorter
{
public:
  KeyBrightSorter() { }
  ~KeyBrightSorter() { }

  virtual int compareElements(AttributeSearchResult* first, AttributeSearchResult* second);
};

class KeyAzmSorter : public AttributeSorter
{
public:
  KeyAzmSorter() { }
  ~KeyAzmSorter() { }

  virtual int compareElements(AttributeSearchResult* first, AttributeSearchResult* second);
};


#endif  // ATTRIBUTESORTING_H_INCLUDED
