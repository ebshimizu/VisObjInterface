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

  shared_ptr<SearchResultContainer> operator[](int i);

  int size();

  void resized() override;
  void paint(Graphics& g) override;

  void setWidth(int width);

  void setCols(int cols);

  void sort(AttributeSorter* s);
private:
  Array<shared_ptr<SearchResultContainer> > _contents;

  int _cols;
  float _heightRatio;
};



#endif  // SEARCHRESULTLIST_H_INCLUDED
