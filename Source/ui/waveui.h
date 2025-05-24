#pragma once

#include "JuceHeader.h"
#include "../dsp/wsolith.h"
class WaveUI :public juce::Component
{
private:
	std::unique_ptr<Wsolith> wsl;
	std::unique_ptr<Wsolith> wsr;
	float dbuf[8192];
public:
	WaveUI()
	{
	}
	~WaveUI()
	{
		wsl.release();
		wsr.release();
	}
	void SetWsolithPtr(Wsolith& l, Wsolith& r)
	{
		wsl.reset(&l);
		wsr.reset(&r);
	}
	void paint(juce::Graphics& g) override
	{
		juce::Rectangle<int> rect = getBounds();
		int w = rect.getWidth(), h = rect.getHeight();

		const float* bufl = wsl->GetInBuf();
		const float* bufr = wsr->GetInBuf();

		int MaxLen = wsl->MaxInBufferSize;
		int pos = wsl->GetPos();
		int len = (float)MaxLen * wsl->GetScale();
		int start = pos + MaxLen - len;



		for (int i = 0; i < w; ++i)dbuf[i] = 0;
		for (int i = 0; i < len; ++i)
		{
			int index1 = i * w / len;
			int index2 = (start + i) % MaxLen;
			//dbuf[index1] += bufl[index2] * bufl[index2];
			//dbuf[index1] += bufr[index2] * bufr[index2];
			dbuf[index1] = std::max(dbuf[index1], fabs(bufl[index2]));
			dbuf[index1] = std::max(dbuf[index1], fabs(bufr[index2]));
		}
		//for (int i = 0; i < w; ++i)dbuf[i] /= len;
		for (int i = 0; i < w; ++i)dbuf[i] = sqrtf(dbuf[i] / 1000.0) * h * 20.0;
		for (int i = 2; i < w; ++i)dbuf[i] = (dbuf[i] + dbuf[i - 1] + dbuf[i - 2]) / 3;
		g.setColour(juce::Colour(0xff004444));
		for (int i = 0; i < w; ++i)
		{
			float v = dbuf[i];
			g.drawLine(i, h / 2 - v - 1, i, h / 2 + v + 1, 2.0);
		}
		g.setColour(juce::Colour(0xff00ffff));
		for (int i = 1; i < w; ++i)
		{
			g.drawLine(i - 1, h / 2 - dbuf[i - 1], i, h / 2 - dbuf[i], 1.5);
			g.drawLine(i - 1, h / 2 + dbuf[i - 1], i, h / 2 + dbuf[i], 1.5);
		}

		float pointer = wsl->GetPointer();
		float rangev = (float)wsl->GetRange() / len;
		float select = wsl->GetSelect();
		g.setColour(juce::Colour(0x3300ff00));
		g.fillRect((float)pointer * w, (float)0, (float)(rangev > 1 ? 1 : rangev) * w, (float)h);
		g.setColour(juce::Colour(0xff00ff00));
		int sx = pointer * w + select * rangev * w;
		g.drawLine(sx, 0, sx, h, 2);


		g.setColour(juce::Colour(0xFF00FF00));
		g.drawLine(0, 0, w, 0, 3);
		g.drawLine(0, 0, 0, h, 3);
		g.drawLine(w, 0, w, h, 3);
		g.drawLine(0, h, w, h, 3);
	}
};