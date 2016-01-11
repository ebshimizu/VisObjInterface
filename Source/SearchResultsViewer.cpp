/*
  ==============================================================================

    SearchResultsViewer.cpp
    Created: 15 Dec 2015 5:07:58pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SearchResultsViewer.h"

SearchResultsRenderer::SearchResultsRenderer(Array<AttributeSearchResult*> results) :
  ThreadWithProgressWindow("Rendering Thumbnails...", true, true), _results(results)
{
}

SearchResultsRenderer::~SearchResultsRenderer()
{
}

void SearchResultsRenderer::run() {
  setProgress(-1);
  int numScenes = _results.size();

  // Set render options
  auto p = getAnimationPatch();
  int width = getGlobalSettings()->_renderWidth * getGlobalSettings()->_thumbnailRenderScale;
  int height = getGlobalSettings()->_renderHeight * getGlobalSettings()->_thumbnailRenderScale;
  p->setDims(width, height);
  p->setSamples(getGlobalSettings()->_thumbnailRenderSamples);

  int i = 0;
  for (auto r : _results) {
    if (threadShouldExit())
      return;

    setProgress((float) i / (float)numScenes);
    setStatusMessage(String(i) + "/" + String(numScenes));

    Image img = Image(Image::ARGB, width, height, true);
    uint8* bufptr = Image::BitmapData(img, Image::BitmapData::readWrite).getPixelPointer(0, 0);

    p->renderSingleFrameToBuffer(r->getSearchResult()->_scene->getDevices(), bufptr);

    r->setImage(img);
    i++;
  }

  setProgress(1);
}

void SearchResultsRenderer::threadComplete(bool userPressedCancel)
{
  if (userPressedCancel) {
    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
      "Render Thumbnails",
      "Render canceled");
  }

  auto p = getAnimationPatch();
  int width = getGlobalSettings()->_renderWidth;
  int height = getGlobalSettings()->_renderHeight;
  p->setDims(width, height);
  p->setSamples(getGlobalSettings()->_stageRenderSamples);

  delete this;
}

//==============================================================================

SearchResultsContainer::SearchResultsContainer()
{

}

SearchResultsContainer::~SearchResultsContainer()
{
  for (const auto& c : _results) {
    delete c;
  }
}

void SearchResultsContainer::paint(Graphics & g)
{
  g.fillAll(Colour(0xff333333));
}

void SearchResultsContainer::resized()
{
  int elemWidth = _width / _resultsPerRow;
  int elemHeight = elemWidth * (9.0 / 16.0);

  int i = 0;
  for (const auto& result : _results) {
    int r = i / 8;
    int c = i % 8;

    result->setBounds(c * elemWidth, r * elemHeight, elemWidth, elemHeight);
    i++;
  }
}

void SearchResultsContainer::display(vector<SearchResult*> results)
{
  for (const auto& c : _results) {
    delete c;
  } 
  _results.clear();

  for (const auto& result : results) {
    AttributeSearchResult* res = new AttributeSearchResult(result);
    addAndMakeVisible(res);

    _results.add(res);
  }

  // render
  (new SearchResultsRenderer(_results))->runThread();
}

void SearchResultsContainer::setWidth(int width)
{
  _width = width;
  int rows = (int)(size(_results) / _resultsPerRow) + 1;
  int elemWidth = _width / _resultsPerRow;
  int elemHeight = elemWidth * (9.0 / 16.0);
  _height = rows * elemHeight;
  setBounds(0, 0, _width, _height);
}

//==============================================================================
SearchResultsViewer::SearchResultsViewer()
{
  _container = new SearchResultsContainer();
  _viewer = new Viewport();
  _viewer->setViewedComponent(_container);
  addAndMakeVisible(_viewer);
}

SearchResultsViewer::~SearchResultsViewer()
{
  delete _container;
  delete _viewer;
}

void SearchResultsViewer::paint (Graphics& g)
{
  /* This demo code just fills the component's background and
     draws some placeholder text to get you started.

     You should replace everything in this method with your own
     drawing code..
  */

  g.fillAll(Colour(0xff333333));
}

void SearchResultsViewer::resized()
{
  auto bounds = getLocalBounds();

  _viewer->setBounds(bounds);
  _container->setWidth(_viewer->getMaximumVisibleWidth());
}

void SearchResultsViewer::display(vector<SearchResult*> results)
{
  _container->display(results);
  _container->resized();
  _container->repaint();
}
