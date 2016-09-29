#ifndef HSVPICKER_H_INCLUDED
#define HSVPICKER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/**
  A component that lets the user choose a colour.

  This class is a modification of the HSVPicker class.
  This color picker shows a HSV view of the color space with H and S being
  plotted on a polar grid, and V being a slider of the right
*/
class  HSVPicker : public Component, public ChangeBroadcaster, protected SliderListener
{
public:

  //==============================================================================
  /** Creates a HSVPicker object.

  The edgeGap value specifies the amount of space to leave around the edge.

  gapAroundColourSpaceComponent indicates how much of a gap to put around the
  colourspace and hue selector components.
  */
  HSVPicker(int edgeGap = 4, int gapAroundColourSpaceComponent = 7);

  /** Destructor. */
  ~HSVPicker();

  //==============================================================================
  /** Returns the colour that the user has currently selected.

  The HSVPicker class is also a ChangeBroadcaster, so listeners can
  register to be told when the colour changes.

  @see setCurrentColour
  */
  Colour getCurrentColour() const;

  /** Changes the colour that is currently being shown. */
  void setCurrentColour(Colour newColour, NotificationType notificationType = sendNotification);

  //==============================================================================
  /** Tells the selector how many preset colour swatches you want to have on the component.

  To enable swatches, you'll need to override getNumSwatches(), getSwatchColour(), and
  setSwatchColour(), to return the number of colours you want, and to set and retrieve
  their values.
  */
  virtual int getNumSwatches() const;

  /** Called by the selector to find out the colour of one of the swatches.

  Your subclass should return the colour of the swatch with the given index.

  To enable swatches, you'll need to override getNumSwatches(), getSwatchColour(), and
  setSwatchColour(), to return the number of colours you want, and to set and retrieve
  their values.
  */
  virtual Colour getSwatchColour(int index) const;

  /** Called by the selector when the user puts a new colour into one of the swatches.

  Your subclass should change the colour of the swatch with the given index.

  To enable swatches, you'll need to override getNumSwatches(), getSwatchColour(), and
  setSwatchColour(), to return the number of colours you want, and to set and retrieve
  their values.
  */
  virtual void setSwatchColour(int index, const Colour& newColour) const;


  //==============================================================================
  /** A set of colour IDs to use to change the colour of various aspects of the keyboard.

  These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
  methods.

  @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
  */
  enum ColourIds
  {
    backgroundColourId = 0x1007000,    /**< the colour used to fill the component's background. */
    labelTextColourId = 0x1007001     /**< the colour used for the labels next to the sliders. */
  };


private:
  //==============================================================================
  class ColourSpaceView;
  class ValSelectorComp;
  class SwatchComponent;
  class ColourComponentSlider;
  class ColourSpaceMarker;
  class ValSelectorMarker;
  friend class ColourSpaceView;
  friend struct ContainerDeletePolicy<ColourSpaceView>;
  friend class ValSelectorComp;
  friend struct ContainerDeletePolicy<ValSelectorComp>;

  Colour colour;
  float h, s, v;
  ScopedPointer<Slider> sliders[4];
  ScopedPointer<ColourSpaceView> colourSpace;
  ScopedPointer<ValSelectorComp> valSelector;
  OwnedArray<SwatchComponent> swatchComponents;
  int edgeGap;
  Rectangle<int> previewArea;

  void setHue(float newH);
  void setVal(float newV);
  void setSV(float newS, float newV);
  void setHS(float newH, float newS);
  void updateHSV();
  void update(NotificationType);
  void sliderValueChanged(Slider*) override;
  void paint(Graphics&) override;
  void resized() override;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HSVPicker)

#if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // This constructor is here temporarily to prevent old code compiling, because the parameters
    // have changed - if you get an error here, update your code to use the new constructor instead..
    HSVPicker(bool);
#endif
};


#endif   // HSVPICKER_H_INCLUDED
