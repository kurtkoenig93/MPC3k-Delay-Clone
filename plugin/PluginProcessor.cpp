// PluginProcessor.cpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

Mpc3kDelayProcessor::Mpc3kDelayProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      params (*this, nullptr, "PARAMS", createLayout())
{
    for (int i = 0; i < Mpc3kDelay::numTaps; ++i)
    {
        auto& t   = tapParams[(size_t) i];
        t.volume   = params.getRawParameterValue (tapId (i, "vol"));
        t.pan      = params.getRawParameterValue (tapId (i, "pan"));
        t.time     = params.getRawParameterValue (tapId (i, "time"));
        t.feedback = params.getRawParameterValue (tapId (i, "fdbk"));
    }
    mixParam = params.getRawParameterValue ("mix");
}

juce::AudioProcessorValueTreeState::ParameterLayout Mpc3kDelayProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    for (int i = 0; i < Mpc3kDelay::numTaps; ++i)
    {
        const String tap = "Tap " + String (i + 1) + " ";

        // Same ranges as the MPC3000 screen. Tap 1 starts audible, taps 2 and 3 start silent.
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { tapId (i, "vol"), 1 }, tap + "Volume",
                                                           NormalisableRange<float> (0.0f, 100.0f, 1.0f),
                                                           i == 0 ? 100.0f : 0.0f));

        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { tapId (i, "pan"), 1 }, tap + "Pan",
                                                           NormalisableRange<float> (-50.0f, 50.0f, 1.0f), 0.0f,
                                                           AudioParameterFloatAttributes().withStringFromValueFunction (
                                                               [] (float v, int)
                                                               {
                                                                   const int p = (int) v;
                                                                   if (p == 0) return String ("C");
                                                                   return p < 0 ? String (-p) + "L" : String (p) + "R";
                                                               })));

        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { tapId (i, "time"), 1 }, tap + "Time",
                                                           NormalisableRange<float> (Mpc3kDelay::minTimeMs, Mpc3kDelay::maxTimeMs, 1.0f),
                                                           Mpc3kDelay::maxTimeMs,
                                                           AudioParameterFloatAttributes().withLabel ("ms")));

        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { tapId (i, "fdbk"), 1 }, tap + "Feedback",
                                                           NormalisableRange<float> (0.0f, 100.0f, 1.0f), 0.0f,
                                                           AudioParameterFloatAttributes().withLabel ("%")));
    }

    // Extra (not on the original): dry/wet. 100% = wet only, for use on a send.
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mix",
                                                       NormalisableRange<float> (0.0f, 100.0f, 1.0f), 100.0f,
                                                       AudioParameterFloatAttributes().withLabel ("%")));
    return layout;
}

bool Mpc3kDelayProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Stereo out; mono or stereo in.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    const auto in = layouts.getMainInputChannelSet();
    return in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo();
}

void Mpc3kDelayProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    delay.prepare (sampleRate);
    monoIn.setSize (1, samplesPerBlock);
    wet.setSize (2, samplesPerBlock);
}

void Mpc3kDelayProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numIn      = getTotalNumInputChannels();

    // Hosts can occasionally send a bigger block than promised.
    if (numSamples > monoIn.getNumSamples())
    {
        monoIn.setSize (1, numSamples, false, false, true);
        wet.setSize (2, numSamples, false, false, true);
    }

    // 1. Copy the latest knob values into the core.
    for (int i = 0; i < Mpc3kDelay::numTaps; ++i)
    {
        const auto& t = tapParams[(size_t) i];
        delay.setTap (i, t.volume->load(), t.pan->load(), t.time->load(), t.feedback->load());
    }

    // 2. Sum the input to mono (the core, like the MPC3000 delay, takes one input).
    auto* mono = monoIn.getWritePointer (0);
    if (numIn >= 2)
    {
        const auto* l = buffer.getReadPointer (0);
        const auto* r = buffer.getReadPointer (1);
        for (int n = 0; n < numSamples; ++n)
            mono[n] = 0.5f * (l[n] + r[n]);
    }
    else
    {
        juce::FloatVectorOperations::copy (mono, buffer.getReadPointer (0), numSamples);
    }

    // 3. Run the delay.
    delay.process (mono, wet.getWritePointer (0), wet.getWritePointer (1), numSamples);

    // 4. Blend dry and wet into the output.
    // With a mono input, the dry signal (kept in the mono buffer) goes to both sides.
    const float wetAmt = mixParam->load() / 100.0f;
    const float dryAmt = 1.0f - wetAmt;

    for (int ch = 0; ch < 2; ++ch)
    {
        const auto* dry = numIn >= 2 ? buffer.getReadPointer (ch) : monoIn.getReadPointer (0);
        auto* out       = buffer.getWritePointer (ch);
        const auto* w   = wet.getReadPointer (ch);

        for (int n = 0; n < numSamples; ++n)
            out[n] = dry[n] * dryAmt + w[n] * wetAmt;
    }
}

juce::AudioProcessorEditor* Mpc3kDelayProcessor::createEditor()
{
    return new Mpc3kDelayEditor (*this);
}

void Mpc3kDelayProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = params.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void Mpc3kDelayProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (params.state.getType()))
            params.replaceState (juce::ValueTree::fromXml (*xml));
}

// JUCE calls this to create the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Mpc3kDelayProcessor();
}
