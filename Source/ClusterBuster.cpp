/*
  ==============================================================================

    ClusterBuster.cpp
    Created: 5 Feb 2016 1:48:11pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "ClusterBuster.h"

//==============================================================================
ClusterBuster::ClusterBuster(Array<AttributeSearchResult*> results) : _results(results)
{
  _elemWidth = getGlobalSettings()->_renderWidth * getGlobalSettings()->_thumbnailRenderScale;
  _elemHeight = getGlobalSettings()->_renderHeight * getGlobalSettings()->_thumbnailRenderScale;

  _height = _elemHeight * results.size();

  int rowMax = 0;
  for (auto r : results) {
    if (r->getClusterElements().size() > rowMax)
      rowMax = r->getClusterElements().size();
  }

  _width = _elemWidth * (rowMax + 1);
  setSize(_width, _height);
}

ClusterBuster::~ClusterBuster()
{
}

void ClusterBuster::paint (Graphics& g)
{
  auto lbounds = getLocalBounds();
  for (auto r : _results) {
    auto row = lbounds.removeFromTop(_elemHeight);
    auto a = row.removeFromLeft(_elemWidth).reduced(2);
    g.drawImageWithin(r->getImage(), a.getX(), a.getY(), a.getWidth(), a.getHeight(), RectanglePlacement::centred);

    for (auto e : r->getClusterElements()) {
      auto a = row.removeFromLeft(_elemWidth).reduced(2);
      g.drawImageWithin(e->getImage(), a.getX(), a.getY(), a.getWidth(), a.getHeight(), RectanglePlacement::centred);
    }
  }
}

void ClusterBuster::resized()
{

}

ClusterBusterWindow::ClusterBusterWindow(Array<AttributeSearchResult*> results) :
  DocumentWindow("Cluster Viewer", Colour(0xff333333), allButtons)
{
  _viewer = new Viewport();
  _cluster = new ClusterBuster(results);
  _viewer->setViewedComponent(_cluster);
  setContentOwned(_viewer, false);
}

ClusterBusterWindow::~ClusterBusterWindow()
{
  getRecorder()->log(ACTION, "All Clusters window closed");
  delete _viewer;
}

void ClusterBusterWindow::resized()
{
  _viewer->setBounds(getLocalBounds());
}

void ClusterBusterWindow::closeButtonPressed()
{
  delete this;
}
