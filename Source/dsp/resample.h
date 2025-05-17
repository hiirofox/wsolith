#pragma once

class ReSampler//�ֲ��ǲ�����
{
private:
	constexpr static int MaxInBufferSize = 32768;//һ��Ҫ�㹻��
	constexpr static int MaxOutBufferSize = 32768;
	float rate = 1.0, globalRate = 1.0;//ԭʼ�����ʣ�Ŀ�������
	int isUpdata = 0;
	float inbuf[MaxInBufferSize] = { 0 };
	int pos = 0, posHop = 0;
	float outbuf[MaxOutBufferSize] = { 0 };
	int writepos = 0;//дָ��
	int readpos = 0;//��ָ��

	constexpr static int MaxFFTLen = 32768;//���FFT����
	float windowin[MaxFFTLen] = { 0 };//���봰
	float windowout[MaxFFTLen] = { 0 };//�����
	float bufre[MaxFFTLen] = { 0 };
	float bufim[MaxFFTLen] = { 0 };
	int fftlenin = 2048;//Ҳ����blocksize
	int fftlenout = 2048;
	int hopsizein = fftlenin / 4;
	int hopsizeout = fftlenout / 4;
public:
	ReSampler()
	{
		memset(inbuf, 0, sizeof(inbuf));
		memset(outbuf, 0, sizeof(outbuf));
		memset(windowin, 0, sizeof(windowin));
		memset(windowout, 0, sizeof(windowout));
		memset(bufre, 0, sizeof(bufre));
		memset(bufim, 0, sizeof(bufim));
		for (int i = 0; i < fftlenin; ++i)
		{
			windowin[i] = 0.5 - 0.5 * cosf(2.0 * M_PI * i / fftlenin);
		}
		for (int i = 0; i < fftlenout; ++i)
		{
			windowout[i] = 0.5 - 0.5 * cosf(2.0 * M_PI * i / fftlenout);
		}
	}
	void SetRate(float rate)//src:dst
	{
		globalRate = rate;
		isUpdata = 1;
	}
	void ProcessIn(const float* buf, int len)
	{
		for (int i = 0; i < len; ++i)
		{
			inbuf[pos] = buf[i];
			posHop++;
			if (posHop >= hopsizein)
			{
				posHop = 0;
				int start = MaxInBufferSize * 100 + pos - fftlenin;//���봰��߽�
				for (int j = 0; j < fftlenin; ++j)
				{
					bufre[j] = inbuf[(start + j) % MaxInBufferSize] * windowin[j];
					bufim[j] = 0.0f;
				}
				fft_f32(bufre, bufim, fftlenin, 1);//fft
				for (int i = fftlenin / 2; i < fftlenout; ++i)//ȥ������
				{
					bufre[i] = 0.0f;
					bufim[i] = 0.0f;
				}
				if (rate > 1.0)//����
				{
					for (int i = fftlenin / 2 / rate; i < fftlenin / 2; ++i)//Ŀ��Ĵ���
					{
						bufre[i] = 0.0f;
						bufim[i] = 0.0f;
					}
				}
				for (int i = 0; i < fftlenout / 2; ++i)//����
				{
					bufre[fftlenout - i - 1] = -bufre[i];//�Բ����ҵ�fft�����е�����
					bufim[fftlenout - i - 1] = bufim[i];
				}
				fft_f32(bufre, bufim, fftlenout, -1);//��fft
				for (int j = 0; j < fftlenout; ++j)
				{
					bufre[j] *= windowout[j];
				}

				float normEnergy = 1.0 / sqrtf(fftlenin * fftlenout);//��һ��

				int n = (float)fftlenin / rate;//�������

				float y0 = 0, y1 = 0, y2 = 0, y3 = 0, y4 = 0, y5 = 0;
				for (int j = 0, k = writepos; j < n; ++j)
				{
					float indexf = (float)j * fftlenout / n;
					int index = indexf;

					// �߽紦��ȷ��������Ч
					if (index < 3)
					{
						// ������߽磬���賬������Ϊ0
						y0 = 0;
						y1 = 0;
						y2 = 0;
						y3 = bufre[index + 0];
						y4 = bufre[index + 1];
						y5 = bufre[index + 2];
					}
					else if (index + 3 >= fftlenout)
					{
						// �����ұ߽�
						y0 = bufre[index - 3];
						y1 = bufre[index - 2];
						y2 = bufre[index - 1];
						y3 = 0;
						y4 = 0;
						y5 = 0;
					}
					else
					{
						// ���������ȡ������
						y0 = bufre[index - 3];
						y1 = bufre[index - 2];
						y2 = bufre[index - 1];
						y3 = bufre[index + 0];
						y4 = bufre[index + 1];
						y5 = bufre[index + 2];
					}

					// �������Lagrange������
					float t = indexf - index;
					float t_plus_2 = t + 2;
					float t_plus_1 = t + 1;
					float t_minus_1 = t - 1;
					float t_minus_2 = t - 2;
					float t_minus_3 = t - 3;

					float L0 = (t_plus_1 * t * t_minus_1 * t_minus_2 * t_minus_3) / (-120.0f);
					float L1 = (t_plus_2 * t * t_minus_1 * t_minus_2 * t_minus_3) / 24.0f;
					float L2 = (t_plus_2 * t_plus_1 * t_minus_1 * t_minus_2 * t_minus_3) / (-12.0f);
					float L3 = (t_plus_2 * t_plus_1 * t * t_minus_2 * t_minus_3) / 12.0f;
					float L4 = (t_plus_2 * t_plus_1 * t * t_minus_1 * t_minus_3) / (-24.0f);
					float L5 = (t_plus_2 * t_plus_1 * t * t_minus_1 * t_minus_2) / 120.0f;

					float out = y0 * L0 + y1 * L1 + y2 * L2 + y3 * L3 + y4 * L4 + y5 * L5;
					outbuf[k % MaxOutBufferSize] += out * normEnergy;
					k++;
				}
				writepos += hopsizeout;

				if (isUpdata)//�������ٸ���
				{
					isUpdata = 0;
					rate = globalRate;
					float n = (float)fftlenin / rate;//�������
					hopsizeout = n / 4;
					int m = 1 << (int)(ceilf(log2f(n)));
					if (m != fftlenout)
					{
						fftlenout = m;
						for (int i = 0; i < fftlenout; ++i)
						{
							windowout[i] = 0.5 - 0.5 * cosf(2.0 * M_PI * i / fftlenout);
						}
					}
				}
			}
			pos = (pos + 1) % MaxInBufferSize;
		}
	}
	int PrepareOut(int len)
	{
		int n = writepos - readpos - (len + fftlenin);
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

};

