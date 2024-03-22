// fm4_adaptnoise_intr.c

#include "fm4_wm8731_init.h"

#define SAMPLING_FREQ 8000
#define NOISE_FREQ 1200.0
#define SIGNAL_FREQ 2500.0
#define NOISE_AMPLITUDE 8000.0
#define SIGNAL_AMPLITUDE 8000.0
#define BETA 1E-12         // adaptive learning rate      
#define N 10               // number of weights

float32_t w[N];            // adaptive filter weights
float32_t x[N];            // adaptive filter delay line

float32_t theta_increment_noise;
float32_t theta_noise = 0.0;
float32_t theta_increment_signal;
float32_t theta_signal = 0.0;
float32_t yn, error, signal, signoise, refnoise, dummy;

void PRGCRC_I2S_IRQHandler(void) 
{
  int16_t i;
  union WM8731_data sample;

  sample.uint32bit = i2s_rx();
  gpio_set(DIAGNOSTIC_PIN, HIGH);

  theta_increment_noise = 2*PI*NOISE_FREQ/SAMPLING_FREQ;
  theta_noise += theta_increment_noise;
  if (theta_noise > 2*PI) theta_noise -= 2*PI;
  theta_increment_signal = 2*PI*SIGNAL_FREQ/SAMPLING_FREQ;
  theta_signal += theta_increment_signal;
  if (theta_signal > 2*PI) theta_signal -= 2*PI;

  refnoise = (NOISE_AMPLITUDE*arm_cos_f32(theta_noise));  
  signoise = (NOISE_AMPLITUDE*arm_sin_f32(theta_noise));
  signal = (SIGNAL_AMPLITUDE*arm_sin_f32(theta_signal));
  
  x[0] = refnoise;   // reference input to adaptive filter
  yn = 0;            // compute adaptive filter output
  for (i = 0; i < N; i++)
     yn += (w[i] * x[i]);
  
  error = signal + signoise - yn; // compute error   
          			
  for (i = N-1; i >= 0; i--)      // update weights
  {                               // and delay line
     dummy = BETA*error;
		 dummy = dummy*x[i];
		 w[i] = w[i] + dummy;
     x[i] = x[i-1];
  }  
  gpio_set(DIAGNOSTIC_PIN, LOW);
  sample.uint16bit[LEFT] = (int16_t)(error);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  
  i2s_tx(sample.uint32bit);

  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn); // probably not needed
}

int main(void)
{
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
