#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CustomLookAndFeel::CustomLookAndFeel()
{
}

void CustomLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle,
                                         juce::Slider& slider)
{
    auto radius = (float) juce::jmin (width / 2, height / 2) - 8.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Track Background (dark glass track)
    juce::Path backgroundTrack;
    backgroundTrack.addCentredArc (centreX, centreY, radius, radius, 0.0f,
                                   rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (0x12ffffff)); // dark transparent white
    g.strokePath (backgroundTrack, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Active track fill (glow gradient)
    if (sliderPos > 0.0f)
    {
        juce::Path activeTrack;
        activeTrack.addCentredArc (centreX, centreY, radius, radius, 0.0f,
                                   rotaryStartAngle, angle, true);
        
        juce::ColourGradient grad (juce::Colour (0xffe67e22), centreX, ry, // Vintage copper orange
                                   juce::Colour (0xffd35400), centreX, ry + rw, false);
        g.setGradientFill (grad);
        g.strokePath (activeTrack, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Dial face
    g.setColour (juce::Colour (0x1b1c24));
    g.fillEllipse (rx + 4, ry + 4, rw - 8, rw - 8);

    g.setColour (juce::Colour (0x2b2e3c));
    g.drawEllipse (rx + 4, ry + 4, rw - 8, rw - 8, 1.5f);

    // Pointer (Gold/copper colored indicator line)
    juce::Path pointer;
    auto pointerLength = radius * 0.5f;
    auto pointerThickness = 3.0f;
    pointer.addRectangle (-pointerThickness * 0.5f, -radius + 4.0f, pointerThickness, pointerLength);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));

    g.setColour (juce::Colour (0xfff39c12)); // Orange-gold tip
    g.fillPath (pointer);
}

//==============================================================================
RamerSynthAudioProcessorEditor::RamerSynthAudioProcessorEditor (RamerSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Apply LookAndFeel to all sliders
    juce::Slider* sliders[] = { &cutoffSlider, &attackSlider, &releaseSlider, &driveSlider, &volumeSlider };
    
    juce::Label* labels[] = { &cutoffLabel, &attackLabel, &releaseLabel, &driveLabel, &volumeLabel };
                              
    juce::String paramNames[] = { "Cutoff", "Attack", "Release", "Drive", "Volume" };

    for (int i = 0; i < 5; ++i)
    {
        sliders[i]->setLookAndFeel (&customLookAndFeel);
        sliders[i]->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        sliders[i]->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
        sliders[i]->setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffa0a5b5));
        sliders[i]->setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        sliders[i]->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (sliders[i]);

        labels[i]->setText (paramNames[i], juce::dontSendNotification);
        labels[i]->setJustificationType (juce::Justification::centred);
        labels[i]->setFont (juce::FontOptions (13.0f, juce::Font::bold));
        labels[i]->setColour (juce::Label::textColourId, juce::Colour (0xffeceff4));
        addAndMakeVisible (labels[i]);
    }

    // Bind parameters to APVTS
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "cutoff", cutoffSlider);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "attack", attackSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "release", releaseSlider);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "drive", driveSlider);
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "volume", volumeSlider);

    // Window sizing (600px width, 400px height)
    setSize (600, 400);
}

RamerSynthAudioProcessorEditor::~RamerSynthAudioProcessorEditor()
{
    // Clean up LookAndFeel before sliders go out of scope
    juce::Slider* sliders[] = { &cutoffSlider, &attackSlider, &releaseSlider, &driveSlider, &volumeSlider };
    for (auto* s : sliders)
        s->setLookAndFeel (nullptr);
}

