/*
  ==============================================================================

    ClusterBuster.h
    Created: 5 Feb 2016 1:48:11pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef CLUSTERBUSTER_H_INCLUDED
#define CLUSTERBUSTER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AttributeSearchResult.h"

//==============================================================================
/*
  Intended to allow visualization of all clusters at once. displays clusters one
  per row, with the center cluster as first element
*/
class ClusterBuster : public Component
{
public:
  ClusterBuster(Array<AttributeSearchResult*> results);
  ~ClusterBuster();

  void paint (Graphics&);
  void resized();

private:
  Array<AttributeSearchResult*> _results;
  int _width;
  int _height;
  int _elemWidth;
  int _elemHeight;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClusterBuster)
};

class ClusterBusterWindow : public DocumentWindow
{
public:
  ClusterBusterWindow(Array<AttributeSearchResult*> results);
  ~ClusterBusterWindow();

  void resized() override;
  void closeButtonPressed() override;

private:
  Viewport* _viewer;
  ClusterBuster* _cluster;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClusterBusterWindow);
};

#endif  // CLUSTERBUSTER_H_INCLUDED
