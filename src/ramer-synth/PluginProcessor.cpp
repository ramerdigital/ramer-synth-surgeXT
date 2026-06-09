#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "sst/plugininfra/cpufeatures.h"
#include "FilterConfiguration.h"

// Define constants for classic oscillator parameters
const int co_shape = 0;
const int co_unison_detune = 5;
const int co_unison_voices = 6;

RamerSynthAudioProcessor::RamerSynthAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    try
    {
        // Skip scanning wavetable files from disk to make the plugin self-contained
        surge = std::make_unique<SurgeSynthesizer>(this, SurgeStorage::skipPatchLoadDataPathSentinel);
    }
    catch (const std::exception& e)
    {
        surge.reset();
        return;
    }

    if (surge)
    {
        surge->audio_processing_active = true;
        configureRamerSynthPatch();
    }

    // Cache parameter pointers
    cutoffParam = apvts.getRawParameterValue ("cutoff");
    attackParam = apvts.getRawParameterValue ("attack");
    releaseParam = apvts.getRawParameterValue ("release");
    driveParam = apvts.getRawParameterValue ("drive");
    volumeParam = apvts.getRawParameterValue ("volume");
}

RamerSynthAudioProcessor::~RamerSynthAudioProcessor()
{
}

void RamerSynthAudioProcessor::configureRamerSynthPatch()
{
    if (!surge) return;

    auto& patch = surge->storage.getPatch();

    // Set Scene A active and Scene Mode to Single
    patch.scene_active.val.i = 0; // Scene A
    patch.scenemode.val.i = 0;    // Single

    // Configure Osc 1 on Scene A
    auto& osc1 = patch.scene[0].osc[0];
    osc1.type.val.i = 0; // ot_classic (Classic Oscillator)
    osc1.p[co_shape].val.f = 0.0f; // Sawtooth wave
    osc1.p[co_unison_voices].val.i = 6; // Fat 6-voice unison
    osc1.p[co_unison_detune].val.f = 0.15f; // Unison detune amount

    // Configure Mixer on Scene A (turn off other oscillators and noise)
    patch.scene[0].level_o1.val.f = 1.0f; // Osc 1 volume full
    patch.scene[0].level_o2.val.f = 0.0f; // Osc 2 off
    patch.scene[0].level_o3.val.f = 0.0f; // Osc 3 off
    patch.scene[0].level_noise.val.f = 0.0f; // Noise off
    patch.scene[0].level_ring_12.val.f = 0.0f; // Ring modulation off

    // Configure Filter 1 on Scene A
    auto& filter1 = patch.scene[0].filterunit[0];
    filter1.type.val.i = (int)sst::filters::FilterType::fut_vintageladder; // Vintage Ladder Lowpass Filter
    filter1.cutoff.val.f = 0.5f; // Cutoff
    filter1.resonance.val.f = 0.1f; // Resonance (10%)
    filter1.envmod.val.f = 0.4f; // FEG Mod Amount

    // Configure Envelopes on Scene A (index 0 for AEG, 1 for FEG)
    // AEG (Amplitude Envelope)
    auto& aeg = patch.scene[0].adsr[0];
    aeg.a.val.f = 0.1f; // Soft attack (100ms)
    aeg.d.val.f = 1.0f; // Decay time
    aeg.s.val.f = 0.8f; // Sustain level (80%)
    aeg.r.val.f = 0.3f; // Release time

    // FEG (Filter Envelope)
    auto& feg = patch.scene[0].adsr[1];
    feg.a.val.f = 0.15f; // Attack (150ms)
    feg.d.val.f = 1.2f; // Decay time
    feg.s.val.f = 0.4f; // Sustain level (40%)
    feg.r.val.f = 0.3f; // Release time

    // Configure Waveshaper (Saturation) on Scene A
    auto& ws = patch.scene[0].wsunit;
    ws.type.val.i = (int)sst::waveshapers::WaveshaperType::wst_soft; // Soft saturation
    ws.drive.val.f = 0.0f; // Drive (0dB default)

    // Apply patch changes to the engine
    patch.update_controls(true);
}

juce::AudioProcessorValueTreeState::ParameterLayout RamerSynthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("cutoff", 1), "Cutoff", 0.0f, 1.0f, 0.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("attack", 1), "Attack", 0.0f, 1.0f, 0.1f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("release", 1), "Release", 0.0f, 1.0f, 0.3f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("drive", 1), "Drive", 0.0f, 1.0f, 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("volume", 1), "Volume", 0.0f, 1.0f, 0.8f));

    return { params.begin(), params.end() };
}

const juce::String RamerSynthAudioProcessor::getName() const
{
    return "ramer-synth";
}

