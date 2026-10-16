// PluginEditor.cpp

#include "PluginEditor.h"

namespace
{
    // Layout sizes, in pixels.
    constexpr int margin  = 16;
    constexpr int titleH  = 40;
    constexpr int knobW   = 84;
    constexpr int knobH   = 96;
    constexpr int labelH  = 18;
    constexpr int colGap  = 24;

    const char* knobNames[] = { "Volume", "Pan", "Time", "Feedback" };
    const char* knobIds[]   = { "vol", "pan", "time", "fdbk" };
}

Mpc3kDelayEditor::Mpc3kDelayEditor (Mpc3kDelayProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    for (int t = 0; t < Mpc3kDelay::numTaps; ++t)
    {
        auto& title = tapTitles[(size_t) t];
        title.setText ("Tap " + juce::String (t + 1), juce::dontSendNotification);
        title.setJustificationType (juce::Justification::centred);
        title.setFont (juce::FontOptions (16.0f, juce::Font::bold));
        addAndMakeVisible (title);

        for (int k = 0; k < knobsPerTap; ++k)
            setUpKnob (tapKnobs[(size_t) t][(size_t) k], Mpc3kDelayProcessor::tapId (t, knobIds[k]), knobNames[k]);
    }

    setUpKnob (mixKnob, "mix", "Mix");

    // Taps side by side, each a 2 x 2 grid of knobs; Mix on the right.
    const int colW = 2 * knobW;
    setSize (margin * 2 + Mpc3kDelay::numTaps * (colW + colGap) + knobW,
             margin * 2 + titleH + 2 * (knobH + labelH));
}

void Mpc3kDelayEditor::setUpKnob (Knob& k, const juce::String& paramId, const juce::String& name)
{
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, knobW - 8, 20);
    addAndMakeVisible (k.slider);

    k.label.setText (name, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (k.label);

    k.attachment = std::make_unique<Attachment> (processor.params, paramId, k.slider);
}

void Mpc3kDelayEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e1f22));

    // A faint panel behind each tap's column.
    g.setColour (juce::Colour (0xff2a2c30));
    for (const auto& title : tapTitles)
        g.fillRoundedRectangle (title.getBounds().withHeight (getHeight() - 2 * margin).toFloat().expanded (6.0f, 0.0f), 8.0f);
}

void Mpc3kDelayEditor::resized()
{
    const int colW = 2 * knobW;

    auto place = [] (Knob& k, int x, int y)
    {
        k.slider.setBounds (x, y, knobW, knobH);
        k.label.setBounds (x, y + knobH, knobW, labelH);
    };

    for (int t = 0; t < Mpc3kDelay::numTaps; ++t)
    {
        const int x = margin + t * (colW + colGap);
        tapTitles[(size_t) t].setBounds (x, margin, colW, titleH);

        for (int k = 0; k < knobsPerTap; ++k)
        {
            const int kx = x + (k % 2) * knobW;
            const int ky = margin + titleH + (k / 2) * (knobH + labelH);
            place (tapKnobs[(size_t) t][(size_t) k], kx, ky);
        }
    }

    const int mixX = margin + Mpc3kDelay::numTaps * (colW + colGap);
    place (mixKnob, mixX, margin + titleH);
}
