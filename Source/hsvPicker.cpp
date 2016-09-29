#include "hsvPicker.h"
#include "LumiverseCore.h"

class HSVPicker::ColourComponentSlider  : public Slider
{
public:
    ColourComponentSlider (const String& name)
        : Slider (name)
    {
      if (name == "hue") {
        setRange(0.0, 360, 0.1);
      }
      else {
        setRange(0, 100, 0.1);
      }
    }

    String getTextFromValue (double value)
    {
      return String(value);
    }

    double getValueFromText (const String& text)
    {
        return (double) text.getDoubleValue();
    }

private:
    JUCE_DECLARE_NON_COPYABLE (ColourComponentSlider)
};

//==============================================================================
class HSVPicker::ColourSpaceMarker  : public Component
{
public:
    ColourSpaceMarker()
    {
        setInterceptsMouseClicks (false, false);
    }

    void paint (Graphics& g) override
    {
        g.setColour (Colour::greyLevel (0.1f));
        g.drawEllipse (1.0f, 1.0f, getWidth() - 2.0f, getHeight() - 2.0f, 1.0f);
        g.setColour (Colour::greyLevel (0.9f));
        g.drawEllipse (2.0f, 2.0f, getWidth() - 4.0f, getHeight() - 4.0f, 1.0f);
    }

private:
    JUCE_DECLARE_NON_COPYABLE (ColourSpaceMarker)
};

//==============================================================================
class HSVPicker::ColourSpaceView  : public Component
{
public:
    ColourSpaceView (HSVPicker& cs, float& hue, float& sat, float& val, const int edgeSize)
        : owner (cs), h (hue), s (sat), v (val), lastVal (0.0f), edge (edgeSize)
    {
        addAndMakeVisible (marker);
        setMouseCursor (MouseCursor::CrosshairCursor);
    }

    void paint (Graphics& g) override
    {
        if (colours.isNull())
        {
            const int width = getWidth();
            const int height = getHeight();
            colours = Image (Image::RGB, width, height, false);

            Image::BitmapData pixels (colours, Image::BitmapData::writeOnly);

            // this is a radial map of the HSV color space
            float centerX = width / 2.0f;
            float centerY = height / 2.0f;
            float radius = centerX;
            Eigen::Vector2d vert(0, centerY);

            for (int y = 0; y < height; y++) {
              for (int x = 0; x < width; x++) {
                Eigen::Vector2d pt(centerX - x, centerY - y);

                // out of bounds
                if (pt.norm() > radius) {
                  pixels.setPixelColour(x, y, Colours::white);
                }
                else {
                  // calculate angle
                  double angle = acos(pt.dot(vert) / (pt.norm() * vert.norm()));
                  if (x < centerX)
                    angle = M_PI * 2 - angle;

                  // calculate color
                  double sat = (pt.norm() / radius);
                  pixels.setPixelColour(x, y, Colour(angle / (M_PI * 2), sat, v, 1.0f));
                }
              }
            }
        }

        g.setOpacity (1.0f);
        g.drawImageTransformed (colours,
                                RectanglePlacement (RectanglePlacement::stretchToFit)
                                    .getTransformToFit (colours.getBounds().toFloat(),
                                                        getLocalBounds().reduced (edge).toFloat()),
                                false);
    }

    void mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
      float centerX = getWidth() / 2;
      float centerY = getHeight() / 2;

      // this is a radial map of the HSV color space
      float radius = centerX;
      Eigen::Vector2d vert(0, centerY);

      // set the hue and sat here.
      Eigen::Vector2d pt(centerX - (e.x - edge), centerY - (e.y - edge));

      // out of bounds
      if (pt.norm() > radius)
        return;

      // calculate angle
      double angle = acos(pt.dot(vert) / (pt.norm() * vert.norm()));
      if ((e.x - edge) < centerX)
        angle = M_PI * 2 - angle;

      owner.setHS(angle / (M_PI * 2), pt.norm() / radius);
    }

    void updateIfNeeded()
    {
      if (lastVal != v)
      {
          lastVal = v;
          colours = Image();
          repaint();
      }

      updateMarker();
    }

    void resized() override
    {
        colours = Image();
        updateMarker();
    }

