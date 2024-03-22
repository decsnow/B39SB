// fm4_adaptnoise_CMSIS_intr.c

#include "fm4_wm8731_init.h"

#define SAMPLING_FREQ 8000
#define NOISE_FREQ 1200.0
#define SIGNAL_FREQ 2500.0
#define NOISE_AMPLITUDE 8000.0
#define SIGNAL_AMPLITUDE 8000.0

#define BETA 1E-12         // adaptive learning rate      
#define NUM_TAPS 10        // number of weights
#define BLOCK_SIZE 1

float32_t theta_increment_noise;
float32_t theta_noise = 0.0;
float32_t theta_increment_signal;
float32_t theta_signal = 0.0;
float32_t yout, error, signal, signoise, sigplusnoise, refnoise;


float32_t firStateF32[BLOCK_SIZE + NUM_TAPS -1];
float32_t firCoeffs32[NUM_TAPS] = {0.0};
arm_lms_instance_f32 S;

void PRGCRC_I2S_IRQHandler(void) 
{
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
  sigplusnoise = signoise+signal;
	
  arm_lms_f32(&S, &refnoise, &sigplusnoise, &yout, &error, 1);

  gpio_set(DIAGNOSTIC_PIN, LOW);
  sample.uint16bit[LEFT] = (int16_t)(error);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  
  i2s_tx(sample.uint32bit);

  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn); // probably not needed
}

int main(void)
{
  arm_lms_init_f32(&S, NUM_TAPS, firCoeffs32, firStateF32, BETA, 1);
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
