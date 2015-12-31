/*
  ==============================================================================

    SearchResultsViewer.cpp
    Created: 15 Dec 2015 5:07:58pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SearchResultsViewer.h"

//==============================================================================
SearchResultsViewer::SearchResultsViewer()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

SearchResultsViewer::~SearchResultsViewer()
{
}

void SearchResultsViewer::paint (Graphics& g)
{
  /* This demo code just fills the component's background and
     draws some placeholder text to get you started.

     You should replace everything in this method with your own
     drawing code..
  */

  g.fillAll(Colour(0xff333333));

  g.setColour (Colours::lightblue);
  g.setFont (14.0f);
  g.drawText ("SearchResultsViewer", getLocalBounds(),
              Justification::centred, true);   // draw some placeholder text
}

void SearchResultsViewer::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}