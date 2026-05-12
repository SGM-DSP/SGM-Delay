#include "DelayEngine.h"

void SGMDelayEngine::DelayLine::prepare (int newMaximumSamples)
{
    data.setSize (1, juce::jmax (4, newMaximumSamples + 4));
    reset();
}

void SGMDelayEngine::DelayLine::reset()
{
    data.clear();
    writeIndex = 0;
}

void SGMDelayEngine::DelayLine::push (float sample)
{
    data.setSample (0, writeIndex, sample);
    writeIndex = (writeIndex + 1) % data.getNumSamples();
}

float SGMDelayEngine::DelayLine::read (float delaySamples) const
{
    const auto size = data.getNumSamples();
    const auto clampedDelay = juce::jlimit (1.0f, static_cast<float> (size - 3), delaySamples);
    auto readPosition = static_cast<float> (writeIndex) - clampedDelay;

    while (readPosition < 0.0f)
        readPosition += static_cast<float> (size);

    const auto index0 = static_cast<int> (readPosition) % size;
    const auto index1 = (index0 + 1) % size;
    const auto fraction = readPosition - std::floor (readPosition);

    return juce::jmap (fraction, data.getSample (0, index0), data.getSample (0, index1));
}

void SGMDelayEngine::prepare (double newSampleRate, int maxBlockSize, int)
{
    sampleRate = newSampleRate;
    blockSize = maxBlockSize;

    const auto maximumDelaySamples = static_cast<int> (sampleRate * 96.0);
    for (auto& line : delay)
        line.prepare (maximumDelaySamples);

    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (blockSize), 1 };
    lowCutL.prepare (spec);
    lowCutR.prepare (spec);
    highCutL.prepare (spec);
    highCutR.prepare (spec);

    lowCutL.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    lowCutR.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    highCutL.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    highCutR.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    reset();
}

void SGMDelayEngine::reset()
{
    for (auto& line : delay)
        line.reset();

    lfoPhase = 0.0f;
    envelope = 0.0f;
    lowCutL.reset();
    lowCutR.reset();
    highCutL.reset();
    highCutR.reset();
}

void SGMDelayEngine::process (juce::AudioBuffer<float>& buffer, const DelayParameters& parameters)
{
    const auto numSamples = buffer.getNumSamples();
    const auto hasRight = buffer.getNumChannels() > 1;

    lowCutL.setCutoffFrequency (parameters.lowCutHz);
    lowCutR.setCutoffFrequency (parameters.lowCutHz);
    highCutL.setCutoffFrequency (parameters.highCutHz);
    highCutR.setCutoffFrequency (parameters.highCutHz);

    const auto baseDelaySamples = parameters.timeMs * 0.001f * static_cast<float> (sampleRate);
    const auto spreadSamples = parameters.spreadMs * 0.001f * static_cast<float> (sampleRate);
    const auto modDepthSamples = parameters.modulationDepthMs * 0.001f * static_cast<float> (sampleRate);
    const auto phaseDelta = juce::MathConstants<float>::twoPi * parameters.modulationRateHz / static_cast<float> (sampleRate);
    const auto feedback = parameters.freeze ? 1.0f : juce::jlimit (0.0f, 0.94f, parameters.feedback);
    const auto mix = juce::jlimit (0.0f, 1.0f, parameters.mix);
    const auto width = juce::jlimit (0.0f, 1.8f, parameters.width);
    const auto duckAmount = juce::jlimit (0.0f, 1.0f, parameters.ducking);

    for (int i = 0; i < numSamples; ++i)
    {
        const auto dryL = buffer.getSample (0, i);
        const auto dryR = hasRight ? buffer.getSample (1, i) : dryL;

        const auto input = 0.5f * (std::abs (dryL) + std::abs (dryR));
        envelope += (input - envelope) * (input > envelope ? 0.004f : 0.0008f);

        const auto lfo = std::sin (lfoPhase);
        const auto delayL = baseDelaySamples + lfo * modDepthSamples;
        const auto delayR = baseDelaySamples + spreadSamples - lfo * modDepthSamples;

        auto wetL = delay[0].read (delayL);
        auto wetR = hasRight ? delay[1].read (delayR) : wetL;

        wetL = highCutL.processSample (0, lowCutL.processSample (0, wetL));
        wetR = highCutR.processSample (0, lowCutR.processSample (0, wetR));

        const auto mid = 0.5f * (wetL + wetR);
        const auto side = 0.5f * (wetL - wetR) * width;
        wetL = mid + side;
        wetR = mid - side;

        const auto feedbackSourceL = parameters.pingPong ? wetR : wetL;
        const auto feedbackSourceR = parameters.pingPong ? wetL : wetR;
        const auto inputGain = parameters.freeze ? 0.0f : 1.0f;

        delay[0].push (dryL * inputGain + feedbackSourceL * feedback);
        delay[1].push (dryR * inputGain + feedbackSourceR * feedback);

        const auto duckGain = 1.0f - duckAmount * juce::jlimit (0.0f, 0.85f, envelope * 3.0f);
        buffer.setSample (0, i, dryL * (1.0f - mix) + wetL * mix * duckGain);

        if (hasRight)
            buffer.setSample (1, i, dryR * (1.0f - mix) + wetR * mix * duckGain);

        lfoPhase += phaseDelta;
        if (lfoPhase >= juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;
    }
}
