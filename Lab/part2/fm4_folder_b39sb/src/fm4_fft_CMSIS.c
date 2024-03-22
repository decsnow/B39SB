// fm4_fft_CMSIS.c 

#include "fm4_wm8731_init.h"

#define N 128
#define TESTFREQ 800.0
#define SAMPLING_FREQ 8000.0

float32_t samples[2*N];

int main()
{    
  int n;

  for(n=0 ; n<N ; n++)
  {
    samples[2*n] = arm_cos_f32(2*PI*TESTFREQ*n/SAMPLING_FREQ);
    samples[2*n+1] = 0.0;
  }

  arm_cfft_f32(&arm_cfft_sR_f32_len128, samples, 0, 1);
  while(1){}
}	
