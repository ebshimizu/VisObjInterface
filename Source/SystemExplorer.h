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

class SearchResultsContainer;

// Building blocks for a heirarchical view of the search results.
// In each explorer, the specified system is displayed with all other locked lights
// in the rig. When a result is chosen, the specified system is locked and all other
// views are updated
class SystemExplorer : public Component, public ComboBox::Listener, public Button::Listener,
  public Slider::Listener, public ChangeListener
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
  virtual void changeListenerCallback(ChangeBroadcaster* source) override;
  virtual void mouseDown(const MouseEvent& event) override;

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
  void toggleIntensPin();
  void toggleColorPin();

  void empty();

  // Updates the pin status of the view
  void updatePinState();

  // Updates the intensity slider value
  void updateIntensSlider();

  // intensity slider callbacks
  virtual void sliderDragEnded(Slider* s);
  virtual void sliderValueChanged(Slider* s);

private:
  void updateSingleImage(shared_ptr<SearchResultContainer> result);

  // Returns the distance between the two results in terms of the selected devices
  // for this particular view. Uses avg L2 param distance at the moment.
  double filteredDist(shared_ptr<SearchResultContainer> r1, shared_ptr<SearchResultContainer> r2);

  // because we have a viewport the results are contained within a different
  // object to place in the viewport
  class SystemExplorerResults : public Component {
  public:
    SystemExplorerResults();

    virtual void resized() override;

    Array<shared_ptr<SearchResultContainer> > _results;
  };

  class ParamShifter : public Component {
  public:
    ParamShifter(Slider::Listener* listener);

    void resized() override;
    void paint(Graphics& g) override;

  private:
    Slider _intens;
    Slider _hue;
    Slider _sat;
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
  TextButton _intensPin;
  TextButton _colorPin;
  TextButton _color;

  Colour _currentColor;

  string _name;
  Slider _intens;

  bool _isBlackout;
  bool _isSolo;

  // true when every light in the selection is locked
  bool _isIntensPinned;
  bool _isColorPinned;

  double _distThreshold;
};

class SystemExplorerContainer : public Component, public Button::Listener
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
  void addContainer(DeviceSet devices);
  void addResult(shared_ptr<SearchResultContainer> result);
  void clear();
  void empty(); // remove results but not delete
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

class ExplorerPanel : public Component, public Button::Listener
{
public:
  ExplorerPanel();
  ~ExplorerPanel();

  void initWithResults(SearchResultsContainer* rc);
  void resized() override;
  void paint(Graphics& g) override;

  void buttonClicked(Button* b) override;

  // fills the container with default system views
  void populateSystemViews();

  void clear();

  SystemExplorerContainer* getContainer();

private:
  SystemExplorerContainer* _exp;
  SearchResultsContainer* _results;
  Viewport* _viewer;
};

#endif  // SYSTEMEXPLORER_H_INCLUDED