//==============================================================================
void RamerSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Premium Slate Gradient Background
    juce::ColourGradient bgGrad (juce::Colour (0xff202430), 300, 0,
                                 juce::Colour (0xff0f1015), 300, 400, false);
    g.setGradientFill (bgGrad);
    g.fillAll();

    // Subtle neon copper glow at the top center
    juce::ColourGradient topGlow (juce::Colour (0x1aff8c00), 300, 0,
                                  juce::Colours::transparentBlack, 300, 150, true);
    g.setGradientFill (topGlow);
    g.fillAll();

    // Draw Title Header
    g.setColour (juce::Colour (0xffeceff4));
    g.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    g.drawText ("ramer-synth", 30, 20, 200, 30, juce::Justification::left);

    g.setColour (juce::Colour (0xffd35400)); // Vintage copper highlight
    g.setFont (juce::FontOptions (12.0f, juce::Font::italic));
    g.drawText ("ramerdigital", 30, 45, 200, 20, juce::Justification::left);

    // Draw glassmorphic panel cards for UI layout
    // Card 1: Filter & Saturation Section (Left Column)
    g.setColour (juce::Colour (0x0bffffff)); // glass fill
    g.fillRoundedRectangle (20, 80, 170, 290, 12.0f);
    g.setColour (juce::Colour (0x10ffffff)); // light border
    g.drawRoundedRectangle (20, 80, 170, 290, 12.0f, 1.5f);

    // Section title
    g.setColour (juce::Colour (0xff8fbcbb));
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.drawText ("FILTER & DRIVE", 30, 88, 150, 15, juce::Justification::centred);

    // Card 2: Amplifier Envelope Section (Middle Column)
    g.setColour (juce::Colour (0x0bffffff)); // glass fill
    g.fillRoundedRectangle (210, 80, 170, 290, 12.0f);
    g.setColour (juce::Colour (0x10ffffff)); // light border
    g.drawRoundedRectangle (210, 80, 170, 290, 12.0f, 1.5f);

    // Section title
    g.setColour (juce::Colour (0xff8fbcbb));
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.drawText ("AMPLIFIER ENVELOPE", 220, 88, 150, 15, juce::Justification::centred);

    // Card 3: Master Volume Section (Right Column)
    g.setColour (juce::Colour (0x0bffffff)); // glass fill
    g.fillRoundedRectangle (400, 80, 180, 290, 12.0f);
    g.setColour (juce::Colour (0x10ffffff)); // light border
    g.drawRoundedRectangle (400, 80, 180, 290, 12.0f, 1.5f);

    // Section title
    g.setColour (juce::Colour (0xff8fbcbb));
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.drawText ("MASTER OUTPUT", 415, 88, 150, 15, juce::Justification::centred);

    // Draw version footer using compile time HHmm
    g.setColour (juce::Colour (0x55eceff4)); // subtle transparent white
    g.setFont (juce::FontOptions (10.0f, juce::Font::plain));
    const char* compileTime = __TIME__; // e.g. "18:25:08"
    juce::String versionStr = "v1.0.";
    versionStr << compileTime[0] << compileTime[1] << compileTime[3] << compileTime[4];
    g.drawText (versionStr, 30, 375, 540, 20, juce::Justification::right);
}

void RamerSynthAudioProcessorEditor::resized()
{
    // Layout geometry
    int knobWidth = 110;
    int knobHeight = 90;
    int labelHeight = 20;

    // Card 1: Filter & Drive (Left column, centered at X = 105)
    int x1 = 105 - (knobWidth / 2);
    cutoffSlider.setBounds (x1, 110, knobWidth, knobHeight);
    cutoffLabel.setBounds (x1, 200, knobWidth, labelHeight);

    driveSlider.setBounds (x1, 235, knobWidth, knobHeight);
    driveLabel.setBounds (x1, 325, knobWidth, labelHeight);

    // Card 2: Amplifier Envelope (Middle column, centered at X = 295)
    int x2 = 295 - (knobWidth / 2);
    attackSlider.setBounds (x2, 110, knobWidth, knobHeight);
    attackLabel.setBounds (x2, 200, knobWidth, labelHeight);

    releaseSlider.setBounds (x2, 235, knobWidth, knobHeight);
    releaseLabel.setBounds (x2, 325, knobWidth, labelHeight);

    // Card 3: Master Volume (Right column, centered at X = 490)
    // We make the volume knob slightly larger to emphasize it as master
    int vKnobWidth = 130;
    int vKnobHeight = 110;
    int x3 = 490 - (vKnobWidth / 2);
    int y3 = 80 + (290 - vKnobHeight) / 2 - 10;
    volumeSlider.setBounds (x3, y3, vKnobWidth, vKnobHeight);
    volumeLabel.setBounds (x3, y3 + vKnobHeight, vKnobWidth, labelHeight);
}
