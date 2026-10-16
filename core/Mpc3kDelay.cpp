// Mpc3kDelay.cpp

#include "Mpc3kDelay.h"

#include <algorithm>
#include <cmath>

void Mpc3kDelay::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;

    // Room for the longest delay plus a couple of samples for in-between reads.
    const auto size = (int) std::ceil (maxTimeMs * 0.001 * sampleRate) + 2;
    memory.assign ((size_t) size, 0.0f);
    writePos = 0;
}

void Mpc3kDelay::reset()
{
    std::fill (memory.begin(), memory.end(), 0.0f);
    writePos = 0;
}

void Mpc3kDelay::setTap (int index, float volume, float pan, float timeMs, float feedback)
{
    if (index < 0 || index >= numTaps)
        return;

    auto& t = taps[(size_t) index];

    const float gain = std::clamp (volume, 0.0f, 100.0f) / 100.0f;

    // Pan: -50..+50 mapped to 0..1, equal-power law.
    // NOTE: the MPC3000's real pan law is unknown. Check against reference recordings.
    const float p     = (std::clamp (pan, -50.0f, 50.0f) + 50.0f) / 100.0f;
    const float angle = p * 1.5707963f; // 0 to pi/2
    t.gainL = gain * std::cos (angle);
    t.gainR = gain * std::sin (angle);

    // The MPC3000 sets time in whole milliseconds.
    const float ms = std::round (std::clamp (timeMs, minTimeMs, maxTimeMs));
    t.delaySamples = (float) (ms * 0.001 * sampleRate);

    t.feedback = std::clamp (feedback, 0.0f, 100.0f) / 100.0f;
}

float Mpc3kDelay::readTap (float delaySamples) const
{
    // Read the memory "delaySamples" behind the write point.
    // Linear blend between the two nearest samples, since the delay
    // in samples is rarely a whole number.
    const int size = (int) memory.size();
    float readPos = (float) writePos - delaySamples;
    while (readPos < 0.0f)
        readPos += (float) size;

    const int   i0   = (int) readPos;
    const int   i1   = (i0 + 1) % size;
    const float frac = readPos - (float) i0;

    return memory[(size_t) i0] + frac * (memory[(size_t) i1] - memory[(size_t) i0]);
}

void Mpc3kDelay::process (const float* in, float* outL, float* outR, int numSamples)
{
    const int size = (int) memory.size();

    for (int n = 0; n < numSamples; ++n)
    {
        float left = 0.0f, right = 0.0f, feedbackSum = 0.0f;

        for (const auto& t : taps)
        {
            const float tap = readTap (t.delaySamples);

            // To the stereo mix, through volume and pan.
            left  += tap * t.gainL;
            right += tap * t.gainR;

            // Back into the shared memory, through feedback only (ignores volume).
            feedbackSum += tap * t.feedback;
        }

        float toMemory = in[n] + feedbackSum;

        // Safety net only, so a runaway can't produce infinite values and crash a host.
        // How the real unit clips is unknown; that belongs in the character layer later.
        toMemory = std::clamp (toMemory, -8.0f, 8.0f);

        memory[(size_t) writePos] = toMemory;
        writePos = (writePos + 1) % size;

        outL[n] = left;
        outR[n] = right;
    }
}