bool RamerSynthAudioProcessor::acceptsMidi() const
{
    return true;
}

bool RamerSynthAudioProcessor::producesMidi() const
{
    return false;
}

bool RamerSynthAudioProcessor::isMidiEffect() const
{
    return false;
}

double RamerSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RamerSynthAudioProcessor::getNumPrograms()
{
    return 1;
}

int RamerSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RamerSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RamerSynthAudioProcessor::getProgramName (int index)
{
    return "Default";
}

void RamerSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void RamerSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    if (surge)
    {
        surge->setSamplerate(sampleRate);
    }
}

void RamerSynthAudioProcessor::releaseResources()
{
}

void RamerSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto fpuguard = sst::plugininfra::cpufeatures::FPUStateGuard();

    if (!surge)
    {
        buffer.clear();
        return;
    }

    // Clear unused output channels
    for (int i = 2; i < buffer.getNumChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    int numSamples = buffer.getNumSamples();
    auto* outL = buffer.getWritePointer(0);
    auto* outR = buffer.getWritePointer(1);

    auto midiIt = midiMessages.findNextSamplePosition(0);
    int nextMidi = -1;
    if (midiIt != midiMessages.cend())
        nextMidi = (*midiIt).samplePosition;

    for (int i = 0; i < numSamples; ++i)
    {
        // Handle incoming MIDI messages at this sample index
        while (i == nextMidi)
        {
            const auto& m = (*midiIt).getMessage();
            const int ch = m.getChannel() - 1;

            if (m.isNoteOn())
            {
                if (m.getVelocity() != 0)
                    surge->playNote(ch, m.getNoteNumber(), m.getVelocity(), 0, -1);
                else
                    surge->releaseNote(ch, m.getNoteNumber(), m.getVelocity(), -1);
            }
            else if (m.isNoteOff())
            {
                surge->releaseNote(ch, m.getNoteNumber(), m.getVelocity());
            }
            else if (m.isChannelPressure())
            {
                surge->channelAftertouch(ch, m.getChannelPressureValue());
            }
            else if (m.isAftertouch())
            {
                surge->polyAftertouch(ch, m.getNoteNumber(), m.getAfterTouchValue());
            }
            else if (m.isPitchWheel())
            {
                surge->pitchBend(ch, m.getPitchWheelValue() - 8192);
            }
            else if (m.isController())
            {
                surge->channelController(ch, m.getControllerNumber(), m.getControllerValue());
            }

            midiIt++;
            if (midiIt == midiMessages.cend())
                nextMidi = -1;
            else
                nextMidi = (*midiIt).samplePosition;
        }

        // Process a block of 32 samples in the engine when blockPos is 0
        if (blockPos == 0)
        {
            // Sync current parameter values to the engine
            auto& patch = surge->storage.getPatch();
            surge->setParameter01(surge->idForParameter(&patch.scene[0].filterunit[0].cutoff), cutoffParam->load());
            surge->setParameter01(surge->idForParameter(&patch.scene[0].filterunit[0].resonance), 0.10f); // Fixed to default
            surge->setParameter01(surge->idForParameter(&patch.scene[0].osc[0].p[co_unison_detune]), 0.15f); // Fixed to default
            surge->setParameter01(surge->idForParameter(&patch.scene[0].adsr[0].a), attackParam->load());
            surge->setParameter01(surge->idForParameter(&patch.scene[0].adsr[0].r), releaseParam->load());
            surge->setParameter01(surge->idForParameter(&patch.scene[0].wsunit.drive), driveParam->load());
            surge->setParameter01(surge->idForParameter(&patch.volume), volumeParam->load());
            patch.update_controls(false);

            surge->process();
        }

        // Fetch synthesized audio samples from the engine output buffers
        outL[i] = surge->output[0][blockPos];
        outR[i] = surge->output[1][blockPos];

        blockPos = (blockPos + 1) % BLOCK_SIZE;
    }
}

bool RamerSynthAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* RamerSynthAudioProcessor::createEditor()
{
    return new RamerSynthAudioProcessorEditor (*this);
}

void RamerSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void RamerSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
            if (surge)
            {
                configureRamerSynthPatch();
            }
        }
    }
}

// SurgeSynthesizer::PluginLayer callbacks (ignored as parameters are set from host to engine)
void RamerSynthAudioProcessor::surgeParameterUpdated(const SurgeSynthesizer::ID &id, float value)
{
}

void RamerSynthAudioProcessor::surgeMacroUpdated(long macroNum, float value)
{
}

// juce::AudioProcessor creation helper
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RamerSynthAudioProcessor();
}
