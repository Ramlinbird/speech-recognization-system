#include "featurecomputation.h"

const int filterNum = 40, dimDCT = 13;
float melFreq[filterNum+2] = {0};
float freq[filterNum+2] = {0};
ofstream powerFile, logMelFile, dctFile, norcepFile;
int numSamples, sampleRate;

float freq2mel(float freq)
{
	return 2595*log10f(1+freq/700);
}

float mel2freq(float mel)
{
	return (pow(10, mel/2595)-1)*700;
}

void calMI(float MI[][filterNum], int n)
{
	float lowF,midF,highF,f;
	for (int i = 0; i < filterNum; ++i)
	{
		lowF = freq[i];
		midF = freq[i+1];
		highF = freq[i+2];
		for (int j = 0; j < n; ++j)
		{
			f = ((float)j/(n-1))*(sampleRate/2);
			if (f >= lowF && f <= midF)
				MI[j][i] = (f-lowF)/(midF-lowF);
			else if (f > midF && f <= highF)
				MI[j][i] = (highF-f)/(highF-midF);
			else
				MI[j][i] = 0;
		}
	}
}

int zeroPaddingNum(int prelen)
{
	int i = 0;
	while(true){
		if (pow(2, i) >= prelen)
			break;
		i = i+1;
	}
	return pow(2, i);
}

void constructWindow(float win[], int len)
{
	for (int i = 0; i < len; ++i)
	{
		win[i] = 0.54 - 0.46*cosf(2.0*PI*i/(len-1));
	}
}

void applyWindow(float frame[], float win[], int len)
{
	for (int i = 0; i < len; ++i)
	{
		frame[i] = frame[i]*win[i];
	}
}

void zeroPadding(float frame[], Complex zeroPadFrame[], int len)
{
	for (int i = 0; i < len; ++i)
	{
		zeroPadFrame[i] = frame[i];
	}
}

void calPower(CArray& fftVal, float power[], int len)
{
	for (int i = 0; i < len; ++i)
	{
		power[i] = pow(fftVal[i].real(), 2) + pow(fftVal[i].imag(), 2);
	}
}

void applyFilter(float filterOutput[], float pows[], float MI[][filterNum], int n)
{
	for (int i = 0; i < filterNum; ++i)
	{
		filterOutput[i] = 0;
		for (int j = 0; j < n; ++j)
		{
			filterOutput[i] += pows[j]*MI[j][i];
		}
	}
}

void calMelSpectrum(float filterOutput[], float melSpectrum[])
{
	for (int i = 0; i < filterNum; ++i)
	{
		melSpectrum[i] = log(filterOutput[i]+eps);
	}
}

void calMelCepstrum(float melSpectrum[], float melCepstrum[])
{
	for (int i = 0; i < dimDCT; ++i)
	{
		melCepstrum[i] = 0;
		for (int j = 0; j < filterNum; ++j)
		{
			melCepstrum[i] += melSpectrum[j]*cosf(PI*i*(2*j+1)/(2*filterNum));
		}
	}
}

void normalization(float normalizedCeps[][dimDCT], int nframes)
{
	float m[dimDCT] = {0}, sum[dimDCT] = {0};
	for (int i = 0; i < dimDCT; i++)
	{
		for (int j = 0; j < nframes; j++)
		{
			sum[i] += normalizedCeps[j][i];
		}
		m[i] = sum[i]/nframes;
	}

	for (int i = 0; i < dimDCT; i++)
	{
		for (int j = 0; j < nframes; j++)
		{
			normalizedCeps[j][i] -= m[i];
		}
	}
	
	if (norcepFile.is_open())
	{
		for (int i = 0; i < nframes; i++)
		{
			for (int j = 0; j < dimDCT; j++)
			{
				norcepFile << normalizedCeps[i][j] << ",";
			}
			norcepFile << endl;
		}
	}
	else
	{
		cout << "Unable to open file " << file4 << endl;
	}
}

void display(string title, float ary[], int len)
{
	cout << "\n========" << title << "========" << endl;
	for (int i = 0; i < len; ++i)
	{
		cout << ary[i] << ", ";
	}
	cout << endl;
}

void saveToFiles(float ary1[], int powLen, float ary2[], float ary3[])
{
	if (powerFile.is_open())
	{
		for (int i = 0; i < powLen; i++)
		{
			powerFile << ary1[i] << ",";
		}
		powerFile << endl;
	}
	else
	{
		cout << "Unable to open file " << file1 << endl;
	}

	if (logMelFile.is_open())
	{
		for (int i = 0; i < filterNum; i++)
		{
			logMelFile << ary2[i] << ",";
		}
		logMelFile << endl;
	}
	else
	{
		cout << "Unable to open file " << file2 << endl;
	}

	if (dctFile.is_open())
	{
		for (int i = 0; i < dimDCT; i++)
		{
			dctFile << ary3[i] << ",";
		}
		dctFile << endl;
	}
	else
	{
		cout << "Unable to open file " << file3 << endl;
	}
}

void summary(int frameSize, int crossSzie, int frameCount)
{
	cout << "\n********** Information about Processing **********\n"; 
	cout << "File  Name : " << wavFile << endl;
	cout << "Samples Cnt: " << numSamples << endl;
	cout << "Sample Rate: " << sampleRate << endl;
	cout << "Frame Size : " << frameSize << endl;
	cout << "Cross Size : " << crossSzie << endl;
	cout << "Frame Count: " << frameCount << endl;
	cout << "Filters Cnt: " << filterNum << endl;
}

bool feature_computation()
{
	short	*wavData;
	wavData = ReadWavFile(wavFile, &numSamples, &sampleRate);
	
	printf("\n\n===== Feature Computation Starting. =====\n"); fflush(stdout);
	
	// Some parameters
	int frameSize = sampleRate*frameTime;
	int crossSize = sampleRate*crossTime;

	float filterOutput[filterNum] = {0};
	float melSpectrum[filterNum] = {0};
	float melCepstrum[dimDCT] = {0};
	int maxFrameCount = ceil((float)numSamples/crossSize);
	float *tmp1 = new float[maxFrameCount*dimDCT];
	float (*normalizedCeps)[dimDCT] = (float(*)[dimDCT])tmp1;

	const int nzeroPad = zeroPaddingNum(frameSize);

	/**************** Mel Filters Parameters ***************/
	float maxMel = freq2mel(sampleRate/2);
	float delMel = maxMel/(filterNum+1);
	for (int i = 0; i < filterNum+2; ++i)
	{
		melFreq[i] = delMel*i;
	}
	for (int i = 0; i < filterNum+2; i++)
	{
		freq[i] = mel2freq(melFreq[i]);
	}
	float *tmp2 = new float[nzeroPad/2*filterNum];
	float (*MI)[filterNum] = (float(*)[filterNum])tmp2;
	calMI(MI, nzeroPad/2);
	/******************************************************/

	/** Window Parameters(Assume numSamples >= frameSize) */
	float *win = new float[frameSize];
	constructWindow(win, frameSize);
	/******************************************************/

	/*********** Pre-emphasizing a speech signal **********/
	float *empData = new float[numSamples];
	empData[0] = 0.05*wavData[0];
	for (int i = 1; i < numSamples; ++i)
	{
		empData[i] = wavData[i] - 0.95*wavData[i-1];
	}
	/******************************************************/


	/* =========== Framing and processing ===============*/
	float *frame = new float[frameSize];
	int frameStart = 0, frameEnd = frameSize, frameLen = frameSize, frameCount = 0;
	powerFile.open (file1);
	logMelFile.open(file2);
	dctFile.open(file3);
	while(true){
		/**************** Current Frame ***************/
		frameCount++;
		for (int i = 0; i < frameLen; i++)
		{
			frame[i] = empData[i+frameStart];
		}

		// Windowing
		applyWindow(frame, win, frameLen);

		// Zero Padding
		Complex *zeroPadFrame = new Complex[nzeroPad];
		memset(zeroPadFrame, 0, nzeroPad*sizeof(Complex));
		zeroPadding(frame, zeroPadFrame, frameLen);

		// FFT Transform
		CArray cframe(zeroPadFrame, nzeroPad);
		fft(cframe);

		// Power Spectrum
		float *pows = new float[nzeroPad/2];
		calPower(cframe, pows, nzeroPad/2);

		// Mel Spectrum
		applyFilter(filterOutput, pows, MI, nzeroPad/2);

		// Log Mel Spectrum
		calMelSpectrum(filterOutput, melSpectrum);

		// Mel Cepstrum
		calMelCepstrum(melSpectrum, melCepstrum);

		// Save melCepstrum for normalization
		for (int i = 0; i < dimDCT; i++)
		{
			normalizedCeps[frameCount-1][i] = melCepstrum[i];
		}

		// Show Final Results
		// display("Mel Cepstrum", melCepstrum, dimDCT);

		// Save All Results for Matlab Plotting
		saveToFiles(pows, nzeroPad/2, melSpectrum, melCepstrum);

		/**************** Next Frame ***************/
		frameStart += crossSize;
		frameEnd += crossSize;
		frameLen = frameEnd-frameStart;
		if (frameStart >= numSamples)
		{
			// Normalize the mel cepstrum values
			norcepFile.open(file4);
			normalization(normalizedCeps, frameCount);
			cout << "max = " << maxFrameCount << endl;

			summary(frameSize, crossSize, frameCount);
			printf("Done.\n"); fflush(stdout);
			break;	// no more frames
		}
		if (frameEnd >= numSamples)
		{
			frameEnd = numSamples;
			frameLen = frameEnd-frameStart;
			// Length of frame changed, Reconstruct Window
			constructWindow(win, frameLen);
		}
	}
	/* =================== END =========================*/

	powerFile.close();
	logMelFile.close();
	dctFile.close();
	norcepFile.close();
	delete[] tmp1;
	delete[] tmp2;
	delete[] empData;
	delete[] win;
	delete[] frame;
	
	return true;
}