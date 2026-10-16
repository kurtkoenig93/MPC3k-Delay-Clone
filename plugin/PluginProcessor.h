// PluginProcessor.h
// JUCE wrapper around the MPC3000 delay core.
// Holds the knobs (parameters) and feeds them to the core every block.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Mpc3kDelay.h"

class Mpc3kDelayProcessor : public juce::AudioProcessor
{
public:
    Mpc3kDelayProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock; // keeps the double-precision version visible (silences a warning)

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 1.5; }

    int getNumPrograms() override    { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // All the knobs live here. The editor connects to them by ID.
    juce::AudioProcessorValueTreeState params;

    // Parameter IDs, e.g. tapId (0, "time") -> "tap1_time"
    static juce::String tapId (int tap, const char* name)
    {
        return "tap" + juce::String (tap + 1) + "_" + name;
    }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    Mpc3kDelay delay;

    // Fast pointers to the current knob values (read every block).
    struct TapParams
    {
        std::atomic<float>* volume   = nullptr;
        std::atomic<float>* pan      = nullptr;
        std::atomic<float>* time     = nullptr;
        std::atomic<float>* feedback = nullptr;
    };
    std::array<TapParams, Mpc3kDelay::numTaps> tapParams;
    std::atomic<float>* mixParam = nullptr;

    // Scratch space, sized in prepareToPlay so nothing is allocated while playing.
    juce::AudioBuffer<float> monoIn, wet;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mpc3kDelayProcessor)
};
