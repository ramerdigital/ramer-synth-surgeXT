#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "PluginProcessor.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle,
                           juce::Slider& slider) override;
};

class RamerSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    RamerSynthAudioProcessorEditor (RamerSynthAudioProcessor&);
    ~RamerSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    RamerSynthAudioProcessor& audioProcessor;

    // UI Controls
    juce::Slider cutoffSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Slider driveSlider;
    juce::Slider volumeSlider;

    // Labels
    juce::Label cutoffLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label driveLabel;
    juce::Label volumeLabel;

    // Slider Attachments to bind controls to APVTS parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;

    CustomLookAndFeel customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RamerSynthAudioProcessorEditor)
};
