/*
  ==============================================================================

    SystemExplorer.h
    Created: 11 Nov 2016 10:05:09am
    Author:  falindrith

  ==============================================================================
*/

#ifndef SYSTEMEXPLORER_H_INCLUDED
#define SYSTEMEXPLORER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "globals.h"
#include "SearchResultContainer.h"

extern class SearchResultsContainer;

// Building blocks for a heirarchical view of the search results.
// In each explorer, the specified system is displayed with all other locked lights
// in the rig. When a result is chosen, the specified system is locked and all other
// views are updated
class SystemExplorer : public Component, public ComboBoxListener, public ButtonListener
{
public:
  SystemExplorer(string name, string system);
  SystemExplorer(string name);
  SystemExplorer(Array<shared_ptr<SearchResultContainer> > results, string name);
  SystemExplorer(Array<shared_ptr<SearchResultContainer> > results, string name, DeviceSet selected);
  ~SystemExplorer();

  void addNewResult(shared_ptr<SearchResultContainer> result);

  // updates the thumbnails. Note that the results are unchanged, but the images
  // get updated based on what's locked
  void updateAllImages(Snapshot* rigState);

  virtual void paint(Graphics& g) override;
  virtual void resized() override;
  DeviceSet getViewedDevices();

  virtual void comboBoxChanged(ComboBox* c);

  // sorts the elements in the explorer by a specified criteria
  void sort(string method);

  // fills the dropdown box with the proper options
  // and initialized the required stuff
  void init();

  // buttons!
  virtual void buttonClicked(Button* b);

  // turns the selected lights off in all parts of the interface
  void blackout();
  void unBlackout();

  // Turns all lights off except the selected lights in all parts of the interface
  void solo();
  void unSolo();

  // locks or unlocks the selected lights 
  void togglePin();

  void clear();

  // Updates the pin status of the view
  void updatePinState();

private:
  void updateSingleImage(shared_ptr<SearchResultContainer> result);

  // because we have a viewport the results are contained within a different
  // object to place in the viewport
  class SystemExplorerResults : public Component {
  public:
    SystemExplorerResults();

    virtual void resized() override;

    Array<shared_ptr<SearchResultContainer> > _results;
  };

  SystemExplorerResults* _container;
  DeviceSet _selected;
  Viewport* _rowElems;
  Image _currentImg;
  Snapshot* _currentState;

  // state for updating images
  Snapshot* _rigState;

  // for temporary ops
  Snapshot* _temp;

  TextButton _blackout;
  TextButton _solo;
  TextButton _pin;

  string _name;

  bool _isBlackout;
  bool _isSolo;

  // true when every light in the selection is locked
  bool _isPinned;
};

class SystemExplorerContainer : public Component, public ButtonListener
{
public:
  SystemExplorerContainer(SearchResultsContainer* c);
  ~SystemExplorerContainer();

  virtual void paint(Graphics& g) override;
  virtual void resized() override;
  void setHeight(int height);

  void sort(string method);
  void addContainer(string system);
  void addContainer();
  void addResult(shared_ptr<SearchResultContainer> result);
  void clear();
  void updateImages();

  virtual void buttonClicked(Button* b) override;

private:
  void addContainer(SystemExplorer* e);

  Array<SystemExplorer*> _explorers;
  SearchResultsContainer* _rc;

  int _rowHeight;

  TextButton _add;
  Array<TextButton*> _deleteButtons;

  int _counter;

  class UpdateImageJob : public ThreadPoolJob {
  public:
    UpdateImageJob(SystemExplorer* e, Snapshot* state);
    ~UpdateImageJob();

    virtual JobStatus runJob();
    SystemExplorer* _e;
    Snapshot* _state;
  };
};

class ExplorerPanel : public Component, public ButtonListener
{
public:
  ExplorerPanel();
  ~ExplorerPanel();

  void initWithResults(SearchResultsContainer* rc);
  void resized() override;
  void paint(Graphics& g) override;

  void buttonClicked(Button* b) override;

  SystemExplorerContainer* getContainer();

private:
  SystemExplorerContainer* _exp;
  SearchResultsContainer* _results;
  Viewport* _viewer;
};

#endif  // SYSTEMEXPLORER_H_INCLUDED
