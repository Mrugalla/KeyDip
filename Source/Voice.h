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
        sampleRate(1.),
        freq(1.f),
        q(1.f),
        gain(1.f),
        atk(1.f),
        rls(1.f),
		noteNumber(-1)
    {}

    void resetFilter() noexcept
    {
        filter.reset();
	}

    void updateFilter(float _q, float _gain) noexcept
    {
		q = _q;
        gain = _gain;
        updateFilter();
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
        updateFilter();
    }

	void operator()(juce::dsp::AudioBlock<float>& block, juce::dsp::AudioBlock<float>& blockFilter,
        float* envBuffer) noexcept
	{
        juce::dsp::ProcessContextNonReplacing<float> context(block, blockFilter);
        filter.process(context);
        const auto numChannels = block.getNumChannels();
		const auto numSamples = block.getNumSamples();
		for (auto s = 0; s < numSamples; ++s)
		{
			auto env = envelope();
            envBuffer[s] = env * (1.f - gain);
		}
		for (auto ch = 0; ch < numChannels; ++ch)
        {
			const auto smplsFilter = blockFilter.getChannelPointer(ch);
            auto smpls = block.getChannelPointer(ch);
            for (auto s = 0; s < numSamples; ++s)
                smpls[s] -= smplsFilter[s] * envBuffer[s];
        }
	}

    void processNoteOn(MTSClient* mtsesp, int _noteNumber) noexcept
    {
		noteNumber = _noteNumber;
		const auto note = static_cast<char>(noteNumber);
        freq = static_cast<float>(MTS_NoteToFrequency(mtsesp, note, -1));
        updateFilter();
        filter.reset();
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
    double sampleRate;
    float freq, q, gain, atk, rls;
    int noteNumber;

    void updateFilter()
    {
        auto& duplicator = filter.get<0>();
        *duplicator.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass
        (
            sampleRate, freq, q
        );
    }
};
