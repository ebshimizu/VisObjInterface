/*
  ==============================================================================

    SearchResultsViewer.h
    Created: 15 Dec 2015 5:07:58pm
    Author:  falindrith

  ==============================================================================
*/

#ifndef SEARCHRESULTSVIEWER_H_INCLUDED
#define SEARCHRESULTSVIEWER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class SearchResultsViewer    : public Component
{
public:
    SearchResultsViewer();
    ~SearchResultsViewer();

    void paint (Graphics&);
    void resized();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SearchResultsViewer)
};


#endif  // SEARCHRESULTSVIEWER_H_INCLUDED
