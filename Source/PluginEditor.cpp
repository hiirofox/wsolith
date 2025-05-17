/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LModelAudioProcessorEditor::LModelAudioProcessorEditor(LModelAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	//setResizable(true, true); // 允许窗口调整大小

	setOpaque(false);  // 允许在边框外面绘制

	//setResizeLimits(64 * 11, 64 * 5, 10000, 10000); // 设置最小宽高为300x200，最大宽高为800x600
	setSize(64 * 13, 64 * 4);
	//setResizeLimits(64 * 13, 64 * 4, 64 * 13, 64 * 4);

	//constrainer.setFixedAspectRatio(11.0 / 4.0);  // 设置为16:9比例
	//setConstrainer(&constrainer);  // 绑定窗口的宽高限制

	K_Pos.setText("pos");
	K_Pos.ParamLink(audioProcessor.GetParams(), "pos");
	addAndMakeVisible(K_Pos);
	K_Time.setText("time");
	K_Time.ParamLink(audioProcessor.GetParams(), "time");
	addAndMakeVisible(K_Time);
	K_Block.setText("block");
	K_Block.ParamLink(audioProcessor.GetParams(), "block");
	addAndMakeVisible(K_Block);
	K_Range.setText("range");
	K_Range.ParamLink(audioProcessor.GetParams(), "range");
	addAndMakeVisible(K_Range);
	K_Feedback.setText("feedback");
	K_Feedback.ParamLink(audioProcessor.GetParams(), "feedback");
	addAndMakeVisible(K_Feedback);

	K_WF1.setText("wform1");
	K_WF1.ParamLink(audioProcessor.GetParams(), "wf1");
	addAndMakeVisible(K_WF1);
	K_RT1.setText("rate1");
	K_RT1.ParamLink(audioProcessor.GetParams(), "rt1");
	addAndMakeVisible(K_RT1);
	K_MX1.setText("mix1");
	K_MX1.ParamLink(audioProcessor.GetParams(), "mx1");
	addAndMakeVisible(K_MX1);

	K_WF2.setText("wform2");
	K_WF2.ParamLink(audioProcessor.GetParams(), "wf2");
	addAndMakeVisible(K_WF2);
	K_RT2.setText("rate2");
	K_RT2.ParamLink(audioProcessor.GetParams(), "rt2");
	addAndMakeVisible(K_RT2);
	K_MX2.setText("mix2");
	K_MX2.ParamLink(audioProcessor.GetParams(), "mx2");
	addAndMakeVisible(K_MX2);

	wui.SetWsolithPtr(p.psl, p.psr);
	addAndMakeVisible(wui);

	startTimerHz(30);
}

LModelAudioProcessorEditor::~LModelAudioProcessorEditor()
{
}

//==============================================================================
void LModelAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colour(0x00, 0x00, 0x00));

	g.fillAll();
	g.setFont(juce::Font("FIXEDSYS", 17.0, 1));
	g.setColour(juce::Colour(0xff00ff00));;

	int w = getBounds().getWidth(), h = getBounds().getHeight();

	g.drawText("L-MODEL WSOLith", juce::Rectangle<float>(32, 16, w, 16), 1);
}

void LModelAudioProcessorEditor::resized()
{
	juce::Rectangle<int> bound = getBounds();
	int x = bound.getX(), y = bound.getY(), w = bound.getWidth(), h = bound.getHeight();
	auto convXY = juce::Rectangle<int>::leftTopRightBottom;

	K_Pos.setBounds(32 + 64 * 0, h - 64 * 1 - 24, 64, 64);
	K_Time.setBounds(32 + 64 * 1, h - 64 * 1 - 24, 64, 64);
	K_Block.setBounds(32 + 64 * 2, h - 64 * 1 - 24, 64, 64);
	K_Range.setBounds(32 + 64 * 3, h - 64 * 1 - 24, 64, 64);
	K_Feedback.setBounds(32 + 64 * 4, h - 64 * 1 - 24, 64, 64);

	K_WF1.setBounds(w - 32 - 64 * 6 - 32, h - 64 * 1 - 24, 64, 64);
	K_RT1.setBounds(w - 32 - 64 * 5 - 32, h - 64 * 1 - 24, 64, 64);
	K_MX1.setBounds(w - 32 - 64 * 4 - 32, h - 64 * 1 - 24, 64, 64);
	K_WF2.setBounds(w - 32 - 64 * 3, h - 64 * 1 - 24, 64, 64);
	K_RT2.setBounds(w - 32 - 64 * 2, h - 64 * 1 - 24, 64, 64);
	K_MX2.setBounds(w - 32 - 64 * 1, h - 64 * 1 - 24, 64, 64);

	wui.setBounds(32, 32, w - 64, h - 64 - 32 - 32 - 8);
}

void LModelAudioProcessorEditor::timerCallback()
{
	repaint();
}
