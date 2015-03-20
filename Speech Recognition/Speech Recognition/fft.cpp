#include "fft.h"

/************************** FFT and IFFT Implementation From Website *****************/
/************* http://rosettacode.org/wiki/Fast_Fourier_transform#C.2B.2B ************/
 
// Cooley¨CTukey FFT (in-place)
void fft(CArray& x)
{
    const size_t N = x.size();
    if (N <= 1) return;
 
    // divide
    CArray even = x[slice(0, N/2, 2)];
    CArray  odd = x[slice(1, N/2, 2)];
 
    // conquer
    fft(even);
    fft(odd);
 
    // combine
    for (size_t k = 0; k < N/2; ++k)
    {
        Complex t = polar(1.0, -2 * PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k+N/2] = even[k] - t;
    }
}
 
// inverse fft (in-place)
void ifft(CArray& x)
{
    // conjugate the complex numbers
    x = x.apply(conj);
 
    // forward fft
    fft( x );
 
    // conjugate the complex numbers again
    x = x.apply(conj);
 
    // scale the numbers
    x /= x.size();
}

void testFFT(){
 	const Complex test[] = { 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0 };
    CArray data(test, 8);
 
    // forward fft
    fft(data);
 
    cout << "fft" << endl;
    for (int i = 0; i < 8; ++i)
    {
        cout << data[i] << endl;
    }
 
    // inverse fft
    ifft(data);
 
    cout << endl << "ifft" << endl;
    for (int i = 0; i < 8; ++i)
    {
        cout << data[i] << endl;
    }
}
/*****************************************************************************/
