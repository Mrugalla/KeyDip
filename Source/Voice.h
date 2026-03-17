#pragma once
#include "libMTSClient.h"
#include "EnvelopeGenerator.h"
#include <JuceHeader.h>

class Voice
{
    using FilterChain = juce::dsp::ProcessorChain<
        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                       juce::dsp::IIR::Coefficients<float>>>;
public:
    Voice() :
        filter(),
        envelope(),
        envBuffer(),
        sampleRate(1.),
        freq(1.f),
        q(1.f),
        gain(1.f),
        atk(1.f),
        rls(1.f),
		noteNumber(-1)
    {}

    void updateFilter(float _q, float _gain) noexcept
    {
		q = _q;
        gain = _gain;
    }

    void updateEnvelope(float _atk, float _rls) noexcept
    {
        atk = _atk;
        rls = _rls;
        envelope.setAttack(atk);
        envelope.setRelease(rls);
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
		sampleRate = spec.sampleRate;
        filter.prepare(spec);
		envelope.prepare(static_cast<float>(sampleRate));
        processNoteOff();
        envBuffer.resize(spec.maximumBlockSize);
    }

	void operator()(juce::dsp::ProcessContextReplacing<float>& context) noexcept
	{
		const auto block = context.getOutputBlock();
		const auto numSamples = block.getNumSamples();
		for (auto s = 0; s < numSamples; ++s)
		{
			auto env = envelope();
			envBuffer[s] = env;
		}
		const auto gainMod = 1.f + envBuffer[0] * (gain - 1.f);
        auto& duplicator = filter.get<0>();
        *duplicator.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter
        (
            sampleRate, freq, q, gainMod
        );
        filter.process(context);
	}

    void processNoteOn(MTSClient* mtsesp, int _noteNumber) noexcept
    {
		noteNumber = _noteNumber;
		const auto note = static_cast<char>(noteNumber);
        freq = static_cast<float>(MTS_NoteToFrequency(mtsesp, note, -1));
        envelope.trigger();
    }

    void processNoteOff(int _noteNumber) noexcept
    {
        if (noteNumber != _noteNumber)
            return;
        envelope.release();
    }

    void processNoteOff() noexcept
    {
        noteNumber = -1;
        envelope.release();
    }
private:
    FilterChain filter;
    EnvelopeGenerator envelope;
    std::vector<float> envBuffer;
    double sampleRate;
    float freq, q, gain, atk, rls;
    int noteNumber;
};
