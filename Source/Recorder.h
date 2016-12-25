/*
  ==============================================================================

    Recorder.h
    Created: 16 Dec 2015 11:56:45am
    Author:  falindrith

  ==============================================================================
*/

#ifndef RECORDER_H_INCLUDED
#define RECORDER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <iostream>
#include <fstream>
#include <chrono>

using namespace std;

enum actionType {
  SYSTEM,   // Actions that don't deal with the Lumiverse Rig
  ACTION,   // Actions that do deal with the Lumiverse Rig
  RENDER,   // A render call
  HOVER,    // A hovered scene
  STATE     // Current state of the rig
};

//==============================================================================
/*
Records all actions taken in the interface
Times recorded are relative to application initialization.
*/
class Recorder
{
public:
  Recorder();
  ~Recorder();

  void setFilename(string filename);
  void log(actionType type, string message);

private:
  string _filename;
  ofstream _file;

  chrono::time_point<chrono::steady_clock> _start;
};


#endif  // RECORDER_H_INCLUDED
