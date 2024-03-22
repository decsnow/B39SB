// fm4_adaptive.c

#include "fm4_wm8731_init.h"

#define BETA 0.01f            // learning rate
#define N 21                  // number of filter coeffs
#define NUM_ITERS 60          // number of iterations

float32_t desired[NUM_ITERS]; // storage for results
float32_t y_out[NUM_ITERS];
float32_t error[NUM_ITERS];
float32_t w[N+1] = {0.0};       // adaptive filter weights
float32_t x[N+1] = {0.0};       // adaptive filter delay line
int i, t;
float32_t d, y, e;

int main()
{
  for (t = 0; t < NUM_ITERS; t++)
  {
    x[0] = sin(2*PI*t/8);        // get new input sample
    d = cos(2*PI*t/8);         // get new desired output
    y = 0;                       // compute filter output
    for (i = 0; i <= N; i++)
      y += (w[i]*x[i]);
    e = d - y;                   // compute error
    for (i = N; i >= 0; i--)
    {
      w[i] += (BETA*e*x[i]);       // update filter weights
      if (i != 0)
        x[i] = x[i-1];               // shift data in delay line
    }
    desired[t] = d;              // store results
    y_out[t] = y;
    error[t] = e;
  }
  while(1){}
}
