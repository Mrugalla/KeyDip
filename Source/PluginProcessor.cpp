#include "PluginProcessor.h"

KeyDipAudioProcessor::KeyDipAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	 : AudioProcessor (BusesProperties()
					 #if ! JucePlugin_IsMidiEffect
					  #if ! JucePlugin_IsSynth
					   .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
					  #endif
					   .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
					 #endif
					   ),
	mtsesp(MTS_RegisterClient()),
	apvts(*this, nullptr, "PARAMETERS", createParameterLayout()),
	voices(),
	q(-1.f),
	gain(-40.f),
	atk(-1.f),
	rls(-1.f),
	activeFilterIndex(0)
#endif
{
}

juce::AudioProcessorValueTreeState::ParameterLayout KeyDipAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	layout.add (std::make_unique<juce::AudioParameterFloat>
		(
		"qFactor", "Q Factor",
		juce::NormalisableRange<float> (1.f, 30.0f, 0.1f, 1.0f),
			25.0f
		));

	layout.add (std::make_unique<juce::AudioParameterFloat>
		(
		"gain", "Gain",
		juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f, 1.0f),
			-40.0f
		));

	layout.add (std::make_unique<juce::AudioParameterFloat>
		(
		"attack", "Attack",
		juce::NormalisableRange<float> (.1f, 420.0f, 0.1f, 0.2f),
			20.0f
		));

	layout.add (std::make_unique<juce::AudioParameterFloat>
		(
		"release", "Release",
		juce::NormalisableRange<float> (.1f, 420.0f, 0.1f, 0.2f),
			60.0f
		));

	layout.add (std::make_unique<juce::AudioParameterFloat>
		(
		"trim", "Trim",
		juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f, 1.0f),
			0.0f
		));

	return layout;
}

KeyDipAudioProcessor::~KeyDipAudioProcessor()
{
	MTS_DeregisterClient(mtsesp);
}

const juce::String KeyDipAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KeyDipAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KeyDipAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KeyDipAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KeyDipAudioProcessor::getTailLengthSeconds() const
{
    return 0.;
}

int KeyDipAudioProcessor::getNumPrograms()
{
    return 1;
}

int KeyDipAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KeyDipAudioProcessor::setCurrentProgram (int)
{
}

const juce::String KeyDipAudioProcessor::getProgramName (int)
{
    return {};
}

void KeyDipAudioProcessor::changeProgramName (int, const juce::String&)
{
}

void KeyDipAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	juce::dsp::ProcessSpec spec;
	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = samplesPerBlock;
	spec.numChannels = getTotalNumOutputChannels();

	for (auto& voice : voices)
		voice.prepare(spec);
	q = gain = atk = rls = -60.f;
}

void KeyDipAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KeyDipAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void KeyDipAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels  = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();
	const auto numSamples = buffer.getNumSamples();
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear (i, 0, numSamples);
	if (numSamples == 0)
		return;
	static constexpr float Eps = .0001f;

	const auto currentQFactor = apvts.getRawParameterValue("qFactor")->load();
	const auto currentGain = apvts.getRawParameterValue("gain")->load();
	if (std::abs(currentQFactor - q) > Eps ||
		std::abs(currentGain - gain) > Eps)
	{
		q = currentQFactor;
		gain = currentGain;
		const auto gainAmp = std::pow(10.f, gain / 20.f);
		for (auto i = 0; i < NumVoices; ++i)
		{
			auto& voice = voices[i];
			voice.updateFilter(q, gainAmp);
		}
	}

	const auto currentAttack = apvts.getRawParameterValue("attack")->load();
	const auto currentRelease = apvts.getRawParameterValue("release")->load();
	if(std::abs(currentAttack - atk) > Eps ||
		std::abs(currentRelease - rls) > Eps)
	{
		atk = currentAttack;
		rls = currentRelease;
		for (auto i = 0; i < NumVoices; ++i)
		{
			auto& voice = voices[i];
			voice.updateEnvelope(atk, rls);
		}
	}

	auto s = 0;
	for (const auto metadata : midiMessages)
	{
		const auto ts = metadata.samplePosition;
		const auto msg = metadata.getMessage();

		if (s < ts)
		{
			const auto numSamplesSub = ts - s;
			juce::dsp::AudioBlock<float> block(buffer);
			auto subBlock = block.getSubBlock(s, numSamplesSub);
			juce::dsp::ProcessContextReplacing<float> context(subBlock);
			for (int i = 0; i < NumVoices; ++i)
				voices[i](context);
		}

		if (msg.isNoteOn())
		{
			const auto noteNumber = msg.getNoteNumber();
			auto& voice = voices[activeFilterIndex];
			voice.processNoteOn(mtsesp, noteNumber);
			activeFilterIndex = (activeFilterIndex + 1) % NumVoices;
		}
		else if (msg.isNoteOff())
		{
			const auto noteNumber = msg.getNoteNumber();
			for (auto i = 0; i < NumVoices; ++i)
				voices[i].processNoteOff(noteNumber);
		}
		else if (msg.isAllNotesOff())
		{
			for (auto i = 0; i < NumVoices; ++i)
				voices[i].processNoteOff();
			activeFilterIndex = 0;
		}

		s = ts;
	}

	const auto ts = numSamples;
	if (s < numSamples)
	{
		juce::dsp::AudioBlock<float> block(buffer);
		const auto numSamplesSub = ts - s;
		auto subBlock = block.getSubBlock(s, numSamplesSub);
		juce::dsp::ProcessContextReplacing<float> context(subBlock);
		for (int i = 0; i < NumVoices; ++i)
			voices[i](context);
	}

	const auto trimDb = apvts.getRawParameterValue("trim")->load();
	const auto trimAmp = std::pow(10.f, trimDb / 20.f);
	for (int i = 0; i < totalNumOutputChannels; ++i)
		buffer.applyGain(i, 0, numSamples, trimAmp);
}

bool KeyDipAudioProcessor::hasEditor() const
{
    return false;
}

juce::AudioProcessorEditor* KeyDipAudioProcessor::createEditor()
{
    return nullptr;
}

void KeyDipAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.state.createXml();
    copyXmlToBinary (*state, destData);
}

void KeyDipAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xmlState = getXmlFromBinary (data, sizeInBytes);
    if (xmlState != nullptr)
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KeyDipAudioProcessor();
}
