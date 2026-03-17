#pragma once
#include "Voice.h"
#include <JuceHeader.h>

struct KeyDipAudioProcessor :
    public juce::AudioProcessor
{
    KeyDipAudioProcessor();
    ~KeyDipAudioProcessor() override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static constexpr int NumVoices = 16;
    MTSClient* mtsesp;
    std::array<float, 128> freqsArray;
    juce::AudioProcessorValueTreeState apvts;
	std::atomic<float> &qParam, &gainParam, &atkParam, &rlsParam, &trimParam;
    juce::AudioBuffer<float> bufferFilter;
    std::vector<float> envBuffer;
    std::array<Voice, NumVoices> voices;
    float q, gain, atk, rls;
    int activeFilterIndex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyDipAudioProcessor)
};
