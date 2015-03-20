#include "datacapture.h"

typedef struct
{
    int          frameIndex;
    int          maxFrameIndex;
    SAMPLE      *recordedSamples;
}
paTestData;

// Parameters for Endpointing
int    continuousSilenceCount = 0;
int    totalFrames;
bool   isSpeech;
double bgInit = 0;
double levelInit = 0;
int    initCount = 0;

// Calculate energy in decibel of one frame
double EnergyPerFrameInDecibel(SAMPLE *audioframe, int len){
	double sum = 0;
	for(int i = 0; i <= len; i++)
	{
		double xi = *audioframe++;
		sum += pow(xi, 2);
	}
	// printf("\nenergy = %f\n",10*log10(sum));
	return 10*log10(sum);
}

// Judge whether someone is speaking in one frame
bool classifyFrame(SAMPLE *audioframe, int len){
	isSpeech = false;
	double current = EnergyPerFrameInDecibel(audioframe, len);
	static double level = levelInit; //初始化为第一帧的能量; 
	static double background = bgInit/10;//初始化为前十帧的平均帧能量

	level = ((level * FORGET_FACTOR) + current)/(FORGET_FACTOR + 1);
	if(current < background)
		background = current;
	else
		background += (current - background) * ADJUSTMENT;

	if (level < background)
		level = background;
	if (level -background > THRESHOLD)
		isSpeech= true;

	if (!isSpeech)
		continuousSilenceCount++;
	else
		continuousSilenceCount = 0;
    return isSpeech;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paTestData *data = (paTestData*)userData;
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
    SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    long framesToCalc;
    long i;
    int finished;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

    (void) outputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if( framesLeft < framesPerBuffer )
    {
        framesToCalc = framesLeft;
        finished = paComplete;
    }
    else
    {
        framesToCalc = framesPerBuffer;
        finished = paContinue;
    }

    if( inputBuffer == NULL )
    {
        for( i=0; i<framesToCalc; i++ )
        {
            *wptr++ = SAMPLE_SILENCE;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = SAMPLE_SILENCE;  /* right */
        }
    }
    else
    {
        for( i=0; i<framesToCalc; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
    }

	// 前10帧用于初始化level和background值，恰好给讲话人一定时间冗余
	if(initCount < 10)
	{
		initCount++;
		if(initCount == 1)
			levelInit = EnergyPerFrameInDecibel(wptr-framesToCalc, framesToCalc);
		bgInit += EnergyPerFrameInDecibel(wptr-framesToCalc, framesToCalc);
	}
	else
	    classifyFrame(wptr-framesToCalc, framesToCalc);

    data->frameIndex += framesToCalc;

	if (continuousSilenceCount > SILENT_THRESHOLD)
	{
		finished = paComplete;
		printf("\nStop speaking Detected!  \n");
		totalFrames = data->frameIndex;	//录音的实际大小
	}
    return finished;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int playCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
    paTestData *data = (paTestData*)userData;
    SAMPLE *rptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    SAMPLE *wptr = (SAMPLE*)outputBuffer;
    unsigned int i;
    int finished;
    unsigned int framesLeft = totalFrames - data->frameIndex;

    (void) inputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if( framesLeft < framesPerBuffer )
    {
        /* final buffer... */
        for( i=0; i<framesLeft; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
        for( ; i<framesPerBuffer; i++ )
        {
            *wptr++ = 0;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = 0;  /* right */
        }
        data->frameIndex += framesLeft;
        finished = paComplete;
    }
    else
    {
        for( i=0; i<framesPerBuffer; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
        data->frameIndex += framesPerBuffer;
        finished = paContinue;
    }
    return finished;
}

bool data_capture()
{
    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream*           stream;
    PaError             err = paNoError;
    paTestData          data;
    int                 i;
    int                 numSamples;
    int                 numBytes;
    SAMPLE              max, val;
    double              average;

	printf("\n===== data capture and endpointing test =====\n"); fflush(stdout);

    data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE;
    data.frameIndex = 0;
    numSamples = totalFrames * NUM_CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
    data.recordedSamples = (SAMPLE *) malloc( numBytes );
    if( data.recordedSamples == NULL )
    {
        printf("Could not allocate record array.\n");
        goto done;
    }
    for( i=0; i<numSamples; i++ ) data.recordedSamples[i] = 0;

    err = Pa_Initialize();
    if( err != paNoError ) goto done;

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        goto done;
    }
    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    /* Record some audio. -------------------------------------------- */
    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              NULL,                  /* &outputParameters, */
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              recordCallback,
              &data );
    if( err != paNoError ) goto done;

	printf("Please hit any key to start your record.\n");
	while(!_kbhit());
	continuousSilenceCount = 0;
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto done;
    printf("\nNow recording! Please speak into the microphone.\n"); fflush(stdout);

    while( ( err = Pa_IsStreamActive( stream ) ) == 1)
    {
        Pa_Sleep(500);		// 每0.5秒状态监测1次
		printf("The status is %d at index = %d， and count = %d\n", isSpeech, data.frameIndex, continuousSilenceCount );
    }
    if( err < 0 ) goto done;

    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto done;

	// Refresh parameters of recorded data
	if (totalFrames != data.maxFrameIndex)
	{
		data.maxFrameIndex = totalFrames;
	    numSamples = totalFrames * NUM_CHANNELS;
	    numBytes = numSamples * sizeof(SAMPLE);
	}

	/* Measure maximum peak amplitude. */
    max = 0;
    average = 0.0;
    for( i = 0; i<numSamples; i++ )
    {
        val = data.recordedSamples[i];
        if( val < 0 ) val = -val; /* ABS */
        if( val > max )
        {
            max = val;
        }
        average += val;
    }
	data.recordedSamples[i] = NULL;
    average = average / (double)numSamples;

    printf("\nsample max amplitude = "PRINTF_S_FORMAT"\n", max );
    printf("sample average = %lf\n", average );
	system("pause");

    /* Write recorded data to a file. */
#if WRITE_TO_FILE
    {
		char wavFile[] = "recordData.wav";
		WriteWave(wavFile, data.recordedSamples, numSamples, SAMPLE_RATE);
    }
#endif

    /* Playback recorded data.  -------------------------------------------- */
    data.frameIndex = 0;

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default output device.\n");
        goto done;
    }
    outputParameters.channelCount = NUM_CHANNELS;
    outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              NULL, /* no input */
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              playCallback,
              &data );
    if( err != paNoError ) goto done;

    if( stream )
    {
        err = Pa_StartStream( stream );
        if( err != paNoError ) goto done;
        
        printf("\nNow playing back what you said just now.\n"); fflush(stdout);

		while( ( err = Pa_IsStreamActive( stream ) ) == 1 ) Pa_Sleep(100);
        if( err < 0 ) goto done;
        
        err = Pa_CloseStream( stream );
        if( err != paNoError ) goto done;
        
        printf("Done.\n"); fflush(stdout);
    }

done:
    Pa_Terminate();
    if( data.recordedSamples )       /* Sure it is NULL or valid. */
        free( data.recordedSamples );
    if( err != paNoError )
    {
        fprintf( stderr, "An error occured while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        err = 1;          /* Always return 0 or 1, but no other return codes. */
    }
    return (err == paNoError);
}