private:
    HSVPicker& owner;
    float& h;
    float& s;
    float& v;
    float lastVal;
    ColourSpaceMarker marker;
    const int edge;
    Image colours;

    void updateMarker()
    {
      int width = (getWidth() - edge * 2) / 2;
      int height = (getHeight() - edge * 2) / 2;

      // this is a radial map of the HSV color space
      Eigen::Vector2d angle(cos(h * M_PI * 2 - M_PI_2), sin(h * M_PI * 2 - M_PI_2));
      Eigen::Vector2d center(width, height);
      Eigen::Vector2d pt = center + angle * (s * center[0]);

      marker.setBounds (roundToInt (pt[0]) + edge,
                        roundToInt (pt[1]) + edge,
                        edge * 2, edge * 2);
    }

    JUCE_DECLARE_NON_COPYABLE (ColourSpaceView)
};

//==============================================================================
class HSVPicker::ValSelectorMarker  : public Component
{
public:
    ValSelectorMarker()
    {
        setInterceptsMouseClicks (false, false);
    }

    void paint (Graphics& g) override
    {
        const float cw = (float) getWidth();
        const float ch = (float) getHeight();

        Path p;
        p.addTriangle (1.0f, 1.0f,
                       cw * 0.3f, ch * 0.5f,
                       1.0f, ch - 1.0f);

        p.addTriangle (cw - 1.0f, 1.0f,
                       cw * 0.7f, ch * 0.5f,
                       cw - 1.0f, ch - 1.0f);

        g.setColour (Colours::white.withAlpha (0.75f));
        g.fillPath (p);

        g.setColour (Colours::black.withAlpha (0.75f));
        g.strokePath (p, PathStrokeType (1.2f));
    }

private:
    JUCE_DECLARE_NON_COPYABLE (ValSelectorMarker)
};

//==============================================================================
class HSVPicker::ValSelectorComp  : public Component
{
public:
    ValSelectorComp (HSVPicker& cs, float& hue, const int edgeSize)
        : owner (cs), v (hue), edge (edgeSize)
    {
        addAndMakeVisible (marker);
    }

    void paint (Graphics& g) override
    {
        ColourGradient cg;
        cg.isRadial = false;
        cg.point1.setXY (0.0f, (float) edge);
        cg.point2.setXY (0.0f, (float) (getHeight() - edge));

        cg.addColour(0, Colours::white);
        cg.addColour(1, Colours::black);

        g.setGradientFill (cg);
        g.fillRect (getLocalBounds().reduced (edge));
    }

    void resized() override
    {
        marker.setBounds (0, roundToInt ((getHeight() - edge * 2) * (1 - v)), getWidth(), edge * 2);
    }

    void mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        owner.setVal (1 - ((e.y - edge) / (float) (getHeight() - edge * 2)));
    }

    void updateIfNeeded()
    {
        resized();
    }

private:
    HSVPicker& owner;
    float& v;
    ValSelectorMarker marker;
    const int edge;

    JUCE_DECLARE_NON_COPYABLE (ValSelectorComp)
};

//==============================================================================
class HSVPicker::SwatchComponent   : public Component
{
public:
    SwatchComponent (HSVPicker& cs, int itemIndex)
        : owner (cs), index (itemIndex)
    {
    }

    void paint (Graphics& g) override
    {
        const Colour c (owner.getSwatchColour (index));

        g.fillCheckerBoard (getLocalBounds(), 6, 6,
                            Colour (0xffdddddd).overlaidWith (c),
                            Colour (0xffffffff).overlaidWith (c));
    }

    void mouseDown (const MouseEvent&) override
    {
        PopupMenu m;
        m.addItem (1, TRANS("Use this swatch as the current colour"));
        m.addSeparator();
        m.addItem (2, TRANS("Set this swatch to the current colour"));

        m.showMenuAsync (PopupMenu::Options().withTargetComponent (this),
                         ModalCallbackFunction::forComponent (menuStaticCallback, this));
    }

private:
    HSVPicker& owner;
    const int index;

    static void menuStaticCallback (int result, SwatchComponent* comp)
    {
        if (comp != nullptr)
        {
            if (result == 1)
                comp->setColourFromSwatch();
            else if (result == 2)
                comp->setSwatchFromColour();
        }
    }

    void setColourFromSwatch()
    {
        owner.setCurrentColour (owner.getSwatchColour (index));
    }

    void setSwatchFromColour()
    {
        if (owner.getSwatchColour (index) != owner.getCurrentColour())
        {
            owner.setSwatchColour (index, owner.getCurrentColour());
            repaint();
        }
    }

    JUCE_DECLARE_NON_COPYABLE (SwatchComponent)
};

//==============================================================================
HSVPicker::HSVPicker (const int edge, const int gapAroundColourSpaceComponent)
    : colour (Colours::white),
      edgeGap (edge)
{
    updateHSV();

    addAndMakeVisible (sliders[0] = new ColourComponentSlider ("hue"));
    addAndMakeVisible (sliders[1] = new ColourComponentSlider ("sat"));
    addAndMakeVisible (sliders[2] = new ColourComponentSlider ("val"));

    for (int i = 3; --i >= 0;)
        sliders[i]->addListener (this);

    addAndMakeVisible (colourSpace = new ColourSpaceView (*this, h, s, v, gapAroundColourSpaceComponent));
    addAndMakeVisible (valSelector = new ValSelectorComp (*this, v,  gapAroundColourSpaceComponent));

    update (dontSendNotification);
}

HSVPicker::~HSVPicker()
{
    dispatchPendingMessages();
    swatchComponents.clear();
}

//==============================================================================
Colour HSVPicker::getCurrentColour() const
{
  return colour;
}

void HSVPicker::setCurrentColour (Colour c, NotificationType notification)
{
  if (c != colour)
  {
    colour = c;

    updateHSV();
    update (notification);
  }
}

void HSVPicker::setHue (float newH)
{
    newH = jlimit (0.0f, 1.0f, newH);

    if (h != newH)
    {
        h = newH;
        colour = Colour (h, s, v, colour.getFloatAlpha());
        update (sendNotification);
    }
}

void HSVPicker::setVal(float newV)
{
  newV = jlimit (0.0f, 1.0f, newV);

  if (v != newV)
  {
      v = newV;
      colour = Colour (h, s, v, colour.getFloatAlpha());
      update (sendNotification);
  }
}

void HSVPicker::setSV (float newS, float newV)
{
    newS = jlimit (0.0f, 1.0f, newS);
    newV = jlimit (0.0f, 1.0f, newV);

    if (s != newS || v != newV)
    {
        s = newS;
        v = newV;
        colour = Colour (h, s, v, colour.getFloatAlpha());
        update (sendNotification);
    }
}

void HSVPicker::setHS(float newH, float newS)
{
  newH = jlimit(0.0f, 1.0f, newH);
  newS = jlimit(0.0f, 1.0f, newS);

  if (s != newS || h != newH) {
    h = newH;
    s = newS;
    colour = Colour(h, s, v, colour.getFloatAlpha());
    update(sendNotification);
  }
}

//==============================================================================
void HSVPicker::updateHSV()
{
    colour.getHSB (h, s, v);
}

void HSVPicker::update (NotificationType notification)
{
    if (sliders[0] != nullptr)
    {
        sliders[0]->setValue (colour.getHue() * 360, dontSendNotification);
        sliders[1]->setValue (colour.getSaturation() * 100, dontSendNotification);
        sliders[2]->setValue (colour.getBrightness() * 100,  dontSendNotification);
    }

    if (colourSpace != nullptr)
    {
        colourSpace->updateIfNeeded();
        valSelector->updateIfNeeded();
    }

    repaint (previewArea);

    if (notification != dontSendNotification)
        sendChangeMessage();

    if (notification == sendNotificationSync)
        dispatchPendingMessages();
}

