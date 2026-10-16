// PluginEditor.h
// A simple front panel: three columns (one per tap), four knobs each, plus Mix.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class Mpc3kDelayEditor : public juce::AudioProcessorEditor
{
public:
    explicit Mpc3kDelayEditor (Mpc3kDelayProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    // One knob with its name underneath, connected to a parameter.
    struct Knob
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<Attachment> attachment;
    };

    void setUpKnob (Knob& k, const juce::String& paramId, const juce::String& name);

    Mpc3kDelayProcessor& processor;

    static constexpr int knobsPerTap = 4;
    std::array<std::array<Knob, knobsPerTap>, Mpc3kDelay::numTaps> tapKnobs;
    std::array<juce::Label, Mpc3kDelay::numTaps> tapTitles;
    Knob mixKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mpc3kDelayEditor)
};
