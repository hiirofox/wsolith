/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
LModelAudioProcessor::LModelAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
#endif
{

}


juce::AudioProcessorValueTreeState::ParameterLayout LModelAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	layout.add(std::make_unique<juce::AudioParameterFloat>("pos", "pos", 0, 1, 1));
	layout.add(std::make_unique<juce::AudioParameterFloat>("time", "time", 0, 1, 1));
	layout.add(std::make_unique<juce::AudioParameterFloat>("block", "block", 0, 1, 0.5));
	layout.add(std::make_unique<juce::AudioParameterFloat>("range", "range", 0, 1, 0.5));
	layout.add(std::make_unique<juce::AudioParameterFloat>("feedback", "feedback", 0, 1, 0));

	layout.add(std::make_unique<juce::AudioParameterFloat>("wf1", "wt1", 0, 2, 0));
	layout.add(std::make_unique<juce::AudioParameterFloat>("rt1", "rt1", 0, 1, 0));
	layout.add(std::make_unique<juce::AudioParameterFloat>("mx1", "mx1", 0, 1, 0));
	layout.add(std::make_unique<juce::AudioParameterFloat>("wf2", "wt2", 0, 2, 0));
	layout.add(std::make_unique<juce::AudioParameterFloat>("rt2", "rt2", 0, 1, 0));
	layout.add(std::make_unique<juce::AudioParameterFloat>("mx2", "mx2", 0, 1, 0));

	return layout;
}

LModelAudioProcessor::~LModelAudioProcessor()
{
}

//==============================================================================
const juce::String LModelAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool LModelAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool LModelAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool LModelAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double LModelAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int LModelAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int LModelAudioProcessor::getCurrentProgram()
{
	return 0;
}

void LModelAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String LModelAudioProcessor::getProgramName(int index)
{
	return "wsola granular";
}

void LModelAudioProcessor::changeProgramName(int index, const juce::String& newName)
{

}

//==============================================================================
void LModelAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
}

void LModelAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LModelAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
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


void LModelAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	int isMidiUpdata = 0;
	juce::MidiMessage MidiMsg;//先处理midi事件
	int MidiTime;
	juce::MidiBuffer::Iterator MidiBuf(midiMessages);
	while (MidiBuf.getNextEvent(MidiMsg, MidiTime))
	{
		if (MidiMsg.isNoteOn())
		{
			int note = MidiMsg.getNoteNumber() - 24;
		}
		if (MidiMsg.isNoteOff())
		{
			int note = MidiMsg.getNoteNumber() - 24;
		}
	}
	midiMessages.clear();

	numSamples = buffer.getNumSamples();
	float* wavbufl = buffer.getWritePointer(0);
	float* wavbufr = buffer.getWritePointer(1);
	const float* recbufl = buffer.getReadPointer(0);
	const float* recbufr = buffer.getReadPointer(1);

	float SampleRate = getSampleRate();

	float pos = *Params.getRawParameterValue("pos");
	float time = *Params.getRawParameterValue("time");
	float block = *Params.getRawParameterValue("block");
	float range = *Params.getRawParameterValue("range");
	float feedback = *Params.getRawParameterValue("feedback");
	float wf1 = *Params.getRawParameterValue("wf1");
	float rt1 = *Params.getRawParameterValue("rt1");
	float mx1 = *Params.getRawParameterValue("mx1");
	float wf2 = *Params.getRawParameterValue("wf2");
	float rt2 = *Params.getRawParameterValue("rt2");
	float mx2 = *Params.getRawParameterValue("mx2");


	auto toexp = [](float x, float n) {
		float sign = x < 0 ? -1.0f : 1.0f;
		x = fabsf(x);
		return (expf((x - 1.0) * n) - expf(-n)) / (1.0 - expf(-n)) * sign;
		};

	block = toexp(block, 3);
	range = toexp(range, 3);
	block = block * (8192 - 256) + 256;
	range = range * (8192 - 0) + 0;
	block = (int)block / 4 * 4;
	range = (int)range / 4 * 4;

	time = toexp(time, 2);

	rt1 = toexp(rt1, 6) / 4;
	rt2 = toexp(rt2, 6) / 4;
	wf1t += rt1;
	wf2t += rt2;
	wf1t -= (int)wf1t;
	wf2t -= (int)wf2t;
	float sin1 = sinf(wf1t * 2.0 * M_PI);
	float sqr1 = wf1t > 0.5 ? -1.0 : 1.0;
	float saw1 = atanf(tanf(wf1t * M_PI));
	float sin2 = sinf(wf2t * 2.0 * M_PI);
	float sqr2 = wf2t > 0.5 ? -1.0 : 1.0;
	float saw2 = atanf(tanf(wf2t * M_PI));
	float v1 = 0, v2 = 0;
	if (wf1 < 1.0)
	{
		float mix = wf1;
		v1 = sin1 * (1.0 - mix) + sqr1 * mix;
	}
	else
	{
		float mix = wf1 - 1.0;
		v1 = sqr1 * (1.0 - mix) + saw1 * mix;
	}
	if (wf2 < 1.0)
	{
		float mix = wf2;
		v2 = sin2 * (1.0 - mix) + sqr2 * mix;
	}
	else
	{
		float mix = wf2 - 1.0;
		v2 = sqr2 * (1.0 - mix) + saw2 * mix;
	}

	pos -= v1 * mx1 + v2 * mx2;

	psl.SetBlockRange(block, range);
	psr.SetBlockRange(block, range);
	psl.SetPointer(pos, time);
	psr.SetPointer(pos, time);

	float esum = 0;
	if (readpos >= writepos - numSamples)readpos = writepos - numSamples - 1;
	if (readpos < 0)readpos = 0;
	for (int i = 0; i < numSamples; ++i)
	{
		int index = (readpos++) % 8192;
		tmpbufl[i] = recbufl[i] + minibufl[index];
		tmpbufr[i] = recbufr[i] + minibufr[index];

		float e = tmpbufl[i] + tmpbufr[i];
		e *= e;
		esum += e;
	}
	energy[epos] = esum / numSamples;
	epos = (epos + 1) % MaxEnergyNum;

	psl.ProcessBlock(tmpbufl, wavbufl, numSamples);
	psr.ProcessBlock(tmpbufr, wavbufr, numSamples);

	for (int i = 0; i < numSamples; ++i)
	{
		int index = (writepos++) % 8192;
		minibufl[index] = wavbufl[i] * feedback;
		minibufr[index] = wavbufr[i] * feedback;
	}
}

//==============================================================================
bool LModelAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LModelAudioProcessor::createEditor()
{
	return new LModelAudioProcessorEditor(*this);

	//return new juce::GenericAudioProcessorEditor(*this);//自动绘制(调试)
}

//==============================================================================

void LModelAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.

	// 创建一个 XML 节点
	juce::XmlElement xml("LMEQ_Settings");

	/*juce::MemoryBlock eqDataBlock;
	eqDataBlock.append(&manager, sizeof(ResonatorManager));
	juce::String base64Data = eqDataBlock.toBase64Encoding();
	xml.setAttribute("VIB_MANAGER", base64Data);//添加resonance数据
	*/
	auto state = Params.copyState();
	xml.setAttribute("Knob_Data", state.toXmlString());//添加旋钮数据

	juce::String xmlString = xml.toString();
	destData.append(xmlString.toRawUTF8(), xmlString.getNumBytesAsUTF8());
}

void LModelAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
	  // 将 data 转换为字符串以解析 XML
	juce::String xmlString(static_cast<const char*>(data), sizeInBytes);

	// 解析 XML
	std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(xmlString));
	if (xml == nullptr || !xml->hasTagName("LMEQ_Settings"))
	{
		DBG("Error: Unable to load XML settings");
		return;
	}

	/*
	juce::String base64Data = xml->getStringAttribute("VIB_MANAGER");
	juce::MemoryBlock eqDataBlock;
	eqDataBlock.fromBase64Encoding(base64Data);
	if (eqDataBlock.getData() != NULL)
	{
		std::memcpy(&manager, eqDataBlock.getData(), sizeof(ResonatorManager));
	}
	*/
	auto KnobDataXML = xml->getStringAttribute("Knob_Data");
	Params.replaceState(juce::ValueTree::fromXml(KnobDataXML));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new LModelAudioProcessor();
}