//==============================================================================
void HSVPicker::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));

    const Colour currentColour (getCurrentColour());

    g.fillCheckerBoard (previewArea, 10, 10,
                        Colour (0xffdddddd).overlaidWith (currentColour),
                        Colour (0xffffffff).overlaidWith (currentColour));

    g.setColour (Colours::white.overlaidWith (currentColour).contrasting());
    g.setFont (Font (14.0f, Font::bold));
    String colorDisp = "H: " + String(currentColour.getHue() * 360.0) + " S: " + String(currentColour.getSaturation() * 100) + " V: " + String(currentColour.getBrightness() * 100);
    g.drawText (colorDisp,
                previewArea, Justification::centred, false);

    g.setColour (findColour (labelTextColourId));
    g.setFont (11.0f);

    for (int i = 3; --i >= 0;)
    {
        if (sliders[i]->isVisible())
            g.drawText (sliders[i]->getName() + ":",
                        0, sliders[i]->getY(),
                        sliders[i]->getX() - 8, sliders[i]->getHeight(),
                        Justification::centredRight, false);
    }
}

void HSVPicker::resized()
{
    const int swatchesPerRow = 8;
    const int swatchHeight = 22;

    const int numSliders = 3;
    const int numSwatches = getNumSwatches();

    const int swatchSpace = numSwatches > 0 ? edgeGap + swatchHeight * ((numSwatches + 7) / swatchesPerRow) : 0;
    const int sliderSpace = jmin (22 * numSliders + edgeGap, proportionOfHeight (0.3f));
    const int topSpace = jmin (30 + edgeGap * 2, proportionOfHeight (0.2f));

    previewArea.setBounds (edgeGap, edgeGap, getWidth() - edgeGap * 2, topSpace - edgeGap * 2);

    int y = topSpace;

    const int hueWidth = jmin (50, proportionOfWidth (0.15f));

    colourSpace->setBounds (edgeGap, y,
                            getWidth() - hueWidth - edgeGap - 4,
                            getHeight() - topSpace - sliderSpace - swatchSpace - edgeGap);

    valSelector->setBounds (colourSpace->getRight() + 4, y,
                            getWidth() - edgeGap - (colourSpace->getRight() + 4),
                            colourSpace->getHeight());

    y = getHeight() - sliderSpace - swatchSpace - edgeGap;

    const int sliderHeight = jmax (4, sliderSpace / numSliders);

    for (int i = 0; i < numSliders; ++i)
    {
        sliders[i]->setBounds (proportionOfWidth (0.2f), y,
                               proportionOfWidth (0.72f), sliderHeight - 2);

        y += sliderHeight;
    }

    if (numSwatches > 0)
    {
        const int startX = 8;
        const int xGap = 4;
        const int yGap = 4;
        const int swatchWidth = (getWidth() - startX * 2) / swatchesPerRow;
        y += edgeGap;

        if (swatchComponents.size() != numSwatches)
        {
            swatchComponents.clear();

            for (int i = 0; i < numSwatches; ++i)
            {
                SwatchComponent* const sc = new SwatchComponent (*this, i);
                swatchComponents.add (sc);
                addAndMakeVisible (sc);
            }
        }

        int x = startX;

        for (int i = 0; i < swatchComponents.size(); ++i)
        {
            SwatchComponent* const sc = swatchComponents.getUnchecked(i);

            sc->setBounds (x + xGap / 2,
                           y + yGap / 2,
                           swatchWidth - xGap,
                           swatchHeight - yGap);

            if (((i + 1) % swatchesPerRow) == 0)
            {
                x = startX;
                y += swatchHeight;
            }
            else
            {
                x += swatchWidth;
            }
        }
    }
}

void HSVPicker::sliderValueChanged (Slider*)
{
    if (sliders[0] != nullptr)
        setCurrentColour (Colour ((float)sliders[0]->getValue() / 360.0f,
                                  (float)sliders[1]->getValue() / 100.0f,
                                  (float)sliders[2]->getValue() / 100.0f,
                                  1.0f));
}

//==============================================================================
int HSVPicker::getNumSwatches() const
{
    return 0;
}

Colour HSVPicker::getSwatchColour (const int) const
{
    jassertfalse; // if you've overridden getNumSwatches(), you also need to implement this method
    return Colours::black;
}

void HSVPicker::setSwatchColour (const int, const Colour&) const
{
    jassertfalse; // if you've overridden getNumSwatches(), you also need to implement this method
}
