#ifndef _FFT_H_
#define _FFT_H_

#include <stdio.h>
#include <complex>
#include <valarray>
#include <iostream>
using namespace std;

#define PI 3.141592653589793238460

typedef complex<double> Complex;
typedef valarray<Complex> CArray;

// Cooley¨CTukey FFT (in-place)
void fft(CArray& x);
// inverse fft (in-place)
void ifft(CArray& x);

#endif //_FFT_H_