#pragma once

#include <JuceHeader.h>

struct DelayParameters
{
    float mix = 0.35f;
    float timeMs = 420.0f;
    float feedback = 0.36f;
    float spreadMs = 18.0f;
    float lowCutHz = 90.0f;
    float highCutHz = 9000.0f;
    float modulationRateHz = 0.35f;
    float modulationDepthMs = 3.0f;
    float width = 1.0f;
    float ducking = 0.0f;
    bool pingPong = true;
    bool freeze = false;
};

class SGMDelayEngine
{
public:
    void prepare (double newSampleRate, int maxBlockSize, int numChannels);
    void reset();
    void process (juce::AudioBuffer<float>& buffer, const DelayParameters& parameters);

private:
    class DelayLine
    {
    public:
        void prepare (int newMaximumSamples);
        void reset();
        void push (float sample);
        float read (float delaySamples) const;

    private:
        juce::AudioBuffer<float> data;
        int writeIndex = 0;
    };

    double sampleRate = 44100.0;
    int blockSize = 512;
    float lfoPhase = 0.0f;
    float envelope = 0.0f;

    std::array<DelayLine, 2> delay;
    juce::dsp::StateVariableTPTFilter<float> lowCutL;
    juce::dsp::StateVariableTPTFilter<float> lowCutR;
    juce::dsp::StateVariableTPTFilter<float> highCutL;
    juce::dsp::StateVariableTPTFilter<float> highCutR;
};
