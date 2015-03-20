#ifndef _FEATURECOMPUTATION_H_
#define _FEATURECOMPUTATION_H_

#include <stdlib.h>
#include <math.h>
#include <fstream>
#include "fft.h"
#include "readwave.h"

const double eps=1e-6;
const float frameTime = 0.025, crossTime = 0.010;
const char wavFile[] = "recordData.wav"; // Read data from
const char file1[] = "power.txt", file2[] = "logMel.txt", file3[] = "dct.txt", file4[] = "norcep.txt"; // Output results to

bool feature_computation();

#endif //_FEATURECOMPUTATION_H_
