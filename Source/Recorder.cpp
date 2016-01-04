/*
  ==============================================================================

    Recorder.cpp
    Created: 16 Dec 2015 11:56:45am
    Author:  falindrith

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Recorder.h"

//==============================================================================
Recorder::Recorder()
{
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.

  _filename = "actions.log";
  _file.open(_filename, ios::app);

  _start = chrono::steady_clock::now();
  
  time_t startTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
  _file << "Interface initialized on " << ctime(&startTime) << "\n";
  _file.flush();
}

Recorder::~Recorder()
{
  _file << "================================================================================\n";
  _file.close();
}

void Recorder::setFilename(string filename)
{
  if (_file.is_open())
    _file.close();

  _filename = filename;
  _file.open(_filename, ios::app);
}

void Recorder::log(actionType type, string message)
{
  chrono::time_point<chrono::steady_clock> now = chrono::steady_clock::now();
  chrono::duration<double> seconds = now - _start;

  string t;
  if (type == SYSTEM)
    t = "[s]";
  else if (type == ACTION)
    t = "[a]";
  else if (type == RENDER)
    t = "[r]";

  _file << t << " (" << seconds.count() << ")\t" << message << "\n";
  _file.flush();
}
