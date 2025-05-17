#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include "fft.h"
#include "string.h"

class Wsolith
{
public:
	constexpr static int MaxFFTLen = 16384;//FFT����
	constexpr static int MaxInBufferSize = 131072;//һ��Ҫ�㹻��
	constexpr static int MaxOutBufferSize = MaxFFTLen / 2;
	constexpr static int MaxBlockSize = MaxFFTLen / 2;//���С
	constexpr static int MaxRange = MaxFFTLen / 2;//������Χ

	//ui interface
	float scale = 1.0;
	float select = 1.0;
	float ptr2 = 0;
	const float* GetInBuf() { return inbuf; }
	float GetScale() { return scale; }
	int GetPos() { return pos; }
	float GetPointer() { return ptr2; }
	int GetRange() { return range; }
	float GetSelect() { return (float)select / range; }
private:
	int blockSize = MaxBlockSize;
	int hopSize = blockSize / 4;
	int range = MaxRange;//������Χ

	int stepsum = 0;
	float pointer = 1.0;//�ؼ�!

	float window[MaxBlockSize] = { 0 };
	float window2[MaxBlockSize + MaxRange] = { 0 };

	float inbuf[MaxInBufferSize] = { 0 };
	int pos = 0;
	float copybuf[MaxInBufferSize] = { 0 };
	float globalBuf[MaxInBufferSize] = { 0 };

	float outbuf[MaxOutBufferSize] = { 0 };
	int writepos = 0;//дָ��
	int readpos = 0;//��ָ��
	int GetMaxCorrIndex1()//��������(��ͳ����)
	{
		int index = 0;
		float max = -999999;
		for (int i = 0; i < range; ++i)
		{
			float sum = 0;
			for (int j = 0; j < blockSize; ++j)
			{
				sum += copybuf[i + j] * globalBuf[j];
			}
			if (sum > max)
			{
				max = sum;
				index = i;
			}
		}
		//printf("index:%5d\t\tmax:%5.8f\n", index, max);
		return index;
	}
	int FFTLen = 4096;
	float re1[MaxFFTLen] = { 0 };
	float im1[MaxFFTLen] = { 0 };
	float re2[MaxFFTLen] = { 0 };
	float im2[MaxFFTLen] = { 0 };
	int GetMaxCorrIndex2()//��������(fft) �޺���
	{
		int len1 = range + blockSize;
		for (int i = 0; i < len1; ++i) {
			//float w = window2[i];
			float w = 1.0;
			re1[i] = copybuf[i] * w * 0.1;
			im1[i] = 0.0f;
		}
		for (int i = len1; i < FFTLen; ++i) {
			re1[i] = 0.0f;
			im1[i] = 0.0f;
		}
		for (int i = 0; i < blockSize; ++i) {
			float w = window[i];
			re2[i] = globalBuf[i] * w * 0.1;
			im2[i] = 0.0f;
		}
		for (int i = blockSize; i < FFTLen; ++i) {
			re2[i] = 0.0f;
			im2[i] = 0.0f;
		}
		fft_f32(re1, im1, FFTLen, 1);
		fft_f32(re2, im2, FFTLen, 1);
		for (int i = 0; i < FFTLen; ++i) {
			float re1v = re1[i];
			float im1v = im1[i];
			float re2v = re2[i];
			float im2v = -im2[i];//����ù���
			re1[i] = re1v * re2v - im1v * im2v;
			im1[i] = re1v * im2v + im1v * re2v;
		}
		fft_f32(re1, im1, FFTLen, -1);
		int index = 0;
		float max = -9999999999;
		for (int i = 0; i < range; ++i) {
			float r = re1[i] * re1[i] + im1[i] * im1[i];
			if (r > max) {
				max = r;
				index = i;
			}
		}
		//printf("index:%5d\t\tmax:%5.8f\n", index, max);
		return index;
	}
	int GetMaxCorrIndex3()//��ola
	{
		return 0;
	}
public:
	Wsolith()
	{
		memset(inbuf, 0, sizeof(inbuf));
		memset(outbuf, 0, sizeof(outbuf));
		SetBlockRange(2048, 2048);
	}
	void SetBlockRange(int blockSize, int searchRange)
	{
		if (this->blockSize == blockSize && this->range == searchRange)return;
		if (blockSize + range > MaxFFTLen)return;

		this->blockSize = blockSize;
		this->range = searchRange;
		for (int i = 0; i < blockSize; ++i)
		{
			window[i] = 0.5 - 0.5 * cosf(2.0 * M_PI * i / blockSize);
		}
		for (int i = 0; i < blockSize + range; ++i)
		{
			window2[i] = 0.5 - 0.5 * cosf(2.0 * M_PI * i / (blockSize + range));
		}
		hopSize = blockSize / 4;
		FFTLen = 1 << (int)(ceilf(log2f(blockSize + range)));
	}
	void SetPointer(float pos1, float scale)
	{
		ptr2 = pos1;
		pos1 = 1.0 - (1.0 - pos1) * scale;
		if (pos1 < 0)pos1 = 0;
		if (pos1 > 1)pos1 = 1;
		this->scale = scale;
		pointer = pos1;
	}
	void ProcessIn(const float* buf, int len)
	{
		for (int i = 0; i < len; ++i)
		{
			inbuf[pos] = buf[i];
			pos = (pos + 1) % MaxInBufferSize;

			stepsum++;
			if (stepsum >= hopSize)//�ø�����
			{
				stepsum -= hopSize;
				int posv = pointer * (MaxInBufferSize - range - hopSize - blockSize);
				int start = (int)(pos + posv) % MaxInBufferSize;
				for (int j = 0; j < range + blockSize; ++j)
				{
					copybuf[j] = inbuf[(start + j) % MaxInBufferSize];
				}

				//int index = GetMaxCorrIndex1();//����Ŀ����������
				int index = GetMaxCorrIndex2();//����Ŀ����������

				int start2 = start + index + hopSize;//������Ŀ��飬��׼wsolaʵ��
				//int start2 = start + index;//�������ģ�����һ�¿���ʵ�̶ֹ����߱��������
				for (int j = 0; j < blockSize; ++j)//����Ŀ���
				{
					//globalBuf[j] = inbuf[(start2 + j) % MaxInBufferSize] * window[j];
					globalBuf[j] = inbuf[(start2 + j) % MaxInBufferSize];
				}

				select = index;//ui�á�ʵ����ѡȡ��index

				int start3 = start + index;
				float normv = hopSize / (blockSize / 2.0);//hop = 1/4
				for (int j = 0, k = writepos; j < blockSize; ++j)//����
				{
					outbuf[k % MaxOutBufferSize] += copybuf[j + index] * window[j] * normv;
					k++;
				}
				writepos += hopSize;
			}

		}
	}
	int PrepareOut(int len)
	{
		int n = writepos - readpos - len;
		if (n < 0)return 0;
		return n;
	}
	void ProcessOut(float* buf, int len)
	{
		for (int i = 0; i < len; ++i)
		{
			int index = readpos % MaxOutBufferSize;
			buf[i] = outbuf[index];
			outbuf[index] = 0;
			readpos++;
		}
	}
	void ProcessBlock(const float* in, float* out, int numSamples)
	{
		ProcessIn(in, numSamples);
		if (PrepareOut(numSamples))
		{
			ProcessOut(out, numSamples);
		}
		else
		{
			for (int i = 0; i < numSamples; ++i)
			{
				out[i] = 0;
			}
		}
	}
};