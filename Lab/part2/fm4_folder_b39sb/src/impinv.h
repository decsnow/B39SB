// impinv.h
// second order type 1 Chebyshev LPF with 2dB passband ripple
// and cutoff frequency 1500Hz

#define NUM_SECTIONS 1

float b[NUM_SECTIONS][3]={ {0.0, 0.48255, 0.0} };
float a[NUM_SECTIONS][3]={ {1.0, -0.71624, 0.387913} };
