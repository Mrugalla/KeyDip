#pragma once
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

struct KeyDipAudioProcessorEditor :
    public juce::AudioProcessorEditor
{
	using BoundsF = juce::Rectangle<float>;

    KeyDipAudioProcessorEditor(KeyDipAudioProcessor& p) :
        AudioProcessorEditor(&p),
        processor(p),
        bg(juce::ImageCache::getFromMemory(BinaryData::bg_png, BinaryData::bg_pngSize)),
        qSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
        gainSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
        attackSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
        releaseSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
        trimSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
        qLabel("", "Q"),
		gainLabel("", "Gain"),
        attackLabel("", "Atk"),
		releaseLabel("", "Rls"),
        trimLabel("", "Trim"),
		qAttachment(processor.apvts, "q", qSlider),
		gainAttachment(processor.apvts, "gain", gainSlider),
		attackAttachment(processor.apvts, "attack", attackSlider),
		releaseAttachment(processor.apvts, "release", releaseSlider),
		trimAttachment(processor.apvts, "trim", trimSlider)
    {
        addAndMakeVisible(qSlider);
        addAndMakeVisible(qLabel);
        addAndMakeVisible(gainSlider);
        addAndMakeVisible(gainLabel);
        addAndMakeVisible(attackSlider);
        addAndMakeVisible(attackLabel);
        addAndMakeVisible(releaseSlider);
        addAndMakeVisible(releaseLabel);
        addAndMakeVisible(trimSlider);
        addAndMakeVisible(trimLabel);
        qLabel.setJustificationType(juce::Justification::centred);
        gainLabel.setJustificationType(juce::Justification::centred);
		attackLabel.setJustificationType(juce::Justification::centred);
		releaseLabel.setJustificationType(juce::Justification::centred);
		trimLabel.setJustificationType(juce::Justification::centred);
		qLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
		gainLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
		attackLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
		releaseLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
		trimLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
		qSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
		gainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
		attackSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
		releaseSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
		trimSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
		qSlider.setColour(juce::Slider::thumbColourId, juce::Colours::black);
		gainSlider.setColour(juce::Slider::thumbColourId, juce::Colours::black);
		attackSlider.setColour(juce::Slider::thumbColourId, juce::Colours::black);
		releaseSlider.setColour(juce::Slider::thumbColourId, juce::Colours::black);
		trimSlider.setColour(juce::Slider::thumbColourId, juce::Colours::black);

		setOpaque(true);
        setSize(420, 240);
    }

	void paint(juce::Graphics& g) override
	{
		if (bg.isValid())
			g.drawImageAt(bg, 0, 0, false);
		else
			g.fillAll(juce::Colours::black);
	}

    void resized() override
    {
        const auto thicc = 2.f;
        const auto bounds = getLocalBounds().toFloat().reduced(thicc);
		const auto titleBounds = bounds.withHeight(bounds.getHeight() * .4f);
		const auto slidersBounds = bounds.withTrimmedTop(titleBounds.getHeight() + thicc);
        {
            auto x = slidersBounds.getX();
            const auto w = slidersBounds.getWidth() / 5.f;
			const auto h = slidersBounds.getHeight();
            const auto yLabel = slidersBounds.getY();
            const auto hLabel = slidersBounds.getHeight() * .2f;
			const auto ySlider = yLabel + hLabel;
			const auto hSlider = h - hLabel;
            qSlider.setBounds(BoundsF(x, ySlider, w, hSlider).toNearestInt());
			qLabel.setBounds(BoundsF(x, yLabel, w, hLabel).toNearestInt());
            x += w;
            gainSlider.setBounds(BoundsF(x, ySlider, w, hSlider).toNearestInt());
			gainLabel.setBounds(BoundsF(x, yLabel, w, hLabel).toNearestInt());
            x += w;
            attackSlider.setBounds(BoundsF(x, ySlider, w, hSlider).toNearestInt());
			attackLabel.setBounds(BoundsF(x, yLabel, w, hLabel).toNearestInt());
            x += w;
            releaseSlider.setBounds(BoundsF(x, ySlider, w, hSlider).toNearestInt());
			releaseLabel.setBounds(BoundsF(x, yLabel, w, hLabel).toNearestInt());
            x += w;
            trimSlider.setBounds(BoundsF(x, ySlider, w, hSlider).toNearestInt());
			trimLabel.setBounds(BoundsF(x, yLabel, w, hLabel).toNearestInt());
        }
    }
private:
    KeyDipAudioProcessor& processor;
    juce::Image bg;
    juce::Slider qSlider, gainSlider, attackSlider, releaseSlider, trimSlider;
    juce::Label qLabel, gainLabel, attackLabel, releaseLabel, trimLabel;
    juce::AudioProcessorValueTreeState::SliderAttachment
        qAttachment, gainAttachment, attackAttachment, releaseAttachment, trimAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyDipAudioProcessorEditor)
};