/*
  ==============================================================================

    SceneViewer.cpp
    Created: 15 Dec 2015 5:07:32pm
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SceneViewer.h"
#include <thread>
#include <chrono>

//==============================================================================
RenderBackgroundThread::RenderBackgroundThread(ArnoldAnimationPatch* p, uint8* bufptr) :
  ThreadWithProgressWindow("Rendering Current Scene", true, true), _p(p), _bufptr(bufptr)
{
  setStatusMessage("Preparing...");
  setProgress(-1.0);
}

RenderBackgroundThread::~RenderBackgroundThread()
{
}

void RenderBackgroundThread::run()
{
  // little bit of thread inception here...
  thread r(&RenderBackgroundThread::renderLoop, this);
  r.detach();

  float prog;
  setStatusMessage("Rendering...");
  while (prog < 1) {
    this_thread::sleep_for(std::chrono::milliseconds(20));
    prog = _p->getPercentage() / 100.0f;

    if (threadShouldExit()) {
      _p->forceInterrupt();
      return;
    }
    
    setProgress(prog);
  }

  setProgress(-1.0);
  setStatusMessage("Finalizing frame...");

  wait(1000);
}

void RenderBackgroundThread::threadComplete(bool userPressedCancel)
{
  if (userPressedCancel) {
    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
      "Render Current Scene",
      "Render canceled");
  }

  delete this;
}

void RenderBackgroundThread::renderLoop() {
  _p->renderSingleFrameToBuffer(getRig()->getDeviceRaw(), _bufptr);
}

//==============================================================================
SceneViewer::SceneViewer()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
  _currentRender = Image();
}

SceneViewer::~SceneViewer()
{
}

void SceneViewer::paint (Graphics& g)
{
  /* This demo code just fills the component's background and
     draws some placeholder text to get you started.

     You should replace everything in this method with your own
     drawing code..
  */
  g.fillAll(Colour(0xff333333));

  g.drawImageWithin(_currentRender, 0, 0, getWidth(), getHeight(), RectanglePlacement::centred);
}

void SceneViewer::resized()
{
  // This method is where you should set the bounds of any child
  // components that your component contains..

}

void SceneViewer::renderScene() {
  // Renders a scene using Arnold through Lumiverse.
  Rig* rig = getRig();

  // Find the patch, we search for the first ArnoldAnimationPatch we can find.
  ArnoldAnimationPatch* p = getAnimationPatch();

  if (p == nullptr) {
    getRecorder()->log(RENDER, "Render failed. Could not find ArnoldAnimationPatch.");
    return;
  }

  // Get the image dimensions
  int width = p->getWidth();
  int height = p->getHeight();

  _currentRender = Image(Image::ARGB, width, height, true);
  uint8* bufptr = Image::BitmapData(_currentRender, Image::BitmapData::readWrite).getPixelPointer(0,0);

  //p->renderSingleFrameToBuffer(rig->getDeviceRaw(), bufptr);
  //p->setSamples(1);
  (new RenderBackgroundThread(p, bufptr))->runThread();

  repaint();
}