#ifndef _READWAV_H_
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

extern struct WavFileHead;

// original functions
// bool	WaveRewind(FILE *wav_file, WavFileHead *wavFileHead);
short	*ReadWave(const char *wavFile, int *numSamples, int *sampleRate);
void	WriteWave(const char *wavFile, short *waveData, int numSamples, int sampleRate);
void	FillWaveHeader(void *buffer, int raw_wave_len, int sampleRate);

// additive functions
void    GetWavHeader(const char *wavFile,short *Bits,int *Rate,short *Format,int *Length,short *Channels);
short   *ReadWavFile(const char *wavFile, int *numSamples, int *sampleRate);
void    readwav_t(const char *wavFile, short *waveData, long times, int *numSamples, int *sampleRate);
void    GetWavTime(const char *wavFile, double *duration);
void    ReadWav(const char *wavFile, short *waveData, int *numSamples, int *sampleRate);

#endif //_READWAV_H_
