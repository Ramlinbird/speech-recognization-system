#ifndef _DATACAPTURE_H_
#define _DATACAPTURE_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>
#include "portaudio.h"
#include "readwave.h"

/***************** Data Sampling *****************/
#define	SAMPLE_RATE	(16000)	
#define	FRAMES_PER_BUFFER	(400)	// 25ms*16000=400
#define	NUM_SECONDS	(60)			// Maximum recording time
#define	NUM_CHANNELS	(1)			// mono
#define	DITHER_FLAG		(0)
#define	WRITE_TO_FILE	(1)

#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;				// 2Bytes = 16bits
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"

/***************** Endpointing Detection *****************/
#define THRESHOLD       (5)			// 10 for quiet enviroment and 5 for noisy
#define ADJUSTMENT      (0.05)
#define FORGET_FACTOR   (1)
#define SILENT_THRESHOLD (20)		// 25ms*20=0.5s

bool data_capture();

#endif //_DATACAPTURE_H_
