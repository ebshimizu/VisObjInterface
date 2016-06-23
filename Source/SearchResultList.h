/*
  ==============================================================================

    SearchResultList.h
    Created: 21 Jun 2016 4:55:23pm
    Author:  eshimizu

  ==============================================================================
*/

#ifndef SEARCHRESULTLIST_H_INCLUDED
#define SEARCHRESULTLIST_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "SearchResultContainer.h"
#include "AttributeSorting.h"

class SearchResultContainer;
class AttributeSorter;

// The search result list contains a list of search results. 
// It is also a displayable, resizable, component
class SearchResultList : public Component
{
public:
  SearchResultList();
  SearchResultList(int cols, float heightRatio);
  ~SearchResultList();

  void addResult(shared_ptr<SearchResultContainer> result);
  void removeResult(int index);

  // basically transfers owenership of the results to something else
  Array<shared_ptr<SearchResultContainer> > removeAllResults();

  // Gets a copy of the results contained in the list
  Array<shared_ptr<SearchResultContainer> > getAllResults();

  shared_ptr<SearchResultContainer> operator[](int i);

  // number of elements in the list, non-recursive
  int size();

  // number of elements in the list, recursive
  int numElements();

  void resized() override;
  void paint(Graphics& g) override;

  // Use this to trigger a resize and re-layout elements
  void setWidth(int width);

  // number of columns to display
  void setCols(int cols);

  // Sort the contained elements
  void sort(AttributeSorter* s);
private:
  Array<shared_ptr<SearchResultContainer> > _contents;

  int _cols;
  float _heightRatio;
};



#endif  // SEARCHRESULTLIST_H_INCLUDED
