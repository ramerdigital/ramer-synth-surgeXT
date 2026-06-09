#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "SurgeSynthesizer.h"

class RamerSynthAudioProcessor : public juce::AudioProcessor,
                                 public SurgeSynthesizer::PluginLayer
{
public:
    RamerSynthAudioProcessor();
    ~RamerSynthAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // SurgeSynthesizer::PluginLayer overrides
    void surgeParameterUpdated(const SurgeSynthesizer::ID &id, float value) override;
    void surgeMacroUpdated(long macroNum, float value) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    std::unique_ptr<SurgeSynthesizer> surge;
    
    // Cached parameter pointers
    std::atomic<float>* cutoffParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* driveParam = nullptr;
    std::atomic<float>* volumeParam = nullptr;

    int blockPos = 0;

    void configureRamerSynthPatch();
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RamerSynthAudioProcessor)
};
