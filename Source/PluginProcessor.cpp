#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr auto mixId = "mix";
constexpr auto timeDivisionId = "timeDivision";
constexpr auto delayTimeMsId = "delayTimeMs";
constexpr auto feedbackId = "feedback";
constexpr auto spreadId = "spread";
constexpr auto lowCutId = "lowCut";
constexpr auto highCutId = "highCut";
constexpr auto modulationRateId = "modRate";
constexpr auto modulationDepthId = "modDepth";
constexpr auto widthId = "width";
constexpr auto duckingId = "ducking";
constexpr auto pingPongId = "pingPong";
constexpr auto freezeId = "freeze";

float value (const juce::AudioProcessorValueTreeState& state, const char* id)
{
    return state.getRawParameterValue (id)->load();
}

const juce::StringArray timeDivisionNames {
    "8/1", "4/1", "Msec", "1/2", "1/4", "1/8", "1/16", "1/8T", "1/8D"
};

constexpr auto millisecondsDivisionIndex = 2;

constexpr std::array<float, 9> timeDivisionBeats {
    32.0f, 16.0f, 0.0f, 2.0f, 1.0f, 0.5f, 0.25f, 1.0f / 3.0f, 0.75f
};
}

SGMDelayAudioProcessor::SGMDelayAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout SGMDelayAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto percent = [] (float value, int) { return juce::String (juce::roundToInt (value * 100.0f)) + "%"; };
    auto ms = [] (float value, int) { return juce::String (value, value < 100.0f ? 1 : 0) + " ms"; };
    auto hz = [] (float value, int) { return value >= 1000.0f ? juce::String (value / 1000.0f, 1) + " kHz" : juce::String (juce::roundToInt (value)) + " Hz"; };

    params.push_back (std::make_unique<juce::AudioParameterFloat> (mixId, "Mix", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.35f, juce::String(), juce::AudioProcessorParameter::genericParameter, percent));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (timeDivisionId, "Time", timeDivisionNames, millisecondsDivisionIndex));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (delayTimeMsId, "Msec", juce::NormalisableRange<float> (1.0f, 10000.0f, 1.0f, 0.35f), 300.0f, juce::String(), juce::AudioProcessorParameter::genericParameter, ms));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (feedbackId, "Feedback", juce::NormalisableRange<float> (0.0f, 0.94f, 0.001f), 0.36f, juce::String(), juce::AudioProcessorParameter::genericParameter, percent));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (spreadId, "Spread", juce::NormalisableRange<float> (0.0f, 80.0f, 0.1f), 18.0f, juce::String(), juce::AudioProcessorParameter::genericParameter, ms));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (lowCutId, "Low Cut", juce::NormalisableRange<float> (20.0f, 2000.0f, 1.0f, 0.35f), 90.0f, juce::String(), juce::AudioProcessorParameter::genericParameter, hz));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (highCutId, "High Cut", juce::NormalisableRange<float> (1000.0f, 20000.0f, 1.0f, 0.45f), 9000.0f, juce::String(), juce::AudioProcessorParameter::genericParameter, hz));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (modulationRateId, "Mod Rate", juce::NormalisableRange<float> (0.01f, 8.0f, 0.001f, 0.35f), 0.35f, juce::String(), juce::AudioProcessorParameter::genericParameter, hz));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (modulationDepthId, "Mod Depth", juce::NormalisableRange<float> (0.0f, 35.0f, 0.1f), 3.0f, juce::String(), juce::AudioProcessorParameter::genericParameter, ms));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (widthId, "Width", juce::NormalisableRange<float> (0.0f, 1.8f, 0.001f), 1.0f, juce::String(), juce::AudioProcessorParameter::genericParameter, percent));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (duckingId, "Ducking", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f, juce::String(), juce::AudioProcessorParameter::genericParameter, percent));
    params.push_back (std::make_unique<juce::AudioParameterBool> (pingPongId, "Ping Pong", true));
    params.push_back (std::make_unique<juce::AudioParameterBool> (freezeId, "Freeze", false));

    return { params.begin(), params.end() };
}

void SGMDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    lastTransportSample.reset();
    wasPlaying = false;
}

void SGMDelayAudioProcessor::releaseResources()
{
}

bool SGMDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainIn = layouts.getMainInputChannelSet();
    const auto& mainOut = layouts.getMainOutputChannelSet();
    return mainIn == mainOut && (mainOut == juce::AudioChannelSet::mono() || mainOut == juce::AudioChannelSet::stereo());
}

void SGMDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    if (shouldResetDelayForTransport (buffer.getNumSamples()))
        engine.reset();

    engine.process (buffer, readParameters());
}

bool SGMDelayAudioProcessor::shouldResetDelayForTransport (int numSamples)
{
    if (auto* hostPlayHead = getPlayHead())
    {
        if (auto position = hostPlayHead->getPosition())
        {
            const auto isPlaying = position->getIsPlaying();

            if (auto timeInSamples = position->getTimeInSamples())
            {
                bool shouldReset = false;

                if (isPlaying && ! wasPlaying)
                    shouldReset = true;

                if (isPlaying && lastTransportSample.has_value())
                {
                    const auto expected = *lastTransportSample + static_cast<juce::int64> (numSamples);
                    const auto tolerance = juce::jmax<juce::int64> (static_cast<juce::int64> (numSamples * 4),
                                                                    static_cast<juce::int64> (getSampleRate() * 0.05));

                    if (std::abs (*timeInSamples - expected) > tolerance)
                        shouldReset = true;
                }

                lastTransportSample = *timeInSamples;
                wasPlaying = isPlaying;
                return shouldReset;
            }

            if (! isPlaying)
                lastTransportSample.reset();

            wasPlaying = isPlaying;
        }
    }

    return false;
}

DelayParameters SGMDelayAudioProcessor::readParameters() const
{
    DelayParameters p;
    const auto divisionIndex = juce::jlimit (0, static_cast<int> (timeDivisionBeats.size()) - 1, static_cast<int> (std::round (value (apvts, timeDivisionId))));
    const auto quarterNoteMs = 60000.0 / getCurrentBpm();

    p.mix = value (apvts, mixId);
    p.timeMs = divisionIndex == millisecondsDivisionIndex
        ? value (apvts, delayTimeMsId)
        : static_cast<float> (quarterNoteMs * static_cast<double> (timeDivisionBeats[static_cast<size_t> (divisionIndex)]));
    p.feedback = value (apvts, feedbackId);
    p.spreadMs = value (apvts, spreadId);
    p.lowCutHz = value (apvts, lowCutId);
    p.highCutHz = value (apvts, highCutId);
    p.modulationRateHz = value (apvts, modulationRateId);
    p.modulationDepthMs = value (apvts, modulationDepthId);
    p.width = value (apvts, widthId);
    p.ducking = value (apvts, duckingId);
    p.pingPong = value (apvts, pingPongId) > 0.5f;
    p.freeze = value (apvts, freezeId) > 0.5f;
    return p;
}

double SGMDelayAudioProcessor::getCurrentBpm() const
{
    if (auto* hostPlayHead = getPlayHead())
        if (auto position = hostPlayHead->getPosition())
            if (auto bpm = position->getBpm())
                return juce::jlimit (20.0, 300.0, *bpm);

    return 120.0;
}

void SGMDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void SGMDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* SGMDelayAudioProcessor::createEditor()
{
    return new SGMDelayAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SGMDelayAudioProcessor();
}
