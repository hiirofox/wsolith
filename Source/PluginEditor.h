/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/LM_slider.h"
#include "ui/waveui.h"

//==============================================================================
/**
*/
class LModelAudioProcessorEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
	LModelAudioProcessorEditor(LModelAudioProcessor&);
	~LModelAudioProcessorEditor() override;

	//==============================================================================
	void paint(juce::Graphics&) override;
	void resized() override;
	void timerCallback() override;//ע�⣬�����治���ڻ��fft�����

private:
	// This reference is provided as a quick way for your editor to
	// access the processor object that created it.
	LModelAudioProcessor& audioProcessor;

	WaveUI wui;

	LMKnob K_Pos, K_Time;
	LMKnob K_Block;
	LMKnob K_Range, K_Pitch;
	LMKnob K_Feedback;

	LMKnob K_WF1, K_RT1, K_MX1;
	LMKnob K_WF2, K_RT2, K_MX2;

	juce::ComponentBoundsConstrainer constrainer;  // �������ÿ�߱���
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LModelAudioProcessorEditor)
};